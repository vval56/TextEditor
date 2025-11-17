#ifndef IDOCUMENT_H
#define IDOCUMENT_H

#include <QString>
#include <QColor>

class IDocument {
public:
    virtual ~IDocument() = default;

    virtual  QString getSelectedText() const = 0;
    virtual QString getAllText() const = 0;
    virtual QString getPlainText() const = 0;

    virtual void setPlainText(const QString &text) = 0;
    virtual void insertTextAtCursor(const QString &text) = 0;

    virtual bool isModified() const = 0;
};

#endif
