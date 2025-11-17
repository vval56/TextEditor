#ifndef IDOCUMENT_H
#define IDOCUMENT_H

#include <QString>
#include <QColor>

class IDocument {
public:
    virtual ~IDocument() = default;

    // Read
    virtual QString getSelectedText() const = 0;
    virtual QString getAllText() const = 0;
    virtual QString getPlainText() const = 0;

    // Write
    virtual void setPlainText(const QString &text) = 0;
    virtual void insertTextAtCursor(const QString &text) = 0;

    // Formatting
    virtual void applyFontFamily(const QString &family) = 0;
    virtual void applyFontSize(int size) = 0;
    virtual void applyBold(bool bold) = 0;
    virtual void applyItalic(bool italic) = 0;
    virtual void applyUnderline(bool underline) = 0;
    virtual void applyTextColor(const QColor &color) = 0;

    // Stats
    virtual int getWordCount() const = 0;
    virtual int getCharacterCount() const = 0;
    virtual int getLineCount() const = 0;
    virtual int getParagraphCount() const = 0;

    // State
    virtual bool isModified() const = 0;
};

#endif // IDOCUMENT_H
