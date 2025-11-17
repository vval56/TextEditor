#include "../headers/edittools.h"
#include <QTextCursor>
#include <QMessageBox>
#include <QRegularExpression>

void UpperCaseTool::execute(IDocument* document, QTextEdit* textEdit) {
    (void)textEdit;
    if (!document) {
        return;
    }
    const QString selectedText = document->getSelectedText();
    if (!selectedText.isEmpty()) {
        document->insertTextAtCursor(selectedText.toUpper());
    }
}

bool UpperCaseTool::canExecute(IDocument* document, QTextEdit* textEdit) const {
    (void)textEdit;
    return document && !document->getSelectedText().isEmpty();
}

void LowerCaseTool::execute(IDocument* document, QTextEdit* textEdit) {
    (void)textEdit;
    if (!document) {
        return;
    }
    const QString selectedText = document->getSelectedText();
    if (!selectedText.isEmpty()) {
        document->insertTextAtCursor(selectedText.toLower());
    }
}

bool LowerCaseTool::canExecute(IDocument* document, QTextEdit* textEdit) const {
    (void)textEdit;
    return document && !document->getSelectedText().isEmpty();
}

void WordCountTool::execute(IDocument* document, QTextEdit* textEdit) {
    (void)textEdit;
    if (!document) {
        return;
    }
    const QString text = document->getPlainText();
    qsizetype wordCount = 0;
    qsizetype charCount = text.length();
    qsizetype lineCount = text.count('\n') + 1;

    QStringList words = text.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    wordCount = words.size();

    QMessageBox::information(nullptr, "Word Count",
                             QString("Words: %1\nCharacters: %2\nLines: %3")
                                 .arg(wordCount)
                                 .arg(charCount)
                                 .arg(lineCount));
}

void DuplicateLineTool::execute(IDocument* document, QTextEdit* textEdit) {
    (void)document;
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

void EditToolManager::executeTool(const QString& name, IDocument* document, QTextEdit* textEdit) const {
    for (const auto& tool : tools_) {
        if (tool->getName() == name && tool->canExecute(document, textEdit)) {
            tool->execute(document, textEdit);
            return;
        }
    }
}
