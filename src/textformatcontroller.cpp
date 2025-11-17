#include "../headers/textformatcontroller.h"
#include "../headers/texteditorui.h"

#include <QTextEdit>
#include <QTextCursor>
#include <QFontComboBox>
#include <QComboBox>
#include <QFont>
#include <QColorDialog>

TextFormatController::TextFormatController(QTextEdit *textEdit,
                                           QObject *parent)
    : QObject(parent)
    , textEdit_(textEdit)
    , boldAct_(nullptr)
    , italicAct_(nullptr)
    , underlineAct_(nullptr)
    , alignLeftAct_(nullptr)
    , alignCenterAct_(nullptr)
    , alignRightAct_(nullptr)
    , alignJustifyAct_(nullptr)
    , fontCombo_(nullptr)
    , fontSizeCombo_(nullptr)
{
}

void TextFormatController::attachUi(const TextEditorUi *ui)
{
    if (!ui) return;

    boldAct_        = ui->boldAct();
    italicAct_      = ui->italicAct();
    underlineAct_   = ui->underlineAct();
    alignLeftAct_   = ui->alignLeftAct();
    alignCenterAct_ = ui->alignCenterAct();
    alignRightAct_  = ui->alignRightAct();
    alignJustifyAct_= ui->alignJustifyAct();
    fontCombo_      = ui->fontCombo();
    fontSizeCombo_  = ui->fontSizeCombo();

    if (boldAct_)      QObject::connect(boldAct_,      &QAction::triggered, this, &TextFormatController::textBold);
    if (italicAct_)    QObject::connect(italicAct_,    &QAction::triggered, this, &TextFormatController::textItalic);
    if (underlineAct_) QObject::connect(underlineAct_, &QAction::triggered, this, &TextFormatController::textUnderline);

    if (alignLeftAct_)    QObject::connect(alignLeftAct_,   &QAction::triggered, this, &TextFormatController::textAlignLeft);
    if (alignCenterAct_)  QObject::connect(alignCenterAct_, &QAction::triggered, this, &TextFormatController::textAlignCenter);
    if (alignRightAct_)   QObject::connect(alignRightAct_,  &QAction::triggered, this, &TextFormatController::textAlignRight);
    if (alignJustifyAct_) QObject::connect(alignJustifyAct_,&QAction::triggered, this, &TextFormatController::textAlignJustify);

    if (fontCombo_) {
        QObject::connect(fontCombo_, &QFontComboBox::currentTextChanged,
                         this, &TextFormatController::textFamily);
    }
    if (fontSizeCombo_) {
        QObject::connect(fontSizeCombo_, &QComboBox::currentTextChanged,
                         this, &TextFormatController::textSize);
    }
}

void TextFormatController::mergeFormatOnWordOrSelection(const QTextCharFormat &format)
{
    if (!textEdit_) return;
    QTextCursor cursor = textEdit_->textCursor();
    if (!cursor.hasSelection())
        cursor.select(QTextCursor::WordUnderCursor);
    cursor.mergeCharFormat(format);
    textEdit_->mergeCurrentCharFormat(format);
}

void TextFormatController::textBold()
{
    if (!boldAct_) return;
    QTextCharFormat fmt;
    fmt.setFontWeight(boldAct_->isChecked() ? QFont::Bold : QFont::Normal);
    mergeFormatOnWordOrSelection(fmt);
}

void TextFormatController::textItalic()
{
    if (!italicAct_) return;
    QTextCharFormat fmt;
    fmt.setFontItalic(italicAct_->isChecked());
    mergeFormatOnWordOrSelection(fmt);
}

void TextFormatController::textUnderline()
{
    if (!underlineAct_) return;
    QTextCharFormat fmt;
    fmt.setFontUnderline(underlineAct_->isChecked());
    mergeFormatOnWordOrSelection(fmt);
}

void TextFormatController::textFamily(const QString &family)
{
    QTextCharFormat fmt;
    fmt.setFontFamilies({family});
    mergeFormatOnWordOrSelection(fmt);
}

void TextFormatController::textSize(const QString &pointSizeStr)
{
    bool ok = false;
    const qreal pointSize = pointSizeStr.toDouble(&ok);
    if (ok && pointSize > 0) {
        QTextCharFormat fmt;
        fmt.setFontPointSize(pointSize);
        mergeFormatOnWordOrSelection(fmt);
    }
}

void TextFormatController::updateAlignmentButtons()
{
    if (!textEdit_) return;
    const Qt::Alignment alignment = textEdit_->alignment();
    if (alignLeftAct_)   alignLeftAct_->setChecked(alignment & Qt::AlignLeft);
    if (alignCenterAct_) alignCenterAct_->setChecked(alignment & Qt::AlignHCenter);
    if (alignRightAct_)  alignRightAct_->setChecked(alignment & Qt::AlignRight);
    if (alignJustifyAct_) alignJustifyAct_->setChecked(alignment & Qt::AlignJustify);
}

void TextFormatController::textAlignLeft()
{
    if (!textEdit_) return;
    textEdit_->setAlignment(Qt::AlignLeft);
    updateAlignmentButtons();
}

void TextFormatController::textAlignCenter()
{
    if (!textEdit_) return;
    textEdit_->setAlignment(Qt::AlignCenter);
    updateAlignmentButtons();
}

void TextFormatController::textAlignRight()
{
    if (!textEdit_) return;
    textEdit_->setAlignment(Qt::AlignRight);
    updateAlignmentButtons();
}

void TextFormatController::textAlignJustify()
{
    if (!textEdit_) return;
    textEdit_->setAlignment(Qt::AlignJustify);
    updateAlignmentButtons();
}

void TextFormatController::textColor()
{
    if (!textEdit_) return;
    const QColor col = QColorDialog::getColor(textEdit_->textColor(), textEdit_);
    if (!col.isValid()) {
        return;
    }

    QTextCharFormat fmt;
    fmt.setForeground(col);
    mergeFormatOnWordOrSelection(fmt);
}

void TextFormatController::currentCharFormatChanged(const QTextCharFormat &format)
{
    if (fontCombo_) {
        fontCombo_->setCurrentFont(format.font());
    }
    if (fontSizeCombo_) {
        fontSizeCombo_->setEditText(QString::number(format.fontPointSize()));
    }
    if (boldAct_)      boldAct_->setChecked(format.font().bold());
    if (italicAct_)    italicAct_->setChecked(format.font().italic());
    if (underlineAct_) underlineAct_->setChecked(format.font().underline());
    updateAlignmentButtons();
}
