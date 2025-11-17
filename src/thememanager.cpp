#include "../headers/thememanager.h"
#include <stdexcept>

ThemeManager::ThemeManager() : currentTheme_(nullptr) {
    initializeDefaultThemes();
}

ThemeManager& ThemeManager::getInstance() {
    static ThemeManager instance; // NOSONAR - Meyers singleton pattern
    return instance;
}

void ThemeManager::initializeDefaultThemes() {
    registerTheme(std::make_unique<LightTheme>());
    registerTheme(std::make_unique<DarkTheme>());
    registerTheme(std::make_unique<BlueTheme>());
    
    setCurrentTheme("Light");
}

void ThemeManager::registerTheme(std::unique_ptr<ITheme> theme) {
    if (!theme) {
        throw ThemeException("Cannot register null theme");
    }
    
    const QString name = theme->getName();
    themes_.try_emplace(name, std::move(theme));
}

MyVector<QString> ThemeManager::getAvailableThemes() const {
    MyVector<QString> themeNames;
    for (const auto& [name, themePtr] : themes_) {
        (void)themePtr;
        themeNames.push_back(name);
    }
    return themeNames;
}

ITheme* ThemeManager::getTheme(const QString& name) const {
    auto it = themes_.find(name);
    if (it == themes_.end()) {
        throw ThemeNotFoundException(name);
    }
    return it->second.get();
}

void ThemeManager::setCurrentTheme(const QString& name) {
    currentTheme_ = getTheme(name);
}

ITheme* ThemeManager::getCurrentTheme() const {
    if (!currentTheme_) {
        throw ThemeException("No current theme set");
    }
    return currentTheme_;
}