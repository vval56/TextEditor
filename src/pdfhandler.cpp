#include "pdfhandler.h"

#include <QTextDocument>
#include <QProcess>
#include <QFileInfo>
#include <QTextStream>
#include <QTemporaryDir>
#include <QPdfWriter>
#include <QPainter>
#include <QStandardPaths>
#include <QPageSize>
#include <QAbstractTextDocumentLayout>

namespace {

bool isPdf(const QString &ext) { return ext.toLower() == QStringLiteral("pdf"); }

}

bool PdfHandler::canLoad(const QString &extension) const { return isPdf(extension); }

bool PdfHandler::canSave(const QString &extension) const { return isPdf(extension); }

bool PdfHandler::ensurePdfToolsAvailable(QString &error) const
{
    const QString pdftohtml = QStandardPaths::findExecutable(QStringLiteral("pdftohtml"));
    const QString pdftotext = QStandardPaths::findExecutable(QStringLiteral("pdftotext"));
    if (pdftohtml.isEmpty() && pdftotext.isEmpty()) {
        error = QObject::tr("Не найдены утилиты pdftohtml/pdftotext (poppler). Установите: brew install poppler");
        return false;
    }
    return true;
}

bool PdfHandler::convertPdfToHtmlStdout(const QString &pdfPath, QString &htmlOut, QString &error) const
{
    QString tool = QStandardPaths::findExecutable(QStringLiteral("pdftohtml"));
    if (!tool.isEmpty()) {
        QProcess p;
        QStringList args;
        args << QStringLiteral("-nodrm") << QStringLiteral("-i") << QStringLiteral("-noframes")
             << QStringLiteral("-stdout") << pdfPath;
        p.start(tool, args);
        if (!p.waitForFinished(-1)) {
            error = QObject::tr("pdftohtml выполняется слишком долго");
            return false;
        }
        if (p.exitStatus() == QProcess::NormalExit && p.exitCode() == 0) {
            htmlOut = QString::fromUtf8(p.readAllStandardOutput());
            if (!htmlOut.trimmed().isEmpty()) {
                return true;
            }
        }
        // иначе попробуем текст
    }

    // Фоллбек: pdftotext -> простой HTML
    QString textTool = QStandardPaths::findExecutable(QStringLiteral("pdftotext"));
    if (textTool.isEmpty()) {
        error = QObject::tr("Не удалось конвертировать PDF в HTML: отсутствует pdftohtml/pdftotext");
        return false;
    }
    QProcess p2;
    QStringList args2;
    args2 << QStringLiteral("-layout") << QStringLiteral("-enc") << QStringLiteral("UTF-8")
          << pdfPath << QStringLiteral("-");
    p2.start(textTool, args2);
    if (!p2.waitForFinished(-1)) {
        error = QObject::tr("pdftotext выполняется слишком долго");
        return false;
    }
    if (p2.exitStatus() != QProcess::NormalExit || p2.exitCode() != 0) {
        error = QObject::tr("pdftotext вернул ошибку: %1").arg(QString::fromUtf8(p2.readAllStandardError()));
        return false;
    }
    const QString text = QString::fromUtf8(p2.readAllStandardOutput());
    htmlOut = QStringLiteral("<pre>%1</pre>").arg(text.toHtmlEscaped());
    return true;
}

bool PdfHandler::load(const QString &filePath,
                      QTextDocument *document,
                      DocumentContext &context,
                      QString &error)
{
    if (!ensurePdfToolsAvailable(error)) {
        return false;
    }

    QString html;
    if (!convertPdfToHtmlStdout(filePath, html, error)) {
        return false;
    }

    document->setHtml(html);
    document->setModified(false);
    context.isReadOnly = false;
    context.workingDirectory.clear();
    context.workingFile.clear();
    return true;
}

bool PdfHandler::save(const QString &filePath,
                      QTextDocument *document,
                      DocumentContext &context,
                      QString &error)
{
    QPdfWriter writer(filePath);
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setResolution(300);
    QPainter painter(&writer);
    if (!painter.isActive()) {
        error = QObject::tr("Не удалось создать PDF для записи");
        return false;
    }

    // Простейший рендеринг QTextDocument постранично
    QRectF pageRect = QRectF(QPointF(0, 0), QSizeF(writer.width(), writer.height()));
    QTextDocument *doc = document;
    doc->documentLayout()->setPaintDevice(painter.device());

    int page = 0;
    qreal y = 0.0;
    while (y < doc->size().height() - 0.1) {
        painter.save();
        painter.translate(0, -y);
        doc->drawContents(&painter, pageRect);
        painter.restore();
        y += pageRect.height();
        if (y < doc->size().height() - 0.1) {
            writer.newPage();
            ++page;
        }
    }

    painter.end();
    context.isReadOnly = false;
    return true;
}


