#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QString>
#include <QObject>
#include <QFileInfo>
#include <QDateTime>
#include <QTextDocument>
#include "idocument.h"

class Document : public QObject, public IDocument
{
    Q_OBJECT

public:
    explicit Document(QObject *parent = nullptr);
    explicit Document(const QString &text, QObject *parent = nullptr);

    QTextDocument *qtDocument() const;

    bool loadFromFile(const QString &fileName);
    bool saveToFile(const QString &fileName);
    bool save();
    QString getFileName() const;
    QString getFilePath() const;
    QDateTime getLastModified() const;
    qint64 getFileSize() const;
    bool isModified() const override;
    bool isNew() const;

    QString getPlainText() const override;
    void setPlainText(const QString &text) override;

    QString getSelectedText() const override;
    QString getAllText() const override;
    void insertTextAtCursor(const QString &text) override;

public slots:
    void setModified(bool modified = true);
    void clear();

private:
    QString m_filePath;
    QString m_fileName;
    QDateTime m_lastModified;
    qint64 m_fileSize;
    bool m_isNew;

    QTextDocument *m_doc;

    void updateFileInfo();
};

#endif
