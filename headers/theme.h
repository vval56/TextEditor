#ifndef THEME_H
#define THEME_H

#include <QString>
#include <QColor>
#include <map>

class ITheme {
public:
    virtual ~ITheme() = default;
    virtual QString getName() const = 0;
    virtual QColor getBackgroundColor() const = 0;
    virtual QColor getTextColor() const = 0;
    virtual QColor getHighlightColor() const = 0;
    virtual QString getStylesheet() const = 0;
};

class LightTheme : public ITheme {
public:
    LightTheme() = default; 
    QString getName() const override;
    QColor getBackgroundColor() const override;
    QColor getTextColor() const override;
    QColor getHighlightColor() const override;
    QString getStylesheet() const override;
};

class DarkTheme : public ITheme {
public:
    DarkTheme() = default; 
    QString getName() const override;
    QColor getBackgroundColor() const override;
    QColor getTextColor() const override;
    QColor getHighlightColor() const override;
    QString getStylesheet() const override;
};

class BlueTheme : public ITheme {
public:
    BlueTheme() = default; 
    QString getName() const override;
    QColor getBackgroundColor() const override;
    QColor getTextColor() const override;
    QColor getHighlightColor() const override;
    QString getStylesheet() const override;
};

#endif
