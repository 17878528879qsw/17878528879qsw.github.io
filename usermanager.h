#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <QString>

class UserManager
{
public:
    static UserManager& instance();

    // 用户信息获取
    bool getUserInfo(const QString& username, QString& nickname, QString& avatarPath);
    QString getCurrentUsername() const;
    QString getCurrentNickname() const;
    QString getCurrentAvatarPath() const;

    // 用户信息更新
    bool updateUserInfo(const QString& username, const QString& nickname, const QString& avatarPath);
    bool updateUsername(const QString& oldUsername, const QString& newUsername);
    bool updatePassword(const QString& username, const QString& newPassword);

    // 设置当前用户信息
    void setCurrentUser(const QString& username, const QString& nickname, const QString& avatarPath);

private:
    UserManager();
    ~UserManager();

    QString m_currentUsername;
    QString m_currentNickname;
    QString m_currentAvatarPath;
};

#endif
