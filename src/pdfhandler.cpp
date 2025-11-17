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
#include <QDir>
#include <QDialog>
#include <QVBoxLayout>
#include <QtPdf/QPdfDocument>
#include <QtPdfWidgets/QPdfView>

namespace {

bool isPdf(const QString &ext) { return ext.toLower() == QStringLiteral("pdf"); }

QString findPopplerTool(const QString &toolName)
{
    if (QString path = QStandardPaths::findExecutable(toolName); !path.isEmpty()) {
        return path;
    }
    const QStringList extraDirs = {
        QStringLiteral("/opt/homebrew/bin"),
        QStringLiteral("/usr/local/bin"),
        QStringLiteral("/usr/bin")
    };

    for (const QString &dir : extraDirs) {
        const QString candidate = dir + QDir::separator() + toolName;
        QFileInfo info(candidate);
        if (info.exists() && info.isExecutable()) {
            return info.absoluteFilePath();
        }
    }

    return QString();
}

}

bool PdfHandler::canLoad(const QString &extension) const { return isPdf(extension); }

bool PdfHandler::canSave(const QString &extension) const { return isPdf(extension); }

bool PdfHandler::ensurePdfToolsAvailable(QString &error) const
{
    const QString pdftohtml = findPopplerTool(QStringLiteral("pdftohtml"));
    if (const QString pdftotext = findPopplerTool(QStringLiteral("pdftotext"));
        pdftohtml.isEmpty() && pdftotext.isEmpty()) {
        error = QObject::tr("Не найдены утилиты pdftohtml/pdftotext (poppler). Установите: brew install poppler");
        return false;
    }
    return true;
}

bool PdfHandler::convertPdfToHtmlStdout(const QString &pdfPath, QString &htmlOut, QString &error) const
{
    if (const QString tool = findPopplerTool(QStringLiteral("pdftohtml")); !tool.isEmpty()) {
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
    }

    QString textTool = findPopplerTool(QStringLiteral("pdftotext"));
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
    auto *dialog = new QDialog();
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle(QFileInfo(filePath).fileName());

    auto *pdfDocument = new QPdfDocument(dialog);
    pdfDocument->load(filePath);
    if (pdfDocument->status() != QPdfDocument::Status::Error) {
        auto *view = new QPdfView(dialog);
        view->setDocument(pdfDocument);
        view->setZoomFactor(1.0);

        auto *layout = new QVBoxLayout(dialog);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(view);

        dialog->resize(900, 700);
        dialog->show();

        const QString infoText = QObject::tr(
            "PDF-файл открыт во встроенном просмотрщике.\n"
            "Путь: %1\n\n"
            "Редактирование содержимого PDF в этом редакторе не поддерживается."
        ).arg(filePath);

        document->setPlainText(infoText);
        document->setModified(false);
        context.isReadOnly = true;
        context.workingDirectory.clear();
        context.workingFile.clear();
        return true;
    }

    delete dialog;

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

    const qreal marginMm = 15.0;
    const qreal marginPt = 72.0 * marginMm / 25.4;
    writer.setPageMargins(QMarginsF(marginPt, marginPt, marginPt, marginPt));

    auto *doc = document;
    doc->documentLayout()->setPaintDevice(painter.device());

    const QRectF paintRect(0, 0, writer.width(), writer.height());
    int page = 0;
    qreal y = 0.0;
    while (y < doc->size().height() - 0.1) {
        painter.save();
        painter.translate(0, -y);
        doc->drawContents(&painter, paintRect);
        painter.restore();
        y += paintRect.height();
        if (y < doc->size().height() - 0.1) {
            writer.newPage();
            ++page;
        }
    }

    painter.end();
    context.isReadOnly = false;
    return true;
}


