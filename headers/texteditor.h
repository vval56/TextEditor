#ifndef TEXTEDITOR_H
#define TEXTEDITOR_H

#include <QMainWindow>
#include <QTextEdit>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QToolBar>
#include <QStatusBar>
#include <QLabel>
#include <QComboBox>
#include <memory>
#include "theme.h"
#include "thememanager.h"
#include "edittols.h"

class TextEditor : public QMainWindow
{
    Q_OBJECT

public:
    TextEditor(QWidget *parent = nullptr);
    ~TextEditor();

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

private:
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void applyTheme();
    void setupEditTools();

    void handleFileOperation(const std::function<void()>& operation, const QString& errorMessage);

    // Основные компоненты
    QTextEdit *textEdit;

    // Меню
    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *viewMenu;
    QMenu *toolsMenu;
    QMenu *helpMenu;

    // Действия
    QAction *newAct;
    QAction *openAct;
    QAction *saveAct;
    QAction *saveAsAct;
    QAction *exitAct;
    QAction *cutAct;
    QAction *copyAct;
    QAction *pasteAct;
    QAction *aboutAct;

    // Панели инструментов
    QToolBar *fileToolBar;
    QToolBar *editToolBar;

    // Статус бар
    QLabel *statusLabel;
    QLabel *themeLabel;
    QComboBox *themeComboBox;
    QComboBox *toolsComboBox;

    // Менеджеры - используем указатель
    ThemeManager* themeManager_;
    std::unique_ptr<EditToolManager> editToolManager_;

    QString currentFile;
};

#endif
