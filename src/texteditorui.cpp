#include "../headers/texteditorui.h"
#include "../headers/texteditor.h"
#include "../headers/textformatcontroller.h"
#include <QMenuBar>
#include <QToolButton>
#include <QColorDialog>
#include <QFontDatabase>

TextEditorUi::TextEditorUi(TextEditor *owner)
    : owner_(owner)
{
}

void TextEditorUi::createActions()
{
    file_.newAct = new QAction("📄 Новый", owner_);
    file_.newAct->setShortcut(QKeySequence::New);
    QObject::connect(file_.newAct, &QAction::triggered,
                     owner_->fileController_.get(), &TextFileController::newFile);

    file_.openAct = new QAction("📂 Открыть", owner_);
    file_.openAct->setShortcut(QKeySequence::Open);
    QObject::connect(file_.openAct, &QAction::triggered,
                     owner_->fileController_.get(), &TextFileController::openFile);

    file_.saveAct = new QAction("💾 Сохранить", owner_);
    file_.saveAct->setShortcut(QKeySequence::Save);
    QObject::connect(file_.saveAct, &QAction::triggered,
                     owner_->fileController_.get(), &TextFileController::saveFile);

    file_.saveAsAct = new QAction("💾 Сохранить как...", owner_);
    file_.saveAsAct->setShortcut(QKeySequence::SaveAs);
    QObject::connect(file_.saveAsAct, &QAction::triggered,
                     owner_->fileController_.get(), &TextFileController::saveAsFile);

    file_.exitAct = new QAction("🚪 Выход", owner_);
    file_.exitAct->setShortcut(QKeySequence::Quit);
    QObject::connect(file_.exitAct, &QAction::triggered, owner_, &QWidget::close);

    edit_.undoAct = new QAction("↶ Отменить", owner_);
    edit_.undoAct->setShortcut(QKeySequence::Undo);
    QObject::connect(edit_.undoAct, &QAction::triggered, owner_->textEdit, &QTextEdit::undo);

    edit_.redoAct = new QAction("↷ Повторить", owner_);
    edit_.redoAct->setShortcut(QKeySequence::Redo);
    QObject::connect(edit_.redoAct, &QAction::triggered, owner_->textEdit, &QTextEdit::redo);

    edit_.cutAct = new QAction("✂ Вырезать", owner_);
    edit_.cutAct->setShortcut(QKeySequence::Cut);
    QObject::connect(edit_.cutAct, &QAction::triggered, owner_->textEdit, &QTextEdit::cut);

    edit_.copyAct = new QAction("📋 Копировать", owner_);
    edit_.copyAct->setShortcut(QKeySequence::Copy);
    QObject::connect(edit_.copyAct, &QAction::triggered, owner_->textEdit, &QTextEdit::copy);

    edit_.pasteAct = new QAction("📝 Вставить", owner_);
    edit_.pasteAct->setShortcut(QKeySequence::Paste);
    QObject::connect(edit_.pasteAct, &QAction::triggered, owner_->textEdit, &QTextEdit::paste);

    const auto shortcutContext = Qt::WidgetWithChildrenShortcut;
    for (QAction *act : { edit_.undoAct, edit_.redoAct, edit_.cutAct, edit_.copyAct, edit_.pasteAct }) {
        act->setShortcutContext(shortcutContext);
        owner_->textEdit->addAction(act);
    }

    format_.boldAct = new QAction("B Жирный", owner_);
    format_.boldAct->setShortcut(QKeySequence::Bold);
    format_.boldAct->setCheckable(true);
    QObject::connect(format_.boldAct, &QAction::triggered, owner_->formatController_.get(), &TextFormatController::textBold);

    format_.italicAct = new QAction("I Курсив", owner_);
    format_.italicAct->setShortcut(QKeySequence::Italic);
    format_.italicAct->setCheckable(true);
    QObject::connect(format_.italicAct, &QAction::triggered, owner_->formatController_.get(), &TextFormatController::textItalic);

    format_.underlineAct = new QAction("U Подчеркнутый", owner_);
    format_.underlineAct->setShortcut(QKeySequence::Underline);
    format_.underlineAct->setCheckable(true);
    QObject::connect(format_.underlineAct, &QAction::triggered, owner_->formatController_.get(), &TextFormatController::textUnderline);

    format_.alignLeftAct = new QAction("◀ По левому краю", owner_);
    format_.alignLeftAct->setCheckable(true);
    QObject::connect(format_.alignLeftAct, &QAction::triggered, owner_->formatController_.get(), &TextFormatController::textAlignLeft);

    format_.alignCenterAct = new QAction("● По центру", owner_);
    format_.alignCenterAct->setCheckable(true);
    QObject::connect(format_.alignCenterAct, &QAction::triggered, owner_->formatController_.get(), &TextFormatController::textAlignCenter);

    format_.alignRightAct = new QAction("▶ По правому краю", owner_);
    format_.alignRightAct->setCheckable(true);
    QObject::connect(format_.alignRightAct, &QAction::triggered, owner_->formatController_.get(), &TextFormatController::textAlignRight);

    format_.alignJustifyAct = new QAction("⬌ По ширине", owner_);
    format_.alignJustifyAct->setCheckable(true);
    QObject::connect(format_.alignJustifyAct, &QAction::triggered,
                     owner_->formatController_.get(), &TextFormatController::textAlignJustify);

    format_.textColorAct = new QAction("A Цвет текста", owner_);
    QObject::connect(format_.textColorAct, &QAction::triggered, owner_->formatController_.get(), &TextFormatController::textColor);

    speech_.speakAct = new QAction("🔊 Озвучить текст", owner_);
    speech_.speakAct->setShortcut(QKeySequence("Ctrl+S"));
    QObject::connect(speech_.speakAct, &QAction::triggered, owner_, &TextEditor::speakSelectedText);

    speech_.stopSpeechAct = new QAction("⏹ Остановить озвучивание", owner_);
    QObject::connect(speech_.stopSpeechAct, &QAction::triggered, owner_, &TextEditor::stopSpeaking);

    speech_.aboutAct = new QAction("ℹ О программе", owner_);
    QObject::connect(speech_.aboutAct, &QAction::triggered, owner_, &TextEditor::about);
}

