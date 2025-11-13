#include "../headers/texteditor.h"
#include "../headers/document.h"
#include <QFileInfo>
#include <QRegularExpression>
#include <QColorDialog>
#include <QFontDatabase>
#include <QActionGroup>
#include <QToolButton>
#include <stdexcept>
#include "../headers/myvector.h"

TextEditor::TextEditor(QWidget *parent)
    : QMainWindow(parent),
    themeManager_(&ThemeManager::getInstance()),
    currentFile(""),
    editToolManager_(std::make_unique<EditToolManager>()),
    speechManager(new SpeechManager(this))
{
    textEdit = new QTextEdit(this);
    textEdit->setDocument(new Document(textEdit));
    setCentralWidget(textEdit);

    // Устанавливаем Times New Roman по умолчанию
    QFont defaultFont("Times New Roman", 12);
    textEdit->setCurrentFont(defaultFont);
    textEdit->setFontPointSize(12);

    createActions();
    createMenus();
    createToolBars();
    createStatusBar();
    setupEditTools();
    setupFormatActions();
    applyTheme();

    setWindowTitle("Текстовый редактор");
    setMinimumSize(800, 600);

    connect(textEdit, &QTextEdit::textChanged, this, &TextEditor::onTextChanged);
    connect(textEdit, &QTextEdit::cursorPositionChanged, this, &TextEditor::updateStatusBar);
    connect(textEdit, &QTextEdit::currentCharFormatChanged, this, &TextEditor::currentCharFormatChanged);
    connect(themeComboBox, &QComboBox::currentTextChanged, this, &TextEditor::changeTheme);
    connect(toolsComboBox, &QComboBox::activated, this, &TextEditor::executeEditTool);
    connect(speechManager, &SpeechManager::errorOccurred, this, &TextEditor::onSpeechError);

    // Автосохранение
    autoSaveTimer = new QTimer(this);
    autoSaveTimer->setSingleShot(true);
    connect(autoSaveTimer, &QTimer::timeout, this, [this]() {
        if (autoSaveEnabled && !currentFile.isEmpty() && textEdit->document()->isModified()) {
            saveFile();
        }
    });

    updateStatusBar();
}

TextEditor::~TextEditor() = default;

