#include "../headers/document.h"
#include <QTextStream>
#include <QFile>
#include <QTextCursor>
#include <QStringConverter>

Document::Document(QObject *parent)
    : QObject(parent), m_doc(new QTextDocument(this))
{
    connect(m_doc, &QTextDocument::contentsChanged, this, [this]() {
        setModified(true);
    });
}

Document::Document(const QString &text, QObject *parent)
    : QObject(parent), m_doc(new QTextDocument(this))
{
    m_doc->setPlainText(text);
    connect(m_doc, &QTextDocument::contentsChanged, this, [this]() {
        setModified(true);
    });
}

QTextDocument *Document::qtDocument() const
{
    return m_doc;
}

bool Document::loadFromFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    m_doc->setPlainText(in.readAll());
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
    out << m_doc->toPlainText();
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

QString Document::getFileName() const { return m_fileName; }
QString Document::getFilePath() const { return m_filePath; }
QDateTime Document::getLastModified() const { return m_lastModified; }
qint64 Document::getFileSize() const { return m_fileSize; }
bool Document::isModified() const { return m_doc->isModified(); }
bool Document::isNew() const { return m_isNew; }

QString Document::getPlainText() const
{
    return m_doc->toPlainText();
}

void Document::setPlainText(const QString &text)
{
    m_doc->setPlainText(text);
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
    m_doc->setModified(modified);
}

void Document::clear()
{
    m_doc->clear();
    m_isNew = true;
    m_filePath.clear();
    m_fileName.clear();
    m_fileSize = 0;
}

QString Document::getSelectedText() const
{
    QTextCursor cursor(m_doc);
    return cursor.selectedText();
}

QString Document::getAllText() const
{
    return m_doc->toPlainText();
}

void Document::insertTextAtCursor(const QString &text)
{
    QTextCursor cursor(m_doc);
    cursor.insertText(text);
}
