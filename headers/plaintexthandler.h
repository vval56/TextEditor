#ifndef PLAINTEXTHANDLER_H
#define PLAINTEXTHANDLER_H

#include "documenthandler.h"

class PlainTextHandler : public DocumentHandler
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
};

#endif // PLAINTEXTHANDLER_H

