// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/BaseStyle.h"
#include "core/OverrideFiles.h"
#include "core/OverrideScripts.h"
#include "core/PageColorScript.h"

#include <QJSEngine>
#include <QTemporaryDir>
#include <QTest>

namespace {

const QString FakeDom = QStringLiteral(R"js(
var root = {
  children: [],
  appendChild: function (element) { element.parentNode = root; root.children.push(element); }
};
var document = {
  documentElement: root,
  getElementById: function (id) { return root.children.find(function (child) { return child.id === id; }) || null; },
  createElement: function (tag) {
    return { tag: tag, id: "", textContent: "", parentNode: null,
      remove: function () { root.children = root.children.filter(function (child) { return child !== this; }, this); this.parentNode = null; } };
  }
};
var observerCount = 0;
function MutationObserver(callback) {
  this.callback = callback;
  this.observe = function () { observerCount++; };
  this.disconnect = function () { observerCount--; };
}
var window = {};
)js");

QJSEngine *engineWithFakeDom()
{
    auto *engine = new QJSEngine;
    engine->evaluate(FakeDom);
    return engine;
}

}

class TestOverrideScripts : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void styleScriptInstallsStyleElementWithCss();
    void styleScriptEscapesQuotesAndNewlines();
    void styleScriptIsIdempotentAndUpdatesInPlace();
    void emptyCssRemovesStyleElement();
    void userScriptRunsInsideGuard();
    void userScriptErrorsAreContained();
    void emptyUserScriptProducesNothing();
    void filesCreateDefaultCssOnDemand();
    void filesResetRestoresDefault();
    void unknownProviderGetsGenericDefault();
    void baseStyleThinsTheScrollbars();
    void pageColorUsesNearestOpaqueBackground();
    void pageColorUnderstandsModernColorSyntax();
    void pageColorFallsBackToThemeColorMeta();
};

void TestOverrideScripts::styleScriptInstallsStyleElementWithCss()
{
    QScopedPointer<QJSEngine> engine(engineWithFakeDom());
    const QJSValue result = engine->evaluate(OverrideScripts::styleScript(QStringLiteral("body { color: red; }")));
    QVERIFY2(!result.isError(), qPrintable(result.toString()));
    QCOMPARE(engine->evaluate(QStringLiteral("root.children.length")).toInt(), 1);
    QCOMPARE(engine->evaluate(QStringLiteral("root.children[0].tag")).toString(), QStringLiteral("style"));
    QCOMPARE(engine->evaluate(QStringLiteral("root.children[0].textContent")).toString(), QStringLiteral("body { color: red; }"));
    QCOMPARE(engine->evaluate(QStringLiteral("observerCount")).toInt(), 1);
}

void TestOverrideScripts::styleScriptEscapesQuotesAndNewlines()
{
    QScopedPointer<QJSEngine> engine(engineWithFakeDom());
    const QString css = QStringLiteral("a::after { content: \"x\\\"y\"; }\n/* </style> */\nb { top: 0 }");
    QVERIFY(!engine->evaluate(OverrideScripts::styleScript(css)).isError());
    QCOMPARE(engine->evaluate(QStringLiteral("root.children[0].textContent")).toString(), css);
}

void TestOverrideScripts::styleScriptIsIdempotentAndUpdatesInPlace()
{
    QScopedPointer<QJSEngine> engine(engineWithFakeDom());
    engine->evaluate(OverrideScripts::styleScript(QStringLiteral("a{}")));
    engine->evaluate(OverrideScripts::styleScript(QStringLiteral("b{}")));
    QCOMPARE(engine->evaluate(QStringLiteral("root.children.length")).toInt(), 1);
    QCOMPARE(engine->evaluate(QStringLiteral("root.children[0].textContent")).toString(), QStringLiteral("b{}"));
    QCOMPARE(engine->evaluate(QStringLiteral("observerCount")).toInt(), 1);
}

void TestOverrideScripts::emptyCssRemovesStyleElement()
{
    QScopedPointer<QJSEngine> engine(engineWithFakeDom());
    engine->evaluate(OverrideScripts::styleScript(QStringLiteral("a{}")));
    engine->evaluate(OverrideScripts::styleScript(QString()));
    QCOMPARE(engine->evaluate(QStringLiteral("root.children.length")).toInt(), 0);
    QCOMPARE(engine->evaluate(QStringLiteral("observerCount")).toInt(), 0);
}

void TestOverrideScripts::userScriptRunsInsideGuard()
{
    QScopedPointer<QJSEngine> engine(engineWithFakeDom());
    const QJSValue result = engine->evaluate(OverrideScripts::userScript(QStringLiteral("window.touched = 42;")));
    QVERIFY2(!result.isError(), qPrintable(result.toString()));
    QCOMPARE(engine->evaluate(QStringLiteral("window.touched")).toInt(), 42);
}

