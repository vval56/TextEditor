#ifndef EDITTOOLS_H
#define EDITTOOLS_H

#include <QTextEdit>
#include <QString>
#include <memory>
#include <vector>

class IEditTool {
public:
    virtual ~IEditTool() = default;
    virtual QString getName() const = 0;
    virtual void execute(QTextEdit* textEdit) = 0;
    virtual bool canExecute(QTextEdit* textEdit) const = 0;
};

class UpperCaseTool : public IEditTool {
public:
    QString getName() const override { return "To Upper Case"; }
    void execute(QTextEdit* textEdit) override;
    bool canExecute(QTextEdit* textEdit) const override;
};

class LowerCaseTool : public IEditTool {
public:
    QString getName() const override { return "To Lower Case"; }
    void execute(QTextEdit* textEdit) override;
    bool canExecute(QTextEdit* textEdit) const override;
};

class WordCountTool : public IEditTool {
public:
    QString getName() const override { return "Word Count"; }
    void execute(QTextEdit* textEdit) override;
    bool canExecute(QTextEdit* textEdit) const override { return true; }
};

class DuplicateLineTool : public IEditTool {
public:
    QString getName() const override { return "Duplicate Line"; }
    void execute(QTextEdit* textEdit) override;
    bool canExecute(QTextEdit* textEdit) const override { return true; }
};

class EditToolManager {
public:
    EditToolManager();

    void registerTool(std::unique_ptr<IEditTool> tool);
    std::vector<IEditTool*> getAvailableTools() const;
    void executeTool(const QString& name, QTextEdit* textEdit);

private:
    std::vector<std::unique_ptr<IEditTool>> tools_;
};

#endif