void TextEditorUi::createMenus()
{
    QMenuBar *mb = owner_->menuBar();

    file_.fileMenu = mb->addMenu("📁 Файл");
    file_.fileMenu->addAction(file_.newAct);
    file_.fileMenu->addAction(file_.openAct);
    file_.fileMenu->addAction(file_.saveAct);
    file_.fileMenu->addAction(file_.saveAsAct);
    file_.fileMenu->addSeparator();
    file_.fileMenu->addAction(file_.exitAct);

    edit_.editMenu = mb->addMenu("✏ Правка");
    edit_.editMenu->addAction(edit_.undoAct);
    edit_.editMenu->addAction(edit_.redoAct);
    edit_.editMenu->addSeparator();
    edit_.editMenu->addAction(edit_.cutAct);
    edit_.editMenu->addAction(edit_.copyAct);
    edit_.editMenu->addAction(edit_.pasteAct);

    format_.formatMenu = mb->addMenu("🎨 Формат");
    format_.formatMenu->addAction(format_.boldAct);
    format_.formatMenu->addAction(format_.italicAct);
    format_.formatMenu->addAction(format_.underlineAct);
    format_.formatMenu->addSeparator();
    format_.formatMenu->addAction(format_.alignLeftAct);
    format_.formatMenu->addAction(format_.alignCenterAct);
    format_.formatMenu->addAction(format_.alignRightAct);
    format_.formatMenu->addAction(format_.alignJustifyAct);
    format_.formatMenu->addSeparator();
    format_.formatMenu->addAction(format_.textColorAct);

    speech_.speechMenu = mb->addMenu("🔊 Речь");
    speech_.speechMenu->addAction(speech_.speakAct);
    speech_.speechMenu->addSeparator();
    speech_.speechMenu->addAction(speech_.stopSpeechAct);

    helpMenu = mb->addMenu("ℹ Справка");
    helpMenu->addAction(speech_.aboutAct);
}