void TextEditor::createActions()
{
    // Файл с эмодзи
    newAct = new QAction("📄 Новый", this);
    newAct->setShortcut(QKeySequence::New);
    connect(newAct, &QAction::triggered, this, &TextEditor::newFile);

    openAct = new QAction("📂 Открыть", this);
    openAct->setShortcut(QKeySequence::Open);
    connect(openAct, &QAction::triggered, this, &TextEditor::openFile);

    saveAct = new QAction("💾 Сохранить", this);
    saveAct->setShortcut(QKeySequence::Save);
    connect(saveAct, &QAction::triggered, this, &TextEditor::saveFile);

    saveAsAct = new QAction("💾 Сохранить как...", this);
    saveAsAct->setShortcut(QKeySequence::SaveAs);
    connect(saveAsAct, &QAction::triggered, this, &TextEditor::saveAsFile);

    exitAct = new QAction("🚪 Выход", this);
    exitAct->setShortcut(QKeySequence::Quit);
    connect(exitAct, &QAction::triggered, this, &QWidget::close);

    // Правка с эмодзи
    undoAct = new QAction("↶ Отменить", this);
    undoAct->setShortcut(QKeySequence::Undo);
    connect(undoAct, &QAction::triggered, textEdit, &QTextEdit::undo);

    redoAct = new QAction("↷ Повторить", this);
    redoAct->setShortcut(QKeySequence::Redo);
    connect(redoAct, &QAction::triggered, textEdit, &QTextEdit::redo);

    cutAct = new QAction("✂ Вырезать", this);
    cutAct->setShortcut(QKeySequence::Cut);
    connect(cutAct, &QAction::triggered, textEdit, &QTextEdit::cut);

    copyAct = new QAction("📋 Копировать", this);
    copyAct->setShortcut(QKeySequence::Copy);
    connect(copyAct, &QAction::triggered, textEdit, &QTextEdit::copy);

    pasteAct = new QAction("📝 Вставить", this);
    pasteAct->setShortcut(QKeySequence::Paste);
    connect(pasteAct, &QAction::triggered, textEdit, &QTextEdit::paste);

    // Обеспечиваем работу Cmd+C/V/X/Z/Y в области редактора на macOS
    const auto shortcutContext = Qt::WidgetWithChildrenShortcut;
    for (QAction *act : { undoAct, redoAct, cutAct, copyAct, pasteAct }) {
        act->setShortcutContext(shortcutContext);
        textEdit->addAction(act);
    }

    // Форматирование с текстовыми иконками (буквами)
    boldAct = new QAction("B Жирный", this);
    boldAct->setShortcut(QKeySequence::Bold);
    boldAct->setCheckable(true);
    connect(boldAct, &QAction::triggered, this, &TextEditor::textBold);

    italicAct = new QAction("I Курсив", this);
    italicAct->setShortcut(QKeySequence::Italic);
    italicAct->setCheckable(true);
    connect(italicAct, &QAction::triggered, this, &TextEditor::textItalic);

    underlineAct = new QAction("U Подчеркнутый", this);
    underlineAct->setShortcut(QKeySequence::Underline);
    underlineAct->setCheckable(true);
    connect(underlineAct, &QAction::triggered, this, &TextEditor::textUnderline);

    // Выравнивание с символами
    alignLeftAct = new QAction("◀ По левому краю", this);
    alignLeftAct->setCheckable(true);
    connect(alignLeftAct, &QAction::triggered, this, &TextEditor::textAlignLeft);

    alignCenterAct = new QAction("● По центру", this);
    alignCenterAct->setCheckable(true);
    connect(alignCenterAct, &QAction::triggered, this, &TextEditor::textAlignCenter);

    alignRightAct = new QAction("▶ По правому краю", this);
    alignRightAct->setCheckable(true);
    connect(alignRightAct, &QAction::triggered, this, &TextEditor::textAlignRight);

    alignJustifyAct = new QAction("⬌ По ширине", this);
    alignJustifyAct->setCheckable(true);
    connect(alignJustifyAct, &QAction::triggered, this, &TextEditor::textAlignJustify);

    textColorAct = new QAction("A Цвет текста", this);
    connect(textColorAct, &QAction::triggered, this, &TextEditor::textColor);

    // Речь с эмодзи
    speakAct = new QAction("🔊 Озвучить текст", this);
    speakAct->setShortcut(QKeySequence("Ctrl+S"));
    connect(speakAct, &QAction::triggered, this, &TextEditor::speakSelectedText);

    stopSpeechAct = new QAction("⏹ Остановить озвучивание", this);
    connect(stopSpeechAct, &QAction::triggered, this, &TextEditor::stopSpeaking);

    // Справка
    aboutAct = new QAction("ℹ О программе", this);
    connect(aboutAct, &QAction::triggered, this, &TextEditor::about);
}

void TextEditor::createMenus()
{
    // Меню Файл с иконками
    fileMenu = menuBar()->addMenu("📁 Файл");
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    // Меню Правка с иконками
    editMenu = menuBar()->addMenu("✏ Правка");
    editMenu->addAction(undoAct);
    editMenu->addAction(redoAct);
    editMenu->addSeparator();
    editMenu->addAction(cutAct);
    editMenu->addAction(copyAct);
    editMenu->addAction(pasteAct);

    // Меню Формат с иконками
    formatMenu = menuBar()->addMenu("🎨 Формат");
    formatMenu->addAction(boldAct);
    formatMenu->addAction(italicAct);
    formatMenu->addAction(underlineAct);
    formatMenu->addSeparator();
    formatMenu->addAction(alignLeftAct);
    formatMenu->addAction(alignCenterAct);
    formatMenu->addAction(alignRightAct);
    formatMenu->addAction(alignJustifyAct);
    formatMenu->addSeparator();
    formatMenu->addAction(textColorAct);

    // Меню Речь с иконками
    speechMenu = menuBar()->addMenu("🔊 Речь");
    speechMenu->addAction(speakAct);
    speechMenu->addSeparator();
    speechMenu->addAction(stopSpeechAct);

    // Меню Справка с иконкой
    helpMenu = menuBar()->addMenu("ℹ Справка");
    helpMenu->addAction(aboutAct);
}

