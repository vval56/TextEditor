#ifndef TEXTEDITORUI_H
#define TEXTEDITORUI_H

#include <memory>
#include <QMenu>
#include <QToolBar>
#include <QLabel>
#include <QComboBox>
#include <QFontComboBox>
#include <QAction>

class TextEditor;

class TextEditorUi
{
public:
    explicit TextEditorUi(TextEditor *owner);

    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();

    QLabel *statusLabel() const { return statusBar_.statusLabel; }
    QLabel *themeLabel() const  { return statusBar_.themeLabel; }
    QComboBox *themeComboBox() const { return statusBar_.themeComboBox; }
    QComboBox *toolsComboBox() const { return statusBar_.toolsComboBox; }
    QFontComboBox *fontCombo() const { return format_.fontCombo; }
    QComboBox *fontSizeCombo() const { return format_.fontSizeCombo; }

    QAction *boldAct() const { return format_.boldAct; }
    QAction *italicAct() const { return format_.italicAct; }
    QAction *underlineAct() const { return format_.underlineAct; }
    QAction *alignLeftAct() const { return format_.alignLeftAct; }
    QAction *alignCenterAct() const { return format_.alignCenterAct; }
    QAction *alignRightAct() const { return format_.alignRightAct; }
    QAction *alignJustifyAct() const { return format_.alignJustifyAct; }
    QAction *textColorAct() const { return format_.textColorAct; }

private:
    TextEditor *owner_ = nullptr;

    QMenu *viewMenu = nullptr;
    QMenu *toolsMenu = nullptr;
    QMenu *helpMenu = nullptr;

    struct FileUi {
        QMenu *fileMenu = nullptr;

        QAction *newAct = nullptr;
        QAction *openAct = nullptr;
        QAction *saveAct = nullptr;
        QAction *saveAsAct = nullptr;
        QAction *exitAct = nullptr;

        QToolBar *fileToolBar = nullptr;
    };

    FileUi file_;

    struct EditUi {
        QMenu *editMenu = nullptr;
        QAction *cutAct = nullptr;
        QAction *copyAct = nullptr;
        QAction *pasteAct = nullptr;
        QAction *undoAct = nullptr;
        QAction *redoAct = nullptr;

        QToolBar *editToolBar = nullptr;
    };

    EditUi edit_;

    struct FormatUi {
        QMenu *formatMenu = nullptr;
        QAction *boldAct = nullptr;
        QAction *italicAct = nullptr;
        QAction *underlineAct = nullptr;
        QAction *alignLeftAct = nullptr;
        QAction *alignCenterAct = nullptr;
        QAction *alignRightAct = nullptr;
        QAction *alignJustifyAct = nullptr;
        QAction *textColorAct = nullptr;

        QToolBar *formatToolBar = nullptr;

        QFontComboBox *fontCombo = nullptr;
        QComboBox *fontSizeCombo = nullptr;
    };

    FormatUi format_;

    struct SpeechUi {
        QMenu *speechMenu = nullptr;
        QAction *speakAct = nullptr;
        QAction *stopSpeechAct = nullptr;
        QAction *aboutAct = nullptr;

        QToolBar *speechToolBar = nullptr;
    };

    SpeechUi speech_;

    struct StatusBarUi {
        QLabel *statusLabel = nullptr;
        QLabel *themeLabel = nullptr;
        QComboBox *themeComboBox = nullptr;
        QComboBox *toolsComboBox = nullptr;
    };

    StatusBarUi statusBar_;
};

#endif
