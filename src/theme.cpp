#include "../headers/theme.h"

QString LightTheme::getName() const {
    return "Light";
}

QColor LightTheme::getBackgroundColor() const {
    return QColor(255, 255, 255);
}

QColor LightTheme::getTextColor() const {
    return QColor(0, 0, 0);
}

QColor LightTheme::getHighlightColor() const {
    return QColor(173, 216, 230);
}

QString LightTheme::getStylesheet() const {
    return R"(
        QMainWindow { background-color: white; }
        QTextEdit {
            background-color: white;
            color: black;
            border: 1px solid #ccc;
            font-family: "Monospace";
        }
        QMenuBar { background-color: #f0f0f0; }
    )";
}

QString DarkTheme::getName() const {
    return "Dark";
}

QColor DarkTheme::getBackgroundColor() const {
    return QColor(53, 53, 53);
}

QColor DarkTheme::getTextColor() const {
    return QColor(255, 255, 255);
}

QColor DarkTheme::getHighlightColor() const {
    return QColor(42, 130, 218);
}

QString DarkTheme::getStylesheet() const {
    return R"(
        QMainWindow { background-color: #353535; }
        QTextEdit {
            background-color: #353535;
            color: white;
            border: 1px solid #555;
            font-family: "Monospace";
        }
        QMenuBar { background-color: #2b2b2b; color: white; }
    )";
}

QString BlueTheme::getName() const {
    return "Blue";
}

QColor BlueTheme::getBackgroundColor() const {
    return QColor(240, 248, 255);
}

QColor BlueTheme::getTextColor() const {
    return QColor(25, 25, 112);
}

QColor BlueTheme::getHighlightColor() const {
    return QColor(135, 206, 250);
}

QString BlueTheme::getStylesheet() const {
    return R"(
        QMainWindow { background-color: #f0f8ff; }
        QTextEdit {
            background-color: #f0f8ff;
            color: #191970;
            border: 1px solid #87ceeb;
            font-family: "Monospace";
        }
        QMenuBar { background-color: #e6f3ff; }
    )";
}
