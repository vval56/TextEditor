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
#include "edittools.h"

class TextEditor : public QMainWindow
{
    Q_OBJECT

public:
    explicit TextEditor(QWidget *parent = nullptr);
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

    template<typename Func>
    void handleFileOperation(Func&& operation, const QString& errorMessage);

    QTextEdit *textEdit;

    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *viewMenu;
    QMenu *toolsMenu;
    QMenu *helpMenu;

    QAction *newAct;
    QAction *openAct;
    QAction *saveAct;
    QAction *saveAsAct;
    QAction *exitAct;
    QAction *cutAct;
    QAction *copyAct;
    QAction *pasteAct;
    QAction *aboutAct;

    QToolBar *fileToolBar;
    QToolBar *editToolBar;

    QLabel *statusLabel;
    QLabel *themeLabel;
    QComboBox *themeComboBox;
    QComboBox *toolsComboBox;

    ThemeManager* themeManager_;
    std::unique_ptr<EditToolManager> editToolManager_;

    QString currentFile;
};

#endif
