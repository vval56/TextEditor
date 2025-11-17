#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QString>
#include <QTextDocument>
#include <QFileInfo>
#include <QDateTime>
#include "idocument.h"

class Document : public QTextDocument, public IDocument
{
    Q_OBJECT

public:
    explicit Document(QObject *parent = nullptr);
    Document(const QString &text, QObject *parent = nullptr);

    // Методы для работы с файлами
    bool loadFromFile(const QString &fileName);
    bool saveToFile(const QString &fileName);
    bool save();

    // Методы для получения информации о документе
    QString getFileName() const;
    QString getFilePath() const;
    QDateTime getLastModified() const;
    qint64 getFileSize() const;
    bool isModified() const override;
    bool isNew() const;

    // Статистика документа
    int getWordCount() const override;
    int getCharacterCount() const override;
    int getLineCount() const override;
    int getParagraphCount() const override;

    // Методы для работы с содержимым
    QString getPlainText() const override;
    void setPlainText(const QString &text) override;

    // Методы форматирования
    void applyFontFamily(const QString &family) override;
    void applyFontSize(int size) override;
    void applyBold(bool bold) override;
    void applyItalic(bool italic) override;
    void applyUnderline(bool underline) override;
    void applyTextColor(const QColor &color) override;
    void applyAlignment(Qt::Alignment alignment);

    // Методы поиска и замены
    int findText(const QString &text, bool caseSensitive = false);
    int replaceText(const QString &search, const QString &replace, bool caseSensitive = false);

    // Методы отмены/повтора
    bool canUndo() const;
    bool canRedo() const;

    // Методы для работы с речью
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

    void updateFileInfo();
};

#endif // DOCUMENT_H
