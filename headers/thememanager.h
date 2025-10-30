#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include "theme.h"
#include "myvector.h"  // Наш собственный контейнер
#include <memory>
#include <unordered_map>
#include <stdexcept>

class ThemeManager {
public:
    ThemeManager();
    
    // Запрещаем копирование и присваивание
    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;
    
    // Получение экземпляра (Singleton)
    static ThemeManager& getInstance();
    
    // Работа с темами - используем наш MyVector вместо std::vector
    void registerTheme(std::unique_ptr<ITheme> theme);
    MyVector<QString> getAvailableThemes() const;  // Изменили на MyVector
    ITheme* getTheme(const QString& name) const;
    void setCurrentTheme(const QString& name);
    ITheme* getCurrentTheme() const;
    
private:
    std::unordered_map<QString, std::unique_ptr<ITheme>> themes_;
    ITheme* currentTheme_;
    
    void initializeDefaultThemes();
};

// Исключения
class ThemeException : public std::runtime_error {
public:
    explicit ThemeException(const std::string& message) 
        : std::runtime_error(message) {}
};

class ThemeNotFoundException : public ThemeException {
public:
    explicit ThemeNotFoundException(const QString& themeName)
        : ThemeException("Theme not found: " + themeName.toStdString()) {}
};

#endif