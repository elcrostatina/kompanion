// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "app/CodeBlockHighlighter.h"

#include <KSyntaxHighlighting/Definition>
#include <KSyntaxHighlighting/Repository>
#include <KSyntaxHighlighting/SyntaxHighlighter>
#include <KSyntaxHighlighting/Theme>

namespace {

constexpr QLatin1StringView FileNamePrefix{"code."};

KSyntaxHighlighting::Repository &sharedRepository()
{
    static KSyntaxHighlighting::Repository repository;
    return repository;
}

KSyntaxHighlighting::Definition definitionForLanguage(const QString &language)
{
    KSyntaxHighlighting::Repository &repository = sharedRepository();
    const QString trimmed = language.trimmed();
    if (trimmed.isEmpty()) {
        return {};
    }
    const KSyntaxHighlighting::Definition byName = repository.definitionForName(trimmed);
    if (byName.isValid()) {
        return byName;
    }
    const QList<KSyntaxHighlighting::Definition> all = repository.definitions();
    const auto sameName = std::find_if(all.cbegin(), all.cend(), [&trimmed](const KSyntaxHighlighting::Definition &definition) {
        return definition.name().compare(trimmed, Qt::CaseInsensitive) == 0;
    });
    if (sameName != all.cend()) {
        return *sameName;
    }
    return repository.definitionForFileName(QString(FileNamePrefix) + trimmed);
}

}

CodeBlockHighlighter::CodeBlockHighlighter(QObject *parent)
    : QObject(parent)
{
}

QQuickTextDocument *CodeBlockHighlighter::textDocument() const
{
    return m_document;
}

void CodeBlockHighlighter::setTextDocument(QQuickTextDocument *document)
{
    if (m_document == document) {
        return;
    }
    delete m_highlighter;
    m_highlighter = nullptr;
    m_document = document;
    if (m_document && m_document->textDocument()) {
        m_highlighter = new KSyntaxHighlighting::SyntaxHighlighter(m_document->textDocument());
        applyDefinitionAndTheme();
    }
    Q_EMIT textDocumentChanged();
}

QString CodeBlockHighlighter::language() const
{
    return m_language;
}

void CodeBlockHighlighter::setLanguage(const QString &language)
{
    if (m_language == language) {
        return;
    }
    m_language = language;
    applyDefinitionAndTheme();
    Q_EMIT languageChanged();
}

bool CodeBlockHighlighter::hasDarkBackground() const
{
    return m_darkBackground;
}

void CodeBlockHighlighter::setDarkBackground(bool dark)
{
    if (m_darkBackground == dark) {
        return;
    }
    m_darkBackground = dark;
    applyDefinitionAndTheme();
    Q_EMIT darkBackgroundChanged();
}

void CodeBlockHighlighter::applyDefinitionAndTheme()
{
    if (!m_highlighter) {
        return;
    }
    const auto themeKind = m_darkBackground ? KSyntaxHighlighting::Repository::DarkTheme : KSyntaxHighlighting::Repository::LightTheme;
    m_highlighter->setTheme(sharedRepository().defaultTheme(themeKind));
    m_highlighter->setDefinition(definitionForLanguage(m_language));
    m_highlighter->rehighlight();
}
