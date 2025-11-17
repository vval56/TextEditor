#ifndef EDITTOOLS_H
#define EDITTOOLS_H

#include <QTextEdit>
#include <QString>
#include <memory>
#include <vector>
#include "idocument.h"

class IEditTool {
public:
    virtual ~IEditTool() = default;
    virtual QString getName() const = 0;
    virtual void execute(IDocument* document, QTextEdit* textEdit) = 0;
    virtual bool canExecute(IDocument* document, QTextEdit* textEdit) const = 0;
};

class UpperCaseTool : public IEditTool {
public:
    QString getName() const override { return "To Upper Case"; }
    void execute(IDocument* document, QTextEdit* textEdit) override;
    bool canExecute(IDocument* document, QTextEdit* textEdit) const override;
};

class LowerCaseTool : public IEditTool {
public:
    QString getName() const override { return "To Lower Case"; }
    void execute(IDocument* document, QTextEdit* textEdit) override;
    bool canExecute(IDocument* document, QTextEdit* textEdit) const override;
};

class WordCountTool : public IEditTool {
public:
    QString getName() const override { return "Word Count"; }
    void execute(IDocument* document, QTextEdit* textEdit) override;
    bool canExecute(IDocument* document, QTextEdit* textEdit) const override {
        (void)document;
        (void)textEdit;
        return true;
    }
};

class DuplicateLineTool : public IEditTool {
public:
    QString getName() const override { return "Duplicate Line"; }
    void execute(IDocument* document, QTextEdit* textEdit) override;
    bool canExecute(IDocument* document, QTextEdit* textEdit) const override {
        (void)document;
        (void)textEdit;
        return true;
    }
};

class EditToolManager {
public:
    EditToolManager();

    void registerTool(std::unique_ptr<IEditTool> tool);
    std::vector<IEditTool*> getAvailableTools() const;
    void executeTool(const QString& name, IDocument* document, QTextEdit* textEdit) const;

private:
    std::vector<std::unique_ptr<IEditTool>> tools_;
};

#endif