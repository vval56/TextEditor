#include "../headers/textfilecontroller.h"
#include "../headers/texteditor.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QtPdf/QPdfDocument>
#include <QtPdfWidgets/QPdfView>

TextFileController::TextFileController(TextEditor *editor, QObject *parent)
    : QObject(parent)
    , editor_(editor)
{
}

void TextFileController::newFile()
{
    editor_->handleFileOperation([this]() {
        editor_->centralStack->setCurrentWidget(editor_->textEdit);
        if (editor_->textEdit->document()->isModified()) {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(editor_, "Создать новый файл",
                                          "Сохранить изменения?",
                                          QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
            if (reply == QMessageBox::Save) {
                saveFile();
            } else if (reply == QMessageBox::Cancel) {
                return;
            }
        }

        editor_->textEdit->clear();
        editor_->currentFile = "";
        editor_->setWindowTitle("Текстовый редактор - Новый файл");
        editor_->ui_->statusLabel()->setText("Новый файл создан");
        stopAutoSave();
        editor_->documentManager_.context() = DocumentContext{};
    }, "Ошибка при создании файла");
}

void TextFileController::openFile()
{
    editor_->handleFileOperation([this]() { openFileImpl(); }, "Ошибка при открытии файла");
}

void TextFileController::openFileImpl()
{
    QString fileName = QFileDialog::getOpenFileName(editor_,
                                                    "Открыть файл",
                                                    QString(),
                                                    editor_->documentManager_.filterForOpenDialog());

    if (fileName.isEmpty()) {
        return;
    }

    const QFileInfo info(fileName);

    if (const QString ext = info.suffix().toLower(); ext == "pdf") {
        if (!editor_->pdfDocument) {
            editor_->pdfDocument = new QPdfDocument(editor_);
        }
        if (!editor_->pdfView) {
            editor_->pdfView = new QPdfView(editor_);
            editor_->pdfView->setDocument(editor_->pdfDocument);
            editor_->pdfView->setPageMode(QPdfView::PageMode::MultiPage);
            editor_->centralStack->addWidget(editor_->pdfView);
        }

        editor_->pdfDocument->load(fileName);
        if (editor_->pdfDocument->status() == QPdfDocument::Status::Error) {
            QMessageBox::warning(editor_, "Ошибка открытия PDF",
                                 "Не удалось загрузить PDF-файл: " + fileName);
            return;
        }

        editor_->centralStack->setCurrentWidget(editor_->pdfView);
        editor_->currentFile = fileName;
        editor_->setWindowTitle("Текстовый редактор - " + info.fileName());
        editor_->ui_->statusLabel()->setText("PDF открыт: " + fileName);
        stopAutoSave();
        return;
    }

    if (QString error; !editor_->documentManager_.loadDocument(fileName, editor_->textEdit->document(), error)) {
        throw DocumentOperationException(error.toStdString());
    }

    editor_->centralStack->setCurrentWidget(editor_->textEdit);
    editor_->currentFile = fileName;
    editor_->setWindowTitle("Текстовый редактор - " + info.fileName());
    editor_->ui_->statusLabel()->setText("Файл открыт: " + fileName);
    editor_->textEdit->document()->setModified(false);
    startAutoSaveIfNeeded();
}

void TextFileController::saveFile()
{
    editor_->handleFileOperation([this]() {
        if (editor_->currentFile.isEmpty()) {
            saveAsFile();
        } else {
            if (QString error; !editor_->documentManager_.saveDocument(editor_->currentFile, editor_->textEdit->document(), error)) {
                throw DocumentOperationException(error.toStdString());
            }

            editor_->textEdit->document()->setModified(false);
            editor_->ui_->statusLabel()->setText("Файл сохранен: " + editor_->currentFile);
        }
    }, "Ошибка при сохранении файла");
}

void TextFileController::saveAsFile()
{
    editor_->handleFileOperation([this]() {
        QString fileName = QFileDialog::getSaveFileName(editor_,
                                                        "Сохранить как",
                                                        editor_->currentFile,
                                                        editor_->documentManager_.filterForSaveDialog());

        if (!fileName.isEmpty()) {
            if (QString error; !editor_->documentManager_.saveDocument(fileName, editor_->textEdit->document(), error)) {
                throw DocumentOperationException(error.toStdString());
            }

            editor_->currentFile = fileName;
            editor_->textEdit->document()->setModified(false);
            editor_->setWindowTitle("Текстовый редактор - " + QFileInfo(fileName).fileName());
            editor_->ui_->statusLabel()->setText("Файл сохранен: " + editor_->currentFile);
            startAutoSaveIfNeeded();
        }
    }, "Ошибка при сохранении файла как");
}

void TextFileController::startAutoSaveIfNeeded()
{
    editor_->autoSaveEnabled = !editor_->currentFile.isEmpty();
    if (editor_->autoSaveEnabled) {
        scheduleAutoSave();
    } else {
        stopAutoSave();
    }
}

void TextFileController::stopAutoSave()
{
    editor_->autoSaveEnabled = false;
    if (editor_->autoSaveTimer) {
        editor_->autoSaveTimer->stop();
    }
}

void TextFileController::scheduleAutoSave()
{
    if (editor_->autoSaveEnabled && editor_->autoSaveTimer) {
        editor_->autoSaveTimer->start(3000);
    }
}
