// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/OverrideScripts.h"

#include <QJsonArray>
#include <QJsonDocument>

namespace {

constexpr QLatin1StringView CssPlaceholder{"%CSS%"};
constexpr QLatin1StringView CodePlaceholder{"%CODE%"};

const QString &styleScriptTemplate()
{
    static const QString source = QStringLiteral(R"js((() => {
  const styleId = "kompanion-override-style";
  const css = %CSS%;
  const existing = document.getElementById(styleId);
  if (window.__kompanionStyleObserver) {
    window.__kompanionStyleObserver.disconnect();
    window.__kompanionStyleObserver = null;
  }
  if (css.length === 0) {
    if (existing) existing.remove();
    return;
  }
  const ensureStyle = () => {
    let style = document.getElementById(styleId);
    if (!style) {
      style = document.createElement("style");
      style.id = styleId;
    }
    if (style.textContent !== css) style.textContent = css;
    const root = document.documentElement;
    if (root && style.parentNode !== root) root.appendChild(style);
  };
  ensureStyle();
  window.__kompanionStyleObserver = new MutationObserver(ensureStyle);
  window.__kompanionStyleObserver.observe(document.documentElement, { childList: true });
})();)js");
    return source;
}

const QString &userScriptTemplate()
{
    static const QString source = QStringLiteral(R"js((() => {
  try {
%CODE%
  } catch (error) {
    console.error("Kompanion user script failed:", error);
  }
})();)js");
    return source;
}

QString asJavaScriptStringLiteral(const QString &text)
{
    const QByteArray json = QJsonDocument(QJsonArray{text}).toJson(QJsonDocument::Compact);
    return QString::fromUtf8(json.mid(1, json.size() - 2));
}

}

QString OverrideScripts::styleScript(const QString &css)
{
    QString source = styleScriptTemplate();
    return source.replace(CssPlaceholder, asJavaScriptStringLiteral(css.trimmed()));
}

QString OverrideScripts::userScript(const QString &javaScript)
{
    if (javaScript.trimmed().isEmpty()) {
        return QString();
    }
    QString source = userScriptTemplate();
    return source.replace(CodePlaceholder, javaScript);
}
