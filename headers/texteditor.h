#ifndef TEXTEDITOR_H
#define TEXTEDITOR_H

#include <QMainWindow>
#include <QTextEdit>
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
#include <QInputDialog>  // Добавь эту строку
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

class TextEditor : public QMainWindow
{
    Q_OBJECT

public:
    explicit TextEditor(QWidget *parent = nullptr);
    ~TextEditor();
    
protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void newFile();
    void openFile();
    void saveFile();
    void saveAsFile();
    void about();
    void changeTheme(const QString& themeName);
    void executeEditTool();
    void updateStatusBar();
    void onTextChanged();

    // Форматирование текста
    void textBold();
    void textItalic();
    void textUnderline();
    void textAlignLeft();
    void textAlignCenter();
    void textAlignRight();
    void textAlignJustify();
    void textFamily(const QString &f);
    void textSize(const QString &p);
    void textColor();
    void currentCharFormatChanged(const QTextCharFormat &format);

    // Речь
    void speakSelectedText();
    void stopSpeaking();
    void onSpeechError(const QString &error);
    

private:
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void applyTheme();
    void setupEditTools();
    void setupFormatActions();
    void mergeFormatOnWordOrSelection(const QTextCharFormat &format);
    void updateAlignmentButtons();

    void handleFileOperation(const std::function<void()>& operation, const QString& errorMessage);
    void startAutoSaveIfNeeded();
    void stopAutoSave();
    void scheduleAutoSave();

    QTextEdit *textEdit;

    // Меню (только в строке меню)
    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *formatMenu;
    QMenu *viewMenu;
    QMenu *toolsMenu;
    QMenu *helpMenu;
    QMenu *speechMenu;

    // Файл
    QAction *newAct;
    QAction *openAct;
    QAction *saveAct;
    QAction *saveAsAct;
    QAction *exitAct;

    // Правка
    QAction *cutAct;
    QAction *copyAct;
    QAction *pasteAct;
    QAction *undoAct;
    QAction *redoAct;

    // Форматирование
    QAction *boldAct;
    QAction *italicAct;
    QAction *underlineAct;
    QAction *alignLeftAct;
    QAction *alignCenterAct;
    QAction *alignRightAct;
    QAction *alignJustifyAct;
    QAction *textColorAct;

    // Речь
    QAction *speakAct;
    QAction *stopSpeechAct;

    // Справка
    QAction *aboutAct;

    // Панели инструментов
    QToolBar *fileToolBar;
    QToolBar *editToolBar;
    QToolBar *formatToolBar;
    QToolBar *speechToolBar;

    // Элементы форматирования
    QFontComboBox *fontCombo;
    QComboBox *fontSizeCombo;

    QLabel *statusLabel;
    QLabel *themeLabel;
    QComboBox *themeComboBox;
    QComboBox *toolsComboBox;

    ThemeManager* themeManager_;
    std::unique_ptr<EditToolManager> editToolManager_;
    SpeechManager *speechManager;

    QString currentFile;

    // Автосохранение для открытых из файла документов
    QTimer *autoSaveTimer;
    bool autoSaveEnabled = false;

    DocumentManager documentManager_;
};

#endif
