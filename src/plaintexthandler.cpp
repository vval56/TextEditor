#include "plaintexthandler.h"

#include <QTextDocument>
#include <QFile>
#include <QTextStream>
#include <QStringConverter>

namespace {

const QStringList kPlainExtensions = {
    QStringLiteral("txt"),
    QStringLiteral("cc"),
    QStringLiteral("cpp"),
    QStringLiteral("h"),
    QStringLiteral("hpp")
};

bool isPlainTextExtension(const QString &ext)
{
    return kPlainExtensions.contains(ext.toLower());
}

}

bool PlainTextHandler::canLoad(const QString &extension) const
{
    return isPlainTextExtension(extension) || extension.isEmpty();
}

bool PlainTextHandler::canSave(const QString &extension) const
{
    return isPlainTextExtension(extension) || extension.isEmpty();
}

bool PlainTextHandler::load(const QString &filePath,
                            QTextDocument *document,
                            DocumentContext &context,
                            QString &error)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        error = QObject::tr("Не удалось открыть файл '%1' для чтения").arg(filePath);
        return false;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);  // Замена setCodec
    document->setPlainText(stream.readAll());
    file.close();

    context.isReadOnly = false;
    context.workingDirectory.clear();
    context.workingFile.clear();

    return true;
}

bool PlainTextHandler::save(const QString &filePath,
                            QTextDocument *document,
                            DocumentContext &context,
                            QString &error)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        error = QObject::tr("Не удалось открыть файл '%1' для записи").arg(filePath);
        return false;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);  // Замена setCodec
    stream << document->toPlainText();
    file.close();

    context.isReadOnly = false;
    context.workingDirectory.clear();
    context.workingFile.clear();

    return true;
}
