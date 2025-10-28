#ifndef PERMISSIONDATABASE_H
#define PERMISSIONDATABASE_H

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QString>
#include <QVariantMap>

class PermissionDatabaseManager {
public:
    static PermissionDatabaseManager& instance();

    bool connect();
    bool insertUser(const QString& username, const QString& nickname, const QString& role,
                   const QString& password);
    bool updateUser(int id, const QString& username, const QString& nickname, const QString& role,
                   const QString& password);
    bool deleteUser(int id);
    bool getUserById(int id, QString& username, QString& nickname, QString& role, QString& password);
    bool getAllUsers(QList<QVariantMap>& users);
    bool searchUsers(const QString& field, const QString& value, QList<QVariantMap>& results);
    bool isSuperAdmin(int userId);

private:
    PermissionDatabaseManager();
    ~PermissionDatabaseManager();
    QSqlDatabase db;
    QString getDatabasePath();
};

#endif
