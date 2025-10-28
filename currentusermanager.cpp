#include "currentusermanager.h"

CurrentUserManager::CurrentUserManager() {}

CurrentUserManager::~CurrentUserManager() {}

CurrentUserManager& CurrentUserManager::instance() {
    static CurrentUserManager instance;
    return instance;
}

void CurrentUserManager::setCurrentUser(const QString& username, const QString& role) {
    this->username = username;
    this->role = role;
}

QString CurrentUserManager::getCurrentUsername() const {
    return username;
}

QString CurrentUserManager::getCurrentRole() const {
    return role;
}
