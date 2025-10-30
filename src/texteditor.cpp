#include "../headers/texteditor.h"
#include <QTextStream>
#include <QFileInfo>
#include <QRegularExpression>
#include <stdexcept>
#include "../headers/myvector.h" 

TextEditor::TextEditor(QWidget *parent)
    : QMainWindow(parent),
    themeManager_(&ThemeManager::getInstance()),
    currentFile(""),
    editToolManager_(std::make_unique<EditToolManager>())
{
    textEdit = new QTextEdit(this);
    setCentralWidget(textEdit);

    createActions();
    createMenus();
    createToolBars();
    createStatusBar();
    setupEditTools();
    applyTheme();

    setWindowTitle("Продвинутый текстовый редактор");
    setMinimumSize(800, 600);

    connect(textEdit, &QTextEdit::textChanged, this, &TextEditor::onTextChanged);
    connect(textEdit, &QTextEdit::cursorPositionChanged, this, &TextEditor::updateStatusBar);
    connect(themeComboBox, &QComboBox::currentTextChanged, this, &TextEditor::changeTheme);
    connect(toolsComboBox, &QComboBox::activated, this, &TextEditor::executeEditTool);

    updateStatusBar();
}

TextEditor::~TextEditor() = default;

void TextEditor::createActions()
{
    newAct = new QAction("Создать", this);
    newAct->setShortcut(QKeySequence::New);
    connect(newAct, &QAction::triggered, this, &TextEditor::newFile);

    openAct = new QAction("Открыть", this);
    openAct->setShortcut(QKeySequence::Open);
    connect(openAct, &QAction::triggered, this, &TextEditor::openFile);

    saveAct = new QAction("Сохранить", this);
    saveAct->setShortcut(QKeySequence::Save);
    connect(saveAct, &QAction::triggered, this, &TextEditor::saveFile);

    saveAsAct = new QAction("Сохранить как...", this);
    saveAsAct->setShortcut(QKeySequence::SaveAs);
    connect(saveAsAct, &QAction::triggered, this, &TextEditor::saveAsFile);

    exitAct = new QAction("Выход", this);
    exitAct->setShortcut(QKeySequence::Quit);
    connect(exitAct, &QAction::triggered, this, &QWidget::close);

    // Правка
    cutAct = new QAction("Вырезать", this);
    cutAct->setShortcut(QKeySequence::Cut);
    connect(cutAct, &QAction::triggered, textEdit, &QTextEdit::cut);

    copyAct = new QAction("Копировать", this);
    copyAct->setShortcut(QKeySequence::Copy);
    connect(copyAct, &QAction::triggered, textEdit, &QTextEdit::copy);

    pasteAct = new QAction("Вставить", this);
    pasteAct->setShortcut(QKeySequence::Paste);
    connect(pasteAct, &QAction::triggered, textEdit, &QTextEdit::paste);

    // Справка
    aboutAct = new QAction("О программе", this);
    connect(aboutAct, &QAction::triggered, this, &TextEditor::about);
}

void TextEditor::createMenus()
{
    fileMenu = menuBar()->addMenu("Файл");
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    editMenu = menuBar()->addMenu("Правка");
    editMenu->addAction(cutAct);
    editMenu->addAction(copyAct);
    editMenu->addAction(pasteAct);

    viewMenu = menuBar()->addMenu("Вид");

    toolsMenu = menuBar()->addMenu("Инструменты");

    helpMenu = menuBar()->addMenu("Справка");
    helpMenu->addAction(aboutAct);
}

void TextEditor::createToolBars()
{
    fileToolBar = addToolBar("Файл");
    fileToolBar->addAction(newAct);
    fileToolBar->addAction(openAct);
    fileToolBar->addAction(saveAct);

    editToolBar = addToolBar("Правка");
    editToolBar->addAction(cutAct);
    editToolBar->addAction(copyAct);
    editToolBar->addAction(pasteAct);
}

void TextEditor::createStatusBar()
{
    statusLabel = new QLabel("Готов");
    statusBar()->addWidget(statusLabel, 1);
    
    themeLabel = new QLabel("Тема:");
    statusBar()->addWidget(themeLabel);
    
    themeComboBox = new QComboBox();
    
    MyVector<QString> themes = themeManager_->getAvailableThemes();
    
    std::cout << "Available themes (" << themes.size() << "): ";
    themes.print();
    
    for (auto it = themes.begin(); it != themes.end(); ++it) {
        themeComboBox->addItem(*it);
    }
    
    themeComboBox->setCurrentText(themeManager_->getCurrentTheme()->getName());
    statusBar()->addWidget(themeComboBox);
    
    toolsComboBox = new QComboBox();
    toolsComboBox->addItem("Инструменты...");
    statusBar()->addWidget(toolsComboBox);
}

