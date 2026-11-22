// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QPointer>
#include <QQmlEngine>
#include <QQuickTextDocument>

namespace KSyntaxHighlighting {
class SyntaxHighlighter;
}

class CodeBlockHighlighter : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QQuickTextDocument *textDocument READ textDocument WRITE setTextDocument NOTIFY textDocumentChanged FINAL)
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged FINAL)
    Q_PROPERTY(bool darkBackground READ hasDarkBackground WRITE setDarkBackground NOTIFY darkBackgroundChanged FINAL)

public:
    explicit CodeBlockHighlighter(QObject *parent = nullptr);

    QQuickTextDocument *textDocument() const;
    void setTextDocument(QQuickTextDocument *document);
    QString language() const;
    void setLanguage(const QString &language);
    bool hasDarkBackground() const;
    void setDarkBackground(bool dark);

Q_SIGNALS:
    void textDocumentChanged();
    void languageChanged();
    void darkBackgroundChanged();

private:
    void applyDefinitionAndTheme();

    QPointer<QQuickTextDocument> m_document;
    KSyntaxHighlighting::SyntaxHighlighter *m_highlighter = nullptr;
    QString m_language;
    bool m_darkBackground = false;
};
