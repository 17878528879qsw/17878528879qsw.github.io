#ifndef DATABASE_H
#define DATABASE_H

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QString>
#include <QVariant>
#include <QStandardPaths>
#include <QDir>

class DatabaseManager
{
public:
    static DatabaseManager& instance();
    bool connect();
    bool createUser(const QString& username, const QString& password, const QString& role);
    bool authenticateUser(const QString& username, const QString& password, QString& role);
    bool userExists(const QString& username);
    bool getUserInfo(const QString& username, QString& nickname, QString& avatarPath);
    bool updateUserInfo(const QString& username, const QString& nickname, const QString& avatarPath);
    bool updateUsername(const QString& oldUsername, const QString& newUsername);
    bool updatePassword(const QString& username, const QString& newPassword);

    void close();

private:
    DatabaseManager();
    ~DatabaseManager();
    QSqlDatabase db;
    void initializeDatabase();
    QString getDatabasePath();
};

#endif
