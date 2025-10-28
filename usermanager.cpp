// usermanager.cpp
#include "usermanager.h"
#include "database.h"

UserManager::UserManager()
    : m_currentUsername("")
    , m_currentNickname("")
    , m_currentAvatarPath("")
{
}

UserManager::~UserManager()
{
}

UserManager& UserManager::instance()
{
    static UserManager instance;
    return instance;
}

void UserManager::setCurrentUser(const QString& username, const QString& nickname, const QString& avatarPath)
{
    m_currentUsername = username;
    m_currentNickname = nickname;
    m_currentAvatarPath = avatarPath;
}

QString UserManager::getCurrentUsername() const
{
    return m_currentUsername;
}

QString UserManager::getCurrentNickname() const
{
    return m_currentNickname;
}

QString UserManager::getCurrentAvatarPath() const
{
    return m_currentAvatarPath;
}

bool UserManager::getUserInfo(const QString& username, QString& nickname, QString& avatarPath)
{
    DatabaseManager& dbManager = DatabaseManager::instance();
    if (dbManager.connect()) {
        return dbManager.getUserInfo(username, nickname, avatarPath);
    }
    return false;
}

bool UserManager::updateUserInfo(const QString& username, const QString& nickname, const QString& avatarPath)
{
    DatabaseManager& dbManager = DatabaseManager::instance();
    if (dbManager.connect()) {
        bool result = dbManager.updateUserInfo(username, nickname, avatarPath);
        if (result) {
            // 更新当前用户信息
            if (m_currentUsername == username) {
                m_currentNickname = nickname;
                m_currentAvatarPath = avatarPath;
            }
        }
        return result;
    }
    return false;
}

bool UserManager::updateUsername(const QString& oldUsername, const QString& newUsername)
{
    DatabaseManager& dbManager = DatabaseManager::instance();
    if (dbManager.connect()) {
        bool result = dbManager.updateUsername(oldUsername, newUsername);
        if (result) {
            // 更新当前用户信息
            if (m_currentUsername == oldUsername) {
                m_currentUsername = newUsername;
            }
        }
        return result;
    }
    return false;
}

bool UserManager::updatePassword(const QString& username, const QString& newPassword)
{
    DatabaseManager& dbManager = DatabaseManager::instance();
    if (dbManager.connect()) {
        return dbManager.updatePassword(username, newPassword);
    }
    return false;
}
