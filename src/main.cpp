#include <QApplication>
#include <QMessageBox>
#include "../headers/texteditor.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    try {
        TextEditor editor;
        editor.show();
        return QApplication::exec();
    } catch (const std::exception& e) {
        QMessageBox::critical(nullptr, "Fatal Error",
                              QString("Application failed to start: %1").arg(e.what()));
        return 1;
    }
}