void TextEditor::createToolBars()
{
    // Панель файловых операций
    fileToolBar = addToolBar("Файл");
    fileToolBar->setMovable(false);
    fileToolBar->setToolButtonStyle(Qt::ToolButtonTextOnly);

    fileToolBar->addAction(newAct);
    fileToolBar->addAction(openAct);
    fileToolBar->addAction(saveAct);
    fileToolBar->addSeparator();

    // Панель редактирования
    editToolBar = addToolBar("Правка");
    editToolBar->setMovable(false);
    editToolBar->setToolButtonStyle(Qt::ToolButtonTextOnly);

    editToolBar->addAction(undoAct);
    editToolBar->addAction(redoAct);
    editToolBar->addSeparator();
    editToolBar->addAction(cutAct);
    editToolBar->addAction(copyAct);
    editToolBar->addAction(pasteAct);

    // Панель форматирования
    formatToolBar = addToolBar("Форматирование");
    formatToolBar->setMovable(false);
    formatToolBar->setToolButtonStyle(Qt::ToolButtonTextOnly);

    // Шрифт
    fontCombo = new QFontComboBox();
    fontCombo->setCurrentFont(QFont("Times New Roman"));
    fontCombo->setMaximumWidth(150);
    formatToolBar->addWidget(fontCombo);
    connect(fontCombo, &QFontComboBox::currentTextChanged, this, &TextEditor::textFamily);

    // Размер шрифта
    fontSizeCombo = new QComboBox();
    fontSizeCombo->setEditable(true);
    fontSizeCombo->setMaximumWidth(50);

    // Стандартные размеры шрифтов
    QFontDatabase db;
    foreach(int size, db.standardSizes())
        fontSizeCombo->addItem(QString::number(size));

    fontSizeCombo->setCurrentText("12");
    formatToolBar->addWidget(fontSizeCombo);
    connect(fontSizeCombo, &QComboBox::currentTextChanged, this, &TextEditor::textSize);

    formatToolBar->addSeparator();

    // Кнопки форматирования текста
    formatToolBar->addAction(boldAct);
    formatToolBar->addAction(italicAct);
    formatToolBar->addAction(underlineAct);
    formatToolBar->addSeparator();

    // Кнопки выравнивания
    formatToolBar->addAction(alignLeftAct);
    formatToolBar->addAction(alignCenterAct);
    formatToolBar->addAction(alignRightAct);
    formatToolBar->addAction(alignJustifyAct);
    formatToolBar->addSeparator();

    // Кнопка цвета текста с выпадающим меню
    QToolButton *colorButton = new QToolButton();
    colorButton->setDefaultAction(textColorAct);
    colorButton->setPopupMode(QToolButton::MenuButtonPopup);

    // Создаем меню с базовыми цветами
    QMenu *colorMenu = new QMenu(this);

    // Базовые цвета
    QList<QColor> basicColors = {
        Qt::black, Qt::white, Qt::red, Qt::darkRed,
        Qt::green, Qt::darkGreen, Qt::blue, Qt::darkBlue,
        Qt::cyan, Qt::darkCyan, Qt::magenta, Qt::darkMagenta,
        Qt::yellow, Qt::darkYellow, Qt::gray, Qt::darkGray
    };

    QList<QString> colorNames = {
        "Черный", "Белый", "Красный", "Темно-красный",
        "Зеленый", "Темно-зеленый", "Синий", "Темно-синий",
        "Голубой", "Темно-голубой", "Пурпурный", "Темно-пурпурный",
        "Желтый", "Темно-желтый", "Серый", "Темно-серый"
    };

    for (int i = 0; i < basicColors.size(); ++i) {
        QAction *colorAction = new QAction(colorNames[i], this);

        // Создаем иконку цвета
        QPixmap pixmap(16, 16);
        pixmap.fill(basicColors[i]);
        colorAction->setIcon(QIcon(pixmap));

        colorAction->setData(basicColors[i]);
        colorMenu->addAction(colorAction);

        connect(colorAction, &QAction::triggered, this, [this, basicColors, i]() {
            QTextCharFormat fmt;
            fmt.setForeground(basicColors[i]);
            mergeFormatOnWordOrSelection(fmt);
        });
    }

    // Добавляем действие для выбора произвольного цвета
    QAction *customColorAction = new QAction("Другой цвет...", this);
    colorMenu->addAction(customColorAction);
    connect(customColorAction, &QAction::triggered, this, &TextEditor::textColor);

    colorButton->setMenu(colorMenu);
    formatToolBar->addWidget(colorButton);

    // Панель речи
    speechToolBar = addToolBar("Речь");
    speechToolBar->setMovable(false);
    speechToolBar->setToolButtonStyle(Qt::ToolButtonTextOnly);
    speechToolBar->addAction(speakAct);
    speechToolBar->addAction(stopSpeechAct);
}

