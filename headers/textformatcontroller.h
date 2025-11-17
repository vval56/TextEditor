#ifndef TEXTFORMATCONTROLLER_H
#define TEXTFORMATCONTROLLER_H

#include <QObject>
#include <QTextCharFormat>

class QTextEdit;
class QAction;
class QFontComboBox;
class QComboBox;
class TextEditorUi;

class TextFormatController : public QObject
{
    Q_OBJECT
public:
    explicit TextFormatController(QTextEdit *textEdit,
                                  QObject *parent = nullptr);

    void attachUi(const TextEditorUi *ui);

public slots:
    void textBold();
    void textItalic();
    void textUnderline();

    void textAlignLeft();
    void textAlignCenter();
    void textAlignRight();
    void textAlignJustify();

    void textFamily(const QString &family);
    void textSize(const QString &pointSizeStr);
    void textColor();

    void mergeFormatOnWordOrSelection(const QTextCharFormat &format);
    void updateAlignmentButtons();
    void currentCharFormatChanged(const QTextCharFormat &format);

private:
    QTextEdit *textEdit_ = nullptr;
    QAction *boldAct_ = nullptr;
    QAction *italicAct_ = nullptr;
    QAction *underlineAct_ = nullptr;
    QAction *alignLeftAct_ = nullptr;
    QAction *alignCenterAct_ = nullptr;
    QAction *alignRightAct_ = nullptr;
    QAction *alignJustifyAct_ = nullptr;
    QFontComboBox *fontCombo_ = nullptr;
    QComboBox *fontSizeCombo_ = nullptr;
};

#endif
