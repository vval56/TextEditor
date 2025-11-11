#include "documentmanager.h"
#include "documenthandler.h"
#include "plaintexthandler.h"
#include "libreofficehandler.h"
#include "pdfhandler.h"  // Добавьте этот include

#include <QTextDocument>
#include <QFileInfo>
#include <QUrl>
#include <QDir>
#include <QObject>
#include <vector>

namespace {

QString normalizeExtension(const QString &path)
{
    QFileInfo info(path);
    return info.suffix().toLower();
}

}

DocumentManager::DocumentManager()
{
    // Добавьте PdfHandler в список обработчиков
    handlers_.push_back(std::make_unique<PlainTextHandler>());
    handlers_.push_back(std::make_unique<LibreOfficeHandler>());
    handlers_.push_back(std::make_unique<PdfHandler>());  // Добавьте эту строку
}

DocumentManager::~DocumentManager() = default;

DocumentContext &DocumentManager::context()
{
    return context_;
}

bool DocumentManager::loadDocument(const QString &filePath, QTextDocument *document, QString &errorMessage)
{
    if (!document) {
        errorMessage = QObject::tr("Документ не инициализирован");
        return false;
    }

    const QString extension = normalizeExtension(filePath);
    DocumentHandler *handler = selectHandlerForExtension(extension, false);
    if (!handler) {
        errorMessage = QObject::tr("Формат '%1' не поддерживается").arg(extension);
        return false;
    }

    context_ = DocumentContext{};
    context_.sourcePath = filePath;
    context_.originalExtension = extension;

    if (!handler->load(filePath, document, context_, errorMessage)) {
        return false;
    }

    if (!context_.workingDirectory.isEmpty()) {
        document->setBaseUrl(QUrl::fromLocalFile(context_.workingDirectory + QDir::separator()));
    }

    return true;
}

bool DocumentManager::saveDocument(const QString &filePath, QTextDocument *document, QString &errorMessage)
{
    if (!document) {
        errorMessage = QObject::tr("Документ не инициализирован");
        return false;
    }

    const QString extension = normalizeExtension(filePath);
    DocumentHandler *handler = selectHandlerForExtension(extension, true);
    if (!handler) {
        errorMessage = QObject::tr("Формат '%1' не поддерживается для сохранения").arg(extension);
        return false;
    }

    if (!handler->save(filePath, document, context_, errorMessage)) {
        return false;
    }

    context_.sourcePath = filePath;
    context_.originalExtension = extension;

    return true;
}

QString DocumentManager::filterForOpenDialog() const
{
    QStringList parts;
    parts << QObject::tr("Все файлы (*.*)");
    parts << QObject::tr("Текстовые файлы (*.txt *.cc *.cpp *.h *.hpp)");
    parts << QObject::tr("Документы (*.docx *.odt)");
    parts << QObject::tr("PDF файлы (*.pdf)");  // Добавьте фильтр для PDF
    return parts.join(";;");
}

QString DocumentManager::filterForSaveDialog() const
{
    QStringList parts;
    parts << QObject::tr("Текстовые файлы (*.txt *.cc *.cpp *.h *.hpp)");
    parts << QObject::tr("Документы (*.docx *.odt)");
    parts << QObject::tr("PDF файлы (*.pdf)");  // Добавьте фильтр для PDF
    return parts.join(";;");
}

DocumentHandler *DocumentManager::selectHandlerForExtension(const QString &extension, bool forSave) const
{
    for (const auto &handler : handlers_) {
        if (!handler) {
            continue;
        }
        if (!forSave && handler->canLoad(extension)) {
            return handler.get();
        }
        if (forSave && handler->canSave(extension)) {
            return handler.get();
        }
    }
    return nullptr;
}
