#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QString>
#include <QTextDocument>
#include <QFileInfo>
#include <QDateTime>

class Document : public QTextDocument
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
    bool isModified() const;
    bool isNew() const;

    // Статистика документа
    int getWordCount() const;
    int getCharacterCount() const;
    int getLineCount() const;
    int getParagraphCount() const;

    // Методы для работы с содержимым
    QString getPlainText() const;
    void setPlainText(const QString &text);

    // Методы форматирования
    void applyFontFamily(const QString &family);
    void applyFontSize(int size);
    void applyBold(bool bold);
    void applyItalic(bool italic);
    void applyUnderline(bool underline);
    void applyTextColor(const QColor &color);
    void applyAlignment(Qt::Alignment alignment);

    // Методы поиска и замены
    int findText(const QString &text, bool caseSensitive = false);
    int replaceText(const QString &search, const QString &replace, bool caseSensitive = false);

    // Методы отмены/повтора
    bool canUndo() const;
    bool canRedo() const;

    // Методы для работы с речью
    QString getSelectedText() const;
    QString getAllText() const;
    void insertTextAtCursor(const QString &text);

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
