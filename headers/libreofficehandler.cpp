#include "libreofficehandler.h"

#include <QTextDocument>
#include <QTemporaryDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDir>
#include <QProcess>
#include <QTextStream>
#include <QTextDocumentWriter>
#include <QFile>
#include <algorithm>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#endif

namespace libreoffice_internal {

const QStringList kLibreOfficeExtensions = {
    QStringLiteral("docx"),
    QStringLiteral("odt"),
    QStringLiteral("pdf")
};

bool copyRecursively(const QString &sourceDirPath, const QString &destinationDirPath)
{
    QDir source(sourceDirPath);
    if (!source.exists()) {
        return false;
    }

    QDir().mkpath(destinationDirPath);
    const QFileInfoList entries = source.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries);

    bool ok = true;
    for (const QFileInfo &entry : entries) {
        const QString srcPath = entry.absoluteFilePath();
        const QString dstPath = destinationDirPath + QDir::separator() + entry.fileName();

        bool result = false;
        if (entry.isDir()) {
            result = copyRecursively(srcPath, dstPath);
        } else {
            if (QFile::exists(dstPath)) {
                QFile::remove(dstPath);
            }
            result = QFile::copy(srcPath, dstPath);
        }

        if (!result) {
            ok = false;
            break;
        }
    }

    return ok;
}

QStringList possibleLibreOfficeBinaries()
{
    return {
        QStringLiteral("/Applications/LibreOffice.app/Contents/MacOS/soffice"),
        QStringLiteral("/usr/bin/soffice"),
        QStringLiteral("/usr/local/bin/soffice"),
        QStringLiteral("/opt/homebrew/bin/soffice"),
        QStringLiteral("soffice")
    };
}

QString extensionFromPath(const QString &path)
{
    return QFileInfo(path).suffix().toLower();
}

QString targetFormatForExtension(const QString &extension)
{
    if (extension == QLatin1String("docx")) {
        return QStringLiteral("docx");
    }
    if (extension == QLatin1String("odt")) {
        return QStringLiteral("odt");
    }
    if (extension == QLatin1String("pdf")) {
        return QStringLiteral("pdf");
    }
    return extension;
}

}

LibreOfficeHandler::LibreOfficeHandler()
    : libreOfficeBinary_(findLibreOfficeExecutable())
{
}

bool LibreOfficeHandler::canLoad(const QString &extension) const
{
    return libreoffice_internal::kLibreOfficeExtensions.contains(extension.toLower());
}

bool LibreOfficeHandler::canSave(const QString &extension) const
{
    return libreoffice_internal::kLibreOfficeExtensions.contains(extension.toLower());
}

bool LibreOfficeHandler::load(const QString &filePath,
                              QTextDocument *document,
                              DocumentContext &context,
                              QString &error)
{
    if (!ensureLibreOfficeAvailable(error)) {
        return false;
    }

    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        error = QObject::tr("Не удалось создать временную директорию для импорта");
        return false;
    }

    QString htmlFile;
    if (!convertToHtml(filePath, tempDir.path(), htmlFile, error)) {
        return false;
    }

    QFile html(htmlFile);
    if (!html.open(QIODevice::ReadOnly | QIODevice::Text)) {
        error = QObject::tr("Не удалось открыть промежуточный HTML '%1'").arg(htmlFile);
        return false;
    }

    QTextStream stream(&html);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    stream.setEncoding(QStringConverter::Utf8);
#else
    stream.setCodec("UTF-8");
#endif

    const QString htmlContent = stream.readAll();
    html.close();

    document->setHtml(htmlContent);
    document->setModified(false);

    context.isReadOnly = false;

    QString persistentDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (persistentDir.isEmpty()) {
        persistentDir = QDir::tempPath();
    }

    QDir().mkpath(persistentDir);
    const QString baseName = QFileInfo(filePath).completeBaseName();
    const QString sessionDirPath = persistentDir + QDir::separator() + baseName + QStringLiteral("_session");

    QDir sessionDir(sessionDirPath);
    if (sessionDir.exists()) {
        sessionDir.removeRecursively();
    }
    QDir().mkpath(sessionDirPath);

    if (!libreoffice_internal::copyRecursively(tempDir.path(), sessionDirPath)) {
        error = QObject::tr("Не удалось скопировать временные файлы LibreOffice в рабочую директорию");
        return false;
    }

    context.workingDirectory = sessionDirPath;
    context.workingFile = sessionDir.absoluteFilePath(QFileInfo(htmlFile).fileName());

    return true;
}

