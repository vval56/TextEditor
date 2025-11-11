#ifndef DOCUMENTHANDLER_H
#define DOCUMENTHANDLER_H

#include <QString>
#include <QStringList>
#include <memory>

class QTextDocument;

struct DocumentContext
{
    QString sourcePath;
    QString workingDirectory;
    QString workingFile;
    QString originalExtension;
    bool isReadOnly = false;
};

class DocumentHandler
{
public:
    virtual ~DocumentHandler() = default;

    virtual bool canLoad(const QString &extension) const = 0;
    virtual bool canSave(const QString &extension) const = 0;

    virtual bool load(const QString &filePath,
                      QTextDocument *document,
                      DocumentContext &context,
                      QString &error) = 0;

    virtual bool save(const QString &filePath,
                      QTextDocument *document,
                      DocumentContext &context,
                      QString &error) = 0;
};

using DocumentHandlerPtr = std::unique_ptr<DocumentHandler>;

#endif // DOCUMENTHANDLER_H