void TestOverrideScripts::userScriptErrorsAreContained()
{
    QScopedPointer<QJSEngine> engine(engineWithFakeDom());
    engine->evaluate(QStringLiteral("var console = { error: function () { window.logged = true; } };"));
    const QJSValue result = engine->evaluate(OverrideScripts::userScript(QStringLiteral("throw new Error('boom');")));
    QVERIFY(!result.isError());
    QVERIFY(engine->evaluate(QStringLiteral("window.logged")).toBool());
}

void TestOverrideScripts::emptyUserScriptProducesNothing()
{
    QVERIFY(OverrideScripts::userScript(QString()).isEmpty());
    QVERIFY(OverrideScripts::userScript(QStringLiteral("  \n ")).isEmpty());
}

void TestOverrideScripts::filesCreateDefaultCssOnDemand()
{
    QTemporaryDir directory;
    const OverrideFiles files(directory.filePath(QStringLiteral("overrides")));
    QVERIFY(files.readCss(QStringLiteral("claude")).isEmpty());
    files.ensureCssExists(QStringLiteral("claude"));
    QVERIFY(files.readCss(QStringLiteral("claude")).contains(QStringLiteral("Claude")));
}

void TestOverrideScripts::filesResetRestoresDefault()
{
    QTemporaryDir directory;
    const OverrideFiles files(directory.filePath(QStringLiteral("overrides")));
    files.ensureCssExists(QStringLiteral("gemini"));
    const QString original = files.readCss(QStringLiteral("gemini"));
    QFile edited(files.cssPath(QStringLiteral("gemini")));
    QVERIFY(edited.open(QIODevice::WriteOnly | QIODevice::Truncate));
    edited.write("body { }");
    edited.close();
    files.ensureCssExists(QStringLiteral("gemini"));
    QCOMPARE(files.readCss(QStringLiteral("gemini")), QStringLiteral("body { }"));
    files.resetCss(QStringLiteral("gemini"));
    QCOMPARE(files.readCss(QStringLiteral("gemini")), original);
}

void TestOverrideScripts::unknownProviderGetsGenericDefault()
{
    QTemporaryDir directory;
    const OverrideFiles files(directory.filePath(QStringLiteral("overrides")));
    files.ensureCssExists(QStringLiteral("my-custom-site"));
    QVERIFY(files.readCss(QStringLiteral("my-custom-site")).contains(QStringLiteral("custom web providers")));
}

namespace {

QString pageColorFor(const QString &elementChain, const QString &metaContent = QString())
{
    QJSEngine engine;
    engine.evaluate(QStringLiteral(R"js(
var window = { innerWidth: 400, innerHeight: 800 };
function getComputedStyle(element) { return { backgroundColor: element.background }; }
%1
var document = {
  elementFromPoint: function () { return leaf; },
  querySelector: function () { return %2; }
};
)js").arg(elementChain, metaContent.isEmpty() ? QStringLiteral("null") : QStringLiteral("{ content: \"%1\" }").arg(metaContent)));
    return engine.evaluate(PageColorScript::sampleBackground()).toString();
}

}

void TestOverrideScripts::baseStyleThinsTheScrollbars()
{
    QVERIFY(BaseStyle::css().contains(QStringLiteral("scrollbar-width: thin")));
}

void TestOverrideScripts::pageColorUsesNearestOpaqueBackground()
{
    const QString chain = QStringLiteral(
        "var root = { background: 'rgb(255, 255, 255)', parentElement: null };"
        "var mid = { background: 'rgb(26, 27, 28)', parentElement: root };"
        "var leaf = { background: 'rgba(0, 0, 0, 0)', parentElement: mid };");
    QCOMPARE(pageColorFor(chain), QStringLiteral("#1a1b1c"));
}

void TestOverrideScripts::pageColorUnderstandsModernColorSyntax()
{
    const QString chain = QStringLiteral("var leaf = { background: 'color(srgb 0.1 0.2 0.4)', parentElement: null };");
    QCOMPARE(pageColorFor(chain), QStringLiteral("#1a3366"));
}

void TestOverrideScripts::pageColorFallsBackToThemeColorMeta()
{
    const QString chain = QStringLiteral("var leaf = { background: 'rgba(0, 0, 0, 0)', parentElement: null };");
    QCOMPARE(pageColorFor(chain, QStringLiteral("#223344")), QStringLiteral("#223344"));
    QVERIFY(pageColorFor(chain).isEmpty());
}

QTEST_MAIN(TestOverrideScripts)

#include "tst_overridescripts.moc"
