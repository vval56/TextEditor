#ifndef DOCUMENTMANAGER_H
#define DOCUMENTMANAGER_H

#include "documenthandler.h"

#include <QString>
#include <memory>
#include <vector>

class QTextDocument;

class DocumentManager
{
public:
    DocumentManager();
    ~DocumentManager();

    DocumentContext &context();

    bool loadDocument(const QString &filePath, QTextDocument *document, QString &errorMessage);
    bool saveDocument(const QString &filePath, QTextDocument *document, QString &errorMessage);

    QString filterForOpenDialog() const;
    QString filterForSaveDialog() const;

private:
    using DocumentHandlerPtr = std::unique_ptr<DocumentHandler>;
    std::vector<DocumentHandlerPtr> handlers_;
    DocumentContext context_;

    DocumentHandler *selectHandlerForExtension(const QString &extension, bool forSave) const;
};

#endif
