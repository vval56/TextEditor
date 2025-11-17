#include "../headers/document.h"
#include <QTextStream>
#include <QFile>
#include <QTextCursor>
#include <QFont>
#include <QStringConverter>
#include <QRegularExpression>

Document::Document(QObject *parent)
    : QTextDocument(parent), m_isNew(true), m_fileSize(0)
{
    connect(this, &QTextDocument::contentsChanged, this, [this]() {
        setModified(true);
    });
}

Document::Document(const QString &text, QObject *parent)
    : QTextDocument(text, parent), m_isNew(true), m_fileSize(0)
{
    connect(this, &QTextDocument::contentsChanged, this, [this]() {
        setModified(true);
    });
}

bool Document::loadFromFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    setPlainText(in.readAll());
    file.close();

    m_filePath = fileName;
    m_fileName = QFileInfo(fileName).fileName();
    updateFileInfo();
    m_isNew = false;
    setModified(false);

    return true;
}

bool Document::saveToFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << toPlainText();
    file.close();

    m_filePath = fileName;
    m_fileName = QFileInfo(fileName).fileName();
    updateFileInfo();
    m_isNew = false;
    setModified(false);

    return true;
}

bool Document::save()
{
    if (m_filePath.isEmpty() || m_isNew) {
        return false;
    }
    return saveToFile(m_filePath);
}

// Реализация остальных методов...
QString Document::getFileName() const { return m_fileName; }
QString Document::getFilePath() const { return m_filePath; }
QDateTime Document::getLastModified() const { return m_lastModified; }
qint64 Document::getFileSize() const { return m_fileSize; }
bool Document::isModified() const { return QTextDocument::isModified(); }
bool Document::isNew() const { return m_isNew; }

int Document::getWordCount() const
{
    QString text = toPlainText();
    QStringList words = text.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    return words.size();
}

int Document::getCharacterCount() const
{
    return toPlainText().length();
}

int Document::getLineCount() const
{
    return toPlainText().count('\n') + 1;
}

int Document::getParagraphCount() const
{
    return blockCount();
}

QString Document::getPlainText() const
{
    return toPlainText();
}

void Document::setPlainText(const QString &text)
{
    QTextDocument::setPlainText(text);
}

void Document::updateFileInfo()
{
    if (!m_filePath.isEmpty()) {
        QFileInfo fileInfo(m_filePath);
        m_lastModified = fileInfo.lastModified();
        m_fileSize = fileInfo.size();
    }
}

void Document::setModified(bool modified)
{
    QTextDocument::setModified(modified);
}

void Document::clear()
{
    QTextDocument::clear();
    m_isNew = true;
    m_filePath.clear();
    m_fileName.clear();
    m_fileSize = 0;
}

// Методы форматирования
void Document::applyFontFamily(const QString &family)
{
    QTextCursor cursor(this);
    if (!cursor.hasSelection()) {
        cursor.select(QTextCursor::WordUnderCursor);
    }

    QTextCharFormat format;
    format.setFontFamily(family);
    cursor.mergeCharFormat(format);
}

void Document::applyFontSize(int size)
{
    QTextCursor cursor(this);
    if (!cursor.hasSelection()) {
        cursor.select(QTextCursor::WordUnderCursor);
    }

    QTextCharFormat format;
    format.setFontPointSize(size);
    cursor.mergeCharFormat(format);
}

void Document::applyBold(bool bold)
{
    QTextCursor cursor(this);
    if (!cursor.hasSelection()) {
        cursor.select(QTextCursor::WordUnderCursor);
    }

    QTextCharFormat format;
    format.setFontWeight(bold ? QFont::Bold : QFont::Normal);
    cursor.mergeCharFormat(format);
}

void Document::applyItalic(bool italic)
{
    QTextCursor cursor(this);
    if (!cursor.hasSelection()) {
        cursor.select(QTextCursor::WordUnderCursor);
    }

    QTextCharFormat format;
    format.setFontItalic(italic);
    cursor.mergeCharFormat(format);
}

void Document::applyUnderline(bool underline)
{
    QTextCursor cursor(this);
    if (!cursor.hasSelection()) {
        cursor.select(QTextCursor::WordUnderCursor);
    }

    QTextCharFormat format;
    format.setFontUnderline(underline);
    cursor.mergeCharFormat(format);
}

void Document::applyTextColor(const QColor &color)
{
    QTextCursor cursor(this);
    if (!cursor.hasSelection()) {
        cursor.select(QTextCursor::WordUnderCursor);
    }

    QTextCharFormat format;
    format.setForeground(QBrush(color));
    cursor.mergeCharFormat(format);
}

// Аналогично для alignment...

QString Document::getSelectedText() const
{
    QTextCursor cursor(const_cast<Document*>(this));
    return cursor.selectedText();
}

QString Document::getAllText() const
{
    return toPlainText();
}

void Document::insertTextAtCursor(const QString &text)
{
    QTextCursor cursor(this);
    cursor.insertText(text);
}