// Методы для речи
void TextEditor::speakSelectedText()
{
    QString textToSpeak;
    QTextCursor cursor = textEdit->textCursor();

    if (cursor.hasSelection()) {
        textToSpeak = cursor.selectedText();
    } else {
        textToSpeak = textEdit->toPlainText();
    }

    if (!textToSpeak.isEmpty()) {
        speechManager->speakText(textToSpeak);
        statusLabel->setText("Озвучивание текста...");
    } else {
        QMessageBox::information(this, "Озвучивание", "Нет текста для озвучивания");
    }
}

void TextEditor::stopSpeaking()
{
    speechManager->stopSpeaking();
    statusLabel->setText("Озвучивание остановлено");
}

void TextEditor::onSpeechError(const QString &error)
{
    QMessageBox::warning(this, "Ошибка речи", error);
    statusLabel->setText("Ошибка: " + error);
}


// Остальные методы остаются без изменений...

// ... остальные методы остаются без изменений ...
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

void TextEditor::setupFormatActions()
{
    // Группируем действия выравнивания
    QActionGroup *alignGroup = new QActionGroup(this);
    alignGroup->addAction(alignLeftAct);
    alignGroup->addAction(alignCenterAct);
    alignGroup->addAction(alignRightAct);
    alignGroup->addAction(alignJustifyAct);
    alignLeftAct->setChecked(true);
}

void TextEditor::mergeFormatOnWordOrSelection(const QTextCharFormat &format)
{
    QTextCursor cursor = textEdit->textCursor();
    if (!cursor.hasSelection())
        cursor.select(QTextCursor::WordUnderCursor);
    cursor.mergeCharFormat(format);
    textEdit->mergeCurrentCharFormat(format);
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

void TextEditor::handleFileOperation(const std::function<void()>& operation, const QString& errorMessage)
{
    try {
        operation();
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Ошибка", errorMessage + ": " + e.what());
    }
}

void TextEditor::startAutoSaveIfNeeded()
{
    autoSaveEnabled = !currentFile.isEmpty();
    if (autoSaveEnabled) {
        scheduleAutoSave();
    } else {
        stopAutoSave();
    }
}

void TextEditor::stopAutoSave()
{
    autoSaveEnabled = false;
    autoSaveTimer->stop();
}

void TextEditor::scheduleAutoSave()
{
    if (autoSaveEnabled) {
        autoSaveTimer->start(3000);
    }
}

// Методы форматирования текста
void TextEditor::textBold()
{
    QTextCharFormat fmt;
    fmt.setFontWeight(boldAct->isChecked() ? QFont::Bold : QFont::Normal);
    mergeFormatOnWordOrSelection(fmt);
}

void TextEditor::textItalic()
{
    QTextCharFormat fmt;
    fmt.setFontItalic(italicAct->isChecked());
    mergeFormatOnWordOrSelection(fmt);
}

void TextEditor::textUnderline()
{
    QTextCharFormat fmt;
    fmt.setFontUnderline(underlineAct->isChecked());
    mergeFormatOnWordOrSelection(fmt);
}

void TextEditor::textFamily(const QString &f)
{
    QTextCharFormat fmt;
    fmt.setFontFamily(f);
    mergeFormatOnWordOrSelection(fmt);
}

void TextEditor::textSize(const QString &p)
{
    qreal pointSize = p.toDouble();
    if (p.toFloat() > 0) {
        QTextCharFormat fmt;
        fmt.setFontPointSize(pointSize);
        mergeFormatOnWordOrSelection(fmt);
    }
}

void TextEditor::textColor()
{
    QColor col = QColorDialog::getColor(textEdit->textColor(), this);
    if (col.isValid()) {
        QTextCharFormat fmt;
        fmt.setForeground(col);
        mergeFormatOnWordOrSelection(fmt);
    }
}

void TextEditor::updateAlignmentButtons()
{
    Qt::Alignment alignment = textEdit->alignment();
    alignLeftAct->setChecked(alignment & Qt::AlignLeft);
    alignCenterAct->setChecked(alignment & Qt::AlignHCenter);
    alignRightAct->setChecked(alignment & Qt::AlignRight);
    alignJustifyAct->setChecked(alignment & Qt::AlignJustify);
}

void TextEditor::textAlignLeft()
{
    textEdit->setAlignment(Qt::AlignLeft);
    updateAlignmentButtons();
}

void TextEditor::textAlignCenter()
{
    textEdit->setAlignment(Qt::AlignCenter);
    updateAlignmentButtons();
}

void TextEditor::textAlignRight()
{
    textEdit->setAlignment(Qt::AlignRight);
    updateAlignmentButtons();
}

void TextEditor::textAlignJustify()
{
    textEdit->setAlignment(Qt::AlignJustify);
    updateAlignmentButtons();
}



void TextEditor::currentCharFormatChanged(const QTextCharFormat &format)
{
    fontCombo->setCurrentFont(format.font());
    fontSizeCombo->setEditText(QString::number(format.fontPointSize()));
    boldAct->setChecked(format.font().bold());
    italicAct->setChecked(format.font().italic());
    underlineAct->setChecked(format.font().underline());

    // Обновляем кнопки выравнивания
    Qt::Alignment alignment = textEdit->alignment();
    alignLeftAct->setChecked(alignment & Qt::AlignLeft);
    alignCenterAct->setChecked(alignment & Qt::AlignHCenter);
    alignRightAct->setChecked(alignment & Qt::AlignRight);
    alignJustifyAct->setChecked(alignment & Qt::AlignJustify);
}

// ... остальные методы (newFile, openFile, saveFile, etc.) остаются без изменений ...
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
        setWindowTitle("Текстовый редактор - Новый файл");
        statusLabel->setText("Новый файл создан");
        stopAutoSave();
        documentManager_.context() = DocumentContext{};
    }, "Ошибка при создании файла");
}

