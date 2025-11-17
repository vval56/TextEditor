#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include "theme.h"
#include "myvector.h" 
#include <memory>
#include <unordered_map>
#include <stdexcept>

class ThemeManager {
public:
    ThemeManager();
    
    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;
    
    static ThemeManager& getInstance();
    
    void registerTheme(std::unique_ptr<ITheme> theme);
    MyVector<QString> getAvailableThemes() const; 
    ITheme* getTheme(const QString& name) const;
    void setCurrentTheme(const QString& name);
    ITheme* getCurrentTheme() const;
    
private:
    std::unordered_map<QString, std::unique_ptr<ITheme>> themes_;
    ITheme* currentTheme_ = nullptr;
    
    void initializeDefaultThemes();
};

class ThemeException : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class ThemeNotFoundException : public ThemeException {
public:
    explicit ThemeNotFoundException(const QString& themeName)
        : ThemeException("Theme not found: " + themeName.toStdString()) {}
};

#endif