bool LibreOfficeHandler::save(const QString &filePath,
                              QTextDocument *document,
                              DocumentContext &context,
                              QString &error)
{
    if (!ensureLibreOfficeAvailable(error)) {
        return false;
    }

    const QString extension = libreoffice_internal::extensionFromPath(filePath);
    if (!libreoffice_internal::kLibreOfficeExtensions.contains(extension)) {
        error = QObject::tr("LibreOffice не поддерживает сохранение формата '%1'").arg(extension);
        return false;
    }

    QString workDir = context.workingDirectory;
    if (workDir.isEmpty()) {
        workDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
        if (workDir.isEmpty()) {
            workDir = QDir::tempPath();
        }
        QDir().mkpath(workDir);
        context.workingDirectory = workDir;
    }

    QString htmlPath = context.workingFile;
    if (htmlPath.isEmpty()) {
        htmlPath = workDir + QDir::separator() + QFileInfo(filePath).completeBaseName() + QStringLiteral(".html");
        context.workingFile = htmlPath;
    }

    if (QTextDocumentWriter writer(htmlPath, QByteArray("HTML")); !writer.write(document)) {
        error = QObject::tr("Не удалось сохранить HTML для конвертации");
        return false;
    }

    if (QString convertError; !convertFromHtml(htmlPath, libreoffice_internal::targetFormatForExtension(extension), filePath, convertError)) {
        error = convertError;
        return false;
    }

    return true;
}

bool LibreOfficeHandler::ensureLibreOfficeAvailable(QString &error) const
{
    if (!libreOfficeBinary_.isEmpty()) {
        return true;
    }

    error = QObject::tr(
                "Не найден исполняемый файл LibreOffice.\n"
                "Установите LibreOffice (brew install --cask libreoffice) или укажите путь через SOFFICE_PATH.\n"
                "Пример: SOFFICE_PATH=/Applications/LibreOffice.app/Contents/MacOS/soffice");
    return false;
}

QString LibreOfficeHandler::findLibreOfficeExecutable() const
{
    if (QByteArray env = qgetenv("SOFFICE_PATH"); !env.isEmpty()) {
        QFileInfo envInfo(QString::fromLocal8Bit(env));
        if (envInfo.isExecutable()) {
            return envInfo.absoluteFilePath();
        }
    }
    for (const QString &candidate : libreoffice_internal::possibleLibreOfficeBinaries()) {
        if (QFileInfo info(candidate); info.isExecutable()) {
            return info.absoluteFilePath();
        }
        const QString fromPath = QStandardPaths::findExecutable(candidate);
        if (!fromPath.isEmpty()) {
            return fromPath;
        }
    }
    return QString();
}

bool LibreOfficeHandler::convertToHtml(const QString &filePath,
                                       const QString &outputDir,
                                       QString &htmlFilePath,
                                       QString &error) const
{
    QProcess process;
    QStringList args;
    args << QStringLiteral("--headless")
         << QStringLiteral("--convert-to")
         << QStringLiteral("html:HTML")
         << filePath
         << QStringLiteral("--outdir")
         << outputDir;

    process.start(libreOfficeBinary_, args);
    if (!process.waitForFinished(-1)) {
        error = QObject::tr("LibreOffice завершается слишком долго при импорте файла");
        return false;
    }

    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        error = QObject::tr("LibreOffice не удалось конвертировать файл: %1").arg(QString::fromUtf8(process.readAllStandardError()));
        return false;
    }

    const QString baseName = QFileInfo(filePath).completeBaseName();
    QDir outDir(outputDir);
    const QStringList htmlFiles = outDir.entryList(QStringList() << baseName + QStringLiteral(".html"), QDir::Files);
    if (htmlFiles.isEmpty()) {
        error = QObject::tr("LibreOffice не создал HTML для '%1'").arg(filePath);
        return false;
    }

    htmlFilePath = outDir.absoluteFilePath(htmlFiles.first());
    return true;
}

bool LibreOfficeHandler::convertFromHtml(const QString &htmlPath,
                                         const QString &targetFormat,
                                         const QString &destinationPath,
                                         QString &error) const
{
    QDir tempDir = QFileInfo(destinationPath).dir();
    const QString outDir = tempDir.absolutePath();

    QProcess process;
    QStringList args;
    args << QStringLiteral("--headless")
         << QStringLiteral("--convert-to")
         << targetFormat
         << htmlPath
         << QStringLiteral("--outdir")
         << outDir;

    process.start(libreOfficeBinary_, args);
    if (!process.waitForFinished(-1)) {
        error = QObject::tr("LibreOffice завершается слишком долго при сохранении файла");
        return false;
    }

    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        error = QObject::tr("LibreOffice не смог конвертировать HTML в '%1': %2")
                    .arg(targetFormat, QString::fromUtf8(process.readAllStandardError()));
        return false;
    }

    const QString baseName = QFileInfo(htmlPath).completeBaseName();
    const QString generatedFile = tempDir.absoluteFilePath(baseName + QStringLiteral(".") + targetFormat);
    if (!QFile::exists(generatedFile)) {
        error = QObject::tr("LibreOffice не создал файл '%1' после конвертации").arg(generatedFile);
        return false;
    }

    if (QFile::exists(destinationPath)) {
        QFile::remove(destinationPath);
    }

    if (!QFile::rename(generatedFile, destinationPath)) {
        if (!QFile::copy(generatedFile, destinationPath)) {
            error = QObject::tr("Не удалось переместить сгенерированный файл в '%1'").arg(destinationPath);
            return false;
        }
        QFile::remove(generatedFile);
    }

    return true;
}
