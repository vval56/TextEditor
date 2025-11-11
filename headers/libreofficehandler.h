#ifndef LIBREOFFICEHANDLER_H
#define LIBREOFFICEHANDLER_H

#include "documenthandler.h"
#include <QTemporaryDir>

class LibreOfficeHandler : public DocumentHandler
{
public:
    LibreOfficeHandler();

    bool canLoad(const QString &extension) const override;
    bool canSave(const QString &extension) const override;

    bool load(const QString &filePath,
              QTextDocument *document,
              DocumentContext &context,
              QString &error) override;

    bool save(const QString &filePath,
              QTextDocument *document,
              DocumentContext &context,
              QString &error) override;

private:
    bool ensureLibreOfficeAvailable(QString &error) const;
    QString findLibreOfficeExecutable() const;
    bool convertToHtml(const QString &filePath,
                       const QString &outputDir,
                       QString &htmlFilePath,
                       QString &error) const;
    bool convertFromHtml(const QString &htmlPath,
                         const QString &targetFormat,
                         const QString &destinationPath,
                         QString &error) const;

    QString libreOfficeBinary_;
};

#endif // LIBREOFFICEHANDLER_H

