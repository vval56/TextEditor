#include "../headers/edittools.h"
#include <QTextCursor>
#include <QMessageBox>
#include <QRegularExpression>

void UpperCaseTool::execute(QTextEdit* textEdit) {
    QString selectedText = textEdit->textCursor().selectedText();
    if (!selectedText.isEmpty()) {
        textEdit->textCursor().insertText(selectedText.toUpper());
    }
}

bool UpperCaseTool::canExecute(QTextEdit* textEdit) const {
    return !textEdit->textCursor().selectedText().isEmpty();
}

void LowerCaseTool::execute(QTextEdit* textEdit) {
    QString selectedText = textEdit->textCursor().selectedText();
    if (!selectedText.isEmpty()) {
        textEdit->textCursor().insertText(selectedText.toLower());
    }
}

bool LowerCaseTool::canExecute(QTextEdit* textEdit) const {
    return !textEdit->textCursor().selectedText().isEmpty();
}

void WordCountTool::execute(QTextEdit* textEdit) {
    QString text = textEdit->toPlainText();
    qsizetype wordCount = 0;
    qsizetype charCount = text.length();
    qsizetype lineCount = text.count('\n') + 1;

    QStringList words = text.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    wordCount = words.size();

    QMessageBox::information(nullptr, "Word Count",
                             QString("Words: %1\nCharacters: %2\nLines: %3")
                                 .arg(static_cast<qlonglong>(wordCount))
                                 .arg(static_cast<qlonglong>(charCount))
                                 .arg(static_cast<qlonglong>(lineCount)));
}

void DuplicateLineTool::execute(QTextEdit* textEdit) {
    QTextCursor cursor = textEdit->textCursor();
    cursor.select(QTextCursor::LineUnderCursor);
    QString lineText = cursor.selectedText();

    cursor.movePosition(QTextCursor::EndOfLine);
    cursor.insertText("\n" + lineText);
}

EditToolManager::EditToolManager() {
    registerTool(std::make_unique<UpperCaseTool>());
    registerTool(std::make_unique<LowerCaseTool>());
    registerTool(std::make_unique<WordCountTool>());
    registerTool(std::make_unique<DuplicateLineTool>());
}

void EditToolManager::registerTool(std::unique_ptr<IEditTool> tool) {
    tools_.push_back(std::move(tool));
}

std::vector<IEditTool*> EditToolManager::getAvailableTools() const {
    std::vector<IEditTool*> availableTools;
    for (const auto& tool : tools_) {
        availableTools.push_back(tool.get());
    }
    return availableTools;
}

void EditToolManager::executeTool(const QString& name, QTextEdit* textEdit) const {
    for (const auto& tool : tools_) {
        if (tool->getName() == name && tool->canExecute(textEdit)) {
            tool->execute(textEdit);
            return;
        }
    }
}
