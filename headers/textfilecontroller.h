#ifndef TEXTFILECONTROLLER_H
#define TEXTFILECONTROLLER_H

#include <QObject>
#include <stdexcept>

class TextEditor;

class TextFileController : public QObject
{
    Q_OBJECT
public:
    explicit TextFileController(TextEditor *editor, QObject *parent = nullptr);

public slots:
    void newFile();
    void openFile();
    void saveFile();
    void saveAsFile();

    void startAutoSaveIfNeeded();
    void stopAutoSave();
    void scheduleAutoSave();

private:
    TextEditor *editor_ = nullptr;

    void openFileImpl();
};

class DocumentOperationException : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

#endif