void TextEditor::openFile()
{
    handleFileOperation([this]() {
        QString fileName = QFileDialog::getOpenFileName(this,
                                                        "Открыть файл",
                                                        QString(),
                                                        documentManager_.filterForOpenDialog());

        if (!fileName.isEmpty()) {
            QString error;
            if (!documentManager_.loadDocument(fileName, textEdit->document(), error)) {
                throw std::runtime_error(error.toStdString());
            }

            currentFile = fileName;
            setWindowTitle("Текстовый редактор - " + QFileInfo(fileName).fileName());
            statusLabel->setText("Файл открыт: " + fileName);
            textEdit->document()->setModified(false);
            startAutoSaveIfNeeded();
        }
    }, "Ошибка при открытии файла");
}

void TextEditor::saveFile()
{
    handleFileOperation([this]() {
        if (currentFile.isEmpty()) {
            saveAsFile();
        } else {
            QString error;
            if (!documentManager_.saveDocument(currentFile, textEdit->document(), error)) {
                throw std::runtime_error(error.toStdString());
            }

            textEdit->document()->setModified(false);
            statusLabel->setText("Файл сохранен: " + currentFile);
        }
    }, "Ошибка при сохранении файла");
}

void TextEditor::saveAsFile()
{
    handleFileOperation([this]() {
        QString fileName = QFileDialog::getSaveFileName(this,
                                                        "Сохранить как",
                                                        currentFile,
                                                        documentManager_.filterForSaveDialog());

        if (!fileName.isEmpty()) {
            QString error;
            if (!documentManager_.saveDocument(fileName, textEdit->document(), error)) {
                throw std::runtime_error(error.toStdString());
            }

            currentFile = fileName;
            textEdit->document()->setModified(false);
            setWindowTitle("Текстовый редактор - " + QFileInfo(fileName).fileName());
            statusLabel->setText("Файл сохранен: " + currentFile);
            startAutoSaveIfNeeded();
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
    // Остановим озвучивание/диктовку перед закрытием окна, чтобы завершить QProcess
    if (speechManager) {
        speechManager->stopSpeaking();
    }
    const bool isUnsavedNewDoc = currentFile.isEmpty() && !textEdit->toPlainText().trimmed().isEmpty();
    if (textEdit->document()->isModified() || isUnsavedNewDoc) {
        auto reply = QMessageBox::question(
            this,
            "Выход",
            "Сохранить изменения перед закрытием?",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
            QMessageBox::Save
        );

        if (reply == QMessageBox::Save) {
            saveFile();
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