void TextEditorUi::createToolBars()
{
    file_.fileToolBar = owner_->addToolBar("Файл");
    file_.fileToolBar->setMovable(false);
    file_.fileToolBar->setToolButtonStyle(Qt::ToolButtonTextOnly);

    file_.fileToolBar->addAction(file_.newAct);
    file_.fileToolBar->addAction(file_.openAct);
    file_.fileToolBar->addAction(file_.saveAct);
    file_.fileToolBar->addSeparator();

    edit_.editToolBar = owner_->addToolBar("Правка");
    edit_.editToolBar->setMovable(false);
    edit_.editToolBar->setToolButtonStyle(Qt::ToolButtonTextOnly);

    edit_.editToolBar->addAction(edit_.undoAct);
    edit_.editToolBar->addAction(edit_.redoAct);
    edit_.editToolBar->addSeparator();
    edit_.editToolBar->addAction(edit_.cutAct);
    edit_.editToolBar->addAction(edit_.copyAct);
    edit_.editToolBar->addAction(edit_.pasteAct);

    format_.formatToolBar = owner_->addToolBar("Форматирование");
    format_.formatToolBar->setMovable(false);
    format_.formatToolBar->setToolButtonStyle(Qt::ToolButtonTextOnly);

    format_.fontCombo = new QFontComboBox();
    format_.fontCombo->setCurrentFont(QFont("Times New Roman"));
    format_.fontCombo->setMaximumWidth(150);
    format_.formatToolBar->addWidget(format_.fontCombo);
    QObject::connect(format_.fontCombo, &QFontComboBox::currentTextChanged,
                     owner_->formatController_.get(), &TextFormatController::textFamily);

    format_.fontSizeCombo = new QComboBox();
    format_.fontSizeCombo->setEditable(true);
    format_.fontSizeCombo->setMaximumWidth(50);

    for (const int size : QFontDatabase::standardSizes()) {
        format_.fontSizeCombo->addItem(QString::number(size));
    }

    format_.fontSizeCombo->setCurrentText("12");
    format_.formatToolBar->addWidget(format_.fontSizeCombo);
    QObject::connect(format_.fontSizeCombo, &QComboBox::currentTextChanged, owner_->formatController_.get(), &TextFormatController::textSize);

    format_.formatToolBar->addSeparator();

    format_.formatToolBar->addAction(format_.boldAct);
    format_.formatToolBar->addAction(format_.italicAct);
    format_.formatToolBar->addAction(format_.underlineAct);
    format_.formatToolBar->addSeparator();
    format_.formatToolBar->addAction(format_.alignLeftAct);
    format_.formatToolBar->addAction(format_.alignCenterAct);
    format_.formatToolBar->addAction(format_.alignRightAct);
    format_.formatToolBar->addAction(format_.alignJustifyAct);
    format_.formatToolBar->addSeparator();

    auto *colorButton = new QToolButton();
    colorButton->setDefaultAction(format_.textColorAct);
    colorButton->setPopupMode(QToolButton::MenuButtonPopup);
    
    auto *colorMenu = new QMenu(owner_);

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

    const auto n = std::min(basicColors.size(), colorNames.size());
    for (qsizetype i = 0; i < n; ++i) {
        auto *colorAction = new QAction(colorNames[i], owner_);

        QPixmap pixmap(16, 16);
        pixmap.fill(basicColors[i]);
        colorAction->setIcon(QIcon(pixmap));

        colorAction->setData(basicColors[i]);
        colorMenu->addAction(colorAction);

        QObject::connect(colorAction, &QAction::triggered, owner_->formatController_.get(),
                         [this, basicColors, i]() {
            QTextCharFormat fmt;
            fmt.setForeground(basicColors[i]);
            owner_->formatController_->mergeFormatOnWordOrSelection(fmt);
        });
    }

    auto *customColorAction = new QAction("Другой цвет...", owner_);
    colorMenu->addAction(customColorAction);
    QObject::connect(customColorAction, &QAction::triggered,
                     owner_->formatController_.get(), &TextFormatController::textColor);

    colorButton->setMenu(colorMenu);
    format_.formatToolBar->addWidget(colorButton);

    speech_.speechToolBar = owner_->addToolBar("Речь");
    speech_.speechToolBar->setMovable(false);
    speech_.speechToolBar->setToolButtonStyle(Qt::ToolButtonTextOnly);
    speech_.speechToolBar->addAction(speech_.speakAct);
    speech_.speechToolBar->addAction(speech_.stopSpeechAct);
}

void TextEditorUi::createStatusBar()
{
    statusBar_.statusLabel = new QLabel("Готов");
    owner_->statusBar()->addWidget(statusBar_.statusLabel, 1);

    statusBar_.themeLabel = new QLabel("Тема:");
    owner_->statusBar()->addWidget(statusBar_.themeLabel);

    statusBar_.themeComboBox = new QComboBox();
    owner_->statusBar()->addWidget(statusBar_.themeComboBox);

    statusBar_.toolsComboBox = new QComboBox();
    statusBar_.toolsComboBox->addItem("Инструменты...");
    owner_->statusBar()->addWidget(statusBar_.toolsComboBox);
}
