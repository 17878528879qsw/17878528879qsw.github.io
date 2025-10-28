#ifndef CURRENTUSERMANAGER_H
#define CURRENTUSERMANAGER_H

#include <QString>

class CurrentUserManager {
public:
    static CurrentUserManager& instance();

    void setCurrentUser(const QString& username, const QString& role);
    QString getCurrentUsername() const;
    QString getCurrentRole() const;

private:
    CurrentUserManager();
    ~CurrentUserManager();

    QString username;
    QString role;
};

#endif
