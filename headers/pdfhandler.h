#ifndef PDFHANDLER_H
#define PDFHANDLER_H

#include "documenthandler.h"

class PdfHandler : public DocumentHandler
{
public:
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
    bool ensurePdfToolsAvailable(QString &error) const;
    bool convertPdfToHtmlStdout(const QString &pdfPath, QString &htmlOut, QString &error) const;
};

#endif // PDFHANDLER_H


