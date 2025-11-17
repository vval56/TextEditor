#ifndef TEXTEDITOR_H
#define TEXTEDITOR_H

#include <QMainWindow>
#include <QTextEdit>
#include <QStackedWidget>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QFileDialog>
#include <QMessageBox>
#include <QToolBar>
#include <QStatusBar>
#include <QLabel>
#include <QComboBox>
#include <QFontComboBox>
#include <QInputDialog> 
#include <QLineEdit>
#include <QSpinBox>
#include <QTimer>
#include <QCloseEvent>
#include <memory>
#include "theme.h"
#include "thememanager.h"
#include "edittools.h"
#include "speechmanager.h"
#include "documentmanager.h"
#include "textfilecontroller.h"
#include "texteditorui.h"

class QPdfView;
class QPdfDocument;
class Document;
class TextFormatController;
class TextEditorUi;

class TextEditor : public QMainWindow
{
    Q_OBJECT

public:
    explicit TextEditor(QWidget *parent = nullptr);
    ~TextEditor() override;

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void about();
    void changeTheme(const QString& themeName);
    void executeEditTool();
    void updateStatusBar();
    void onTextChanged();

    void speakSelectedText();
    void stopSpeaking();
    void onSpeechError(const QString &error);
    

private:
    void applyTheme();
    void setupEditTools();
    void setupFormatActions();
    void mergeFormatOnWordOrSelection(const QTextCharFormat &format);
    void updateAlignmentButtons();

    template<typename Operation>
    void handleFileOperation(Operation operation, const QString& errorMessage)
    {
        try {
            operation();
        } catch (const DocumentOperationException& e) {
            QMessageBox::critical(this, "Ошибка", errorMessage + ": " + e.what());
        }
    }
    void startAutoSaveIfNeeded();
    void stopAutoSave();
    void scheduleAutoSave();

    friend class TextFileController;
    friend class TextEditorUi;

    QStackedWidget *centralStack = nullptr;
    QTextEdit *textEdit;
    Document *document_ = nullptr;
    QPdfView *pdfView = nullptr;
    QPdfDocument *pdfDocument = nullptr;

    ThemeManager* themeManager_ = &ThemeManager::getInstance();
    std::unique_ptr<EditToolManager> editToolManager_ = std::make_unique<EditToolManager>();
    SpeechManager *speechManager;

    std::unique_ptr<TextFormatController> formatController_;
    std::unique_ptr<TextFileController> fileController_;
    std::unique_ptr<TextEditorUi> ui_;

    QString currentFile;

    QTimer *autoSaveTimer;
    bool autoSaveEnabled = false;

    DocumentManager documentManager_;
};

#endif