void TextEditor::setupEditTools()
{
    auto tools = editToolManager_->getAvailableTools();
    for (const auto& tool : tools) {
        toolsComboBox->addItem(tool->getName());
    }
}

void TextEditor::applyTheme()
{
    try {
        ITheme* theme = themeManager_->getCurrentTheme();
        setStyleSheet(theme->getStylesheet());
        updateStatusBar();
    } catch (const ThemeException& e) {
        QMessageBox::warning(this, "Ошибка темы", e.what());
    }
}

template<typename Func>
void TextEditor::handleFileOperation(Func&& operation, const QString& errorMessage)
{
    try {
        std::forward<Func>(operation)();
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Ошибка", errorMessage + ": " + e.what());
    }
}

void TextEditor::newFile()
{
    handleFileOperation([this]() {
        if (textEdit->document()->isModified()) {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, "Создать новый файл",
                                          "Сохранить изменения?",
                                          QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
            if (reply == QMessageBox::Save) {
                saveFile();
            } else if (reply == QMessageBox::Cancel) {
                return;
            }
        }

        textEdit->clear();
        currentFile = "";
        setWindowTitle("Продвинутый текстовый редактор - Новый файл");
        statusLabel->setText("Новый файл создан");
    }, "Ошибка при создании файла");
}

void TextEditor::openFile()
{
    handleFileOperation([this]() {
        QString fileName = QFileDialog::getOpenFileName(this, "Открыть файл", "",
                                                        "Текстовые файлы (*.txt);;Все файлы (*)");

        if (!fileName.isEmpty()) {
            QFile file(fileName);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);
                textEdit->setPlainText(in.readAll());
                file.close();

                currentFile = fileName;
                setWindowTitle("Продвинутый текстовый редактор - " + QFileInfo(fileName).fileName());
                statusLabel->setText("Файл открыт: " + fileName);
                textEdit->document()->setModified(false);
            } else {
                throw std::runtime_error("Не удалось открыть файл для чтения");
            }
        }
    }, "Ошибка при открытии файла");
}

void TextEditor::saveFile()
{
    handleFileOperation([this]() {
        if (currentFile.isEmpty()) {
            saveAsFile();
        } else {
            QFile file(currentFile);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&file);
                out << textEdit->toPlainText();
                file.close();

                textEdit->document()->setModified(false);
                statusLabel->setText("Файл сохранен: " + currentFile);
            } else {
                throw std::runtime_error("Не удалось открыть файл для записи");
            }
        }
    }, "Ошибка при сохранении файла");
}

void TextEditor::saveAsFile()
{
    handleFileOperation([this]() {
        QString fileName = QFileDialog::getSaveFileName(this, "Сохранить как", "",
                                                        "Текстовые файлы (*.txt);;Все файлы (*)");

        if (!fileName.isEmpty()) {
            currentFile = fileName;
            saveFile();
            setWindowTitle("Продвинутый текстовый редактор - " + QFileInfo(fileName).fileName());
        }
    }, "Ошибка при сохранении файла как");
}

void TextEditor::changeTheme(const QString& themeName)
{
    handleFileOperation([this, themeName]() {
        themeManager_->setCurrentTheme(themeName);
        applyTheme();
    }, "Ошибка при смене темы");
}

void TextEditor::executeEditTool()
{
    QString toolName = toolsComboBox->currentText();
    if (toolName != "Инструменты...") {
        editToolManager_->executeTool(toolName, textEdit);
        toolsComboBox->setCurrentIndex(0);
    }
}

void TextEditor::updateStatusBar()
{
    QString text = textEdit->toPlainText();
    int lines = text.count('\n') + 1;

    QStringList words = text.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    int wordCount = words.size();

    int characters = text.length();

    QString status = QString("Строк: %1 | Слов: %2 | Символов: %3 | Тема: %4")
                         .arg(lines).arg(wordCount).arg(characters)
                         .arg(themeManager_->getCurrentTheme()->getName());

    if (textEdit->document()->isModified()) {
        status += " | Изменения не сохранены";
    }

    statusLabel->setText(status);
}

void TextEditor::onTextChanged()
{
    updateStatusBar();
}

void TextEditor::about()
{
    QMessageBox::about(this, "О программе",
                       "Продвинутый текстовый редактор\n"
                       "Создан на Qt C++ с использованием:\n"
                       "- Абстрактных классов и интерфейсов\n"
                       "- STL контейнеров\n"
                       "- Полиморфизма и наследования\n"
                       "- Обработки исключений\n"
                       "Версия 2.0");
}
