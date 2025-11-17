#include "../headers/texteditor.h"
#include "../headers/document.h"
#include "../headers/textformatcontroller.h"
#include <QFileInfo>
#include <QRegularExpression>
#include <QColorDialog>
#include <QFontDatabase>
#include <QActionGroup>
#include <QToolButton>
#include <stdexcept>
#include <QtPdf/QPdfDocument>
#include <QtPdfWidgets/QPdfView>
#include "../headers/myvector.h"

TextEditor::TextEditor(QWidget *parent)
    : QMainWindow(parent),
    themeManager_(&ThemeManager::getInstance()),
    editToolManager_(std::make_unique<EditToolManager>()),
    speechManager(new SpeechManager(this)),
    currentFile("")
{
    centralStack = new QStackedWidget(this);
    fileController_ = std::make_unique<TextFileController>(this);

    textEdit = new QTextEdit(this);
    document_ = new Document(this);
    textEdit->setDocument(document_->qtDocument());
    ui_ = std::make_unique<TextEditorUi>(this);

    formatController_ = std::make_unique<TextFormatController>(textEdit, this);

    ui_->createActions();
    ui_->createMenus();
    ui_->createToolBars();
    ui_->createStatusBar();

    formatController_->attachUi(ui_.get());

    const auto themeNames = themeManager_->getAvailableThemes();
    for (size_t i = 0; i < themeNames.size(); ++i) {
        ui_->themeComboBox()->addItem(themeNames[i]);
    }
    if (const ITheme* currentTheme = themeManager_->getCurrentTheme()) {
        ui_->themeComboBox()->setCurrentText(currentTheme->getName());
    }

    pdfView = nullptr;
    pdfDocument = nullptr;

    centralStack->addWidget(textEdit);
    setCentralWidget(centralStack);

    QFont defaultFont("Times New Roman", 12);
    textEdit->setCurrentFont(defaultFont);
    textEdit->setFontPointSize(12);

    setupEditTools();
    setupFormatActions();
    applyTheme();

    setWindowTitle("Текстовый редактор");
    setMinimumSize(800, 600);

    connect(textEdit, &QTextEdit::textChanged, this, &TextEditor::onTextChanged);
    connect(textEdit, &QTextEdit::cursorPositionChanged, this, &TextEditor::updateStatusBar);
    connect(textEdit, &QTextEdit::currentCharFormatChanged, formatController_.get(), &TextFormatController::currentCharFormatChanged);
    connect(ui_->themeComboBox(), &QComboBox::currentTextChanged, this, &TextEditor::changeTheme);
    connect(ui_->toolsComboBox(), &QComboBox::activated, this, &TextEditor::executeEditTool);
    connect(speechManager, &SpeechManager::errorOccurred, this, &TextEditor::onSpeechError);

    autoSaveTimer = new QTimer(this);
    autoSaveTimer->setSingleShot(true);
    connect(autoSaveTimer, &QTimer::timeout, this, [this]() {
        if (autoSaveEnabled && !currentFile.isEmpty() && textEdit->document()->isModified()) {
            fileController_->saveFile();
        }
    });

    updateStatusBar();
}

TextEditor::~TextEditor() = default;

void TextEditor::speakSelectedText()
{
    if (!document_) {
        QMessageBox::warning(this, "Озвучивание", "Документ недоступен для озвучивания");
        return;
    }

    speechManager->speakFromDocument(document_);
    ui_->statusLabel()->setText("Озвучивание текста...");
}

void TextEditor::stopSpeaking()
{
    speechManager->stopSpeaking();
    ui_->statusLabel()->setText("Озвучивание остановлено");
}

void TextEditor::onSpeechError(const QString &error)
{
    QMessageBox::warning(this, "Ошибка речи", error);
    ui_->statusLabel()->setText("Ошибка: " + error);
}

void TextEditor::setupEditTools()
{
    auto tools = editToolManager_->getAvailableTools();
    for (const auto& tool : tools) {
        ui_->toolsComboBox()->addItem(tool->getName());
    }
}

void TextEditor::setupFormatActions()
{
    auto *alignGroup = new QActionGroup(this);
    alignGroup->addAction(ui_->alignLeftAct());
    alignGroup->addAction(ui_->alignCenterAct());
    alignGroup->addAction(ui_->alignRightAct());
    alignGroup->addAction(ui_->alignJustifyAct());
    ui_->alignLeftAct()->setChecked(true);
}

void TextEditor::mergeFormatOnWordOrSelection(const QTextCharFormat &format)
{
    if (formatController_) {
        formatController_->mergeFormatOnWordOrSelection(format);
    }
}

void TextEditor::applyTheme()
{
    try {
        const ITheme* theme = themeManager_->getCurrentTheme();
        setStyleSheet(theme->getStylesheet());
        updateStatusBar();
    } catch (const ThemeException& e) {
        QMessageBox::warning(this, "Ошибка темы", e.what());
    }
}

void TextEditor::startAutoSaveIfNeeded() { fileController_->startAutoSaveIfNeeded(); }
void TextEditor::stopAutoSave()          { fileController_->stopAutoSave(); }
void TextEditor::scheduleAutoSave()      { fileController_->scheduleAutoSave(); }

void TextEditor::changeTheme(const QString& themeName)
{
    handleFileOperation([this, themeName]() {
        themeManager_->setCurrentTheme(themeName);
        applyTheme();
    }, "Ошибка при смене темы");
}

void TextEditor::executeEditTool()
{
    QString toolName = ui_->toolsComboBox()->currentText();
    if (toolName != "Инструменты...") {
        editToolManager_->executeTool(toolName, document_, textEdit);
        ui_->toolsComboBox()->setCurrentIndex(0);
    }
}

void TextEditor::updateStatusBar()
{
    QString text = textEdit->toPlainText();
    const auto lines = static_cast<int>(text.count('\n')) + 1;

    const auto words = text.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    const auto wordCount = static_cast<int>(words.size());

    const auto characters = static_cast<int>(text.length());

    QString status = QString("Строк: %1 | Слов: %2 | Символов: %3 | Тема: %4")
                         .arg(lines).arg(wordCount).arg(characters)
                         .arg(themeManager_->getCurrentTheme()->getName());

    if (textEdit->document()->isModified()) {
        status += " | Изменения не сохранены";
    }

    ui_->statusLabel()->setText(status);
}

void TextEditor::onTextChanged()
{
    updateStatusBar();
    scheduleAutoSave();
}

void TextEditor::about()
{
    QMessageBox::about(this, "О программе",
                       "Текстовый редактор\n"
                       "Создан на Qt C++ с использованием:\n"
                       "- Абстрактных классов и интерфейсов\n"
                       "- STL контейнеров\n"
                       "- Полиморфизма и наследования\n"
                       "- Обработки исключений\n"
                       "Версия 2.0");
}

void TextEditor::closeEvent(QCloseEvent *event)
{
    if (speechManager) {
        speechManager->stopSpeaking();
    }
    if (const bool isUnsavedNewDoc = currentFile.isEmpty() && !textEdit->toPlainText().trimmed().isEmpty();
        textEdit->document()->isModified() || isUnsavedNewDoc) {
        auto reply = QMessageBox::question(
            this,
            "Выход",
            "Сохранить изменения перед закрытием?",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
            QMessageBox::Save
        );

        if (reply == QMessageBox::Save) {
            fileController_->saveFile();
            if (textEdit->document()->isModified()) {
                event->ignore();
                return;
            }
        } else if (reply == QMessageBox::Cancel) {
            event->ignore();
            return;
        }
    }
    stopAutoSave();
    event->accept();
}
