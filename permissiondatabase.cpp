#include "permissiondatabase.h"
#include <QDir>
#include <QDebug>
#include <QSqlError>

PermissionDatabaseManager::PermissionDatabaseManager() {}

PermissionDatabaseManager::~PermissionDatabaseManager() {
    if (db.isOpen()) {
        db.close();
    }
}

PermissionDatabaseManager& PermissionDatabaseManager::instance() {
    static PermissionDatabaseManager instance;
    return instance;
}

QString PermissionDatabaseManager::getDatabasePath() {
    return "D:/QtProject/product_management.db";
}

bool PermissionDatabaseManager::connect() {
    // 如果已经打开，则直接返回 true
    if (db.isOpen()) {
        return true;
    }

    db = QSqlDatabase::addDatabase("QSQLITE");
    QString path = getDatabasePath();
    QDir dir;
    if (!dir.exists(QDir(path).path())) {
        dir.mkpath(".");
    }
    db.setDatabaseName(path);

    if (!db.open()) {
        qWarning() << "Failed to open database:" << db.lastError().text();
        return false;
    }

    qDebug() << "Connected to product_management.db at:" << path;
    return true;
}

bool PermissionDatabaseManager::insertUser(const QString& username, const QString& nickname,
                                         const QString& role, const QString& password) {
    QSqlQuery query(db);
    query.prepare("INSERT INTO users (username, use_name, role, password, created_at) VALUES (?, ?, ?, ?, CURRENT_TIMESTAMP)");
    query.addBindValue(username);
    query.addBindValue(nickname);
    query.addBindValue(role);
    query.addBindValue(password);

    return query.exec();
}

bool PermissionDatabaseManager::updateUser(int id, const QString& username, const QString& nickname,
                                         const QString& role, const QString& password) {
    QSqlQuery query(db);
    query.prepare("UPDATE users SET username = ?, use_name = ?, role = ?, password = ? WHERE id = ?");
    query.addBindValue(username);
    query.addBindValue(nickname);
    query.addBindValue(role);
    query.addBindValue(password);
    query.addBindValue(id);

    return query.exec();
}

bool PermissionDatabaseManager::deleteUser(int id) {
    QSqlQuery query(db);
    query.prepare("DELETE FROM users WHERE id = ?");
    query.addBindValue(id);
    return query.exec();
}

bool PermissionDatabaseManager::getUserById(int id, QString& username, QString& nickname,
                                          QString& role, QString& password) {
    QSqlQuery query(db);
    query.prepare("SELECT username, use_name, role, password FROM users WHERE id = ?");
    query.addBindValue(id);

    if (query.exec() && query.next()) {
        username = query.value(0).toString();
        nickname = query.value(1).toString();
        role = query.value(2).toString();
        password = query.value(3).toString();
        return true;
    }
    return false;
}

bool PermissionDatabaseManager::getAllUsers(QList<QVariantMap>& users) {
    QSqlQuery query(db);
    query.exec("SELECT id, username, use_name, role, created_at FROM users ORDER BY id");

    while (query.next()) {
        QVariantMap user;
        user["id"] = query.value(0).toInt();
        user["username"] = query.value(1).toString();
        user["nickname"] = query.value(2).toString();
        user["role"] = query.value(3).toString();
        user["created_at"] = query.value(4).toString();
        users.append(user);
    }
    return true;
}

bool PermissionDatabaseManager::searchUsers(const QString& field, const QString& value, QList<QVariantMap>& results) {
    QSqlQuery query(db);
    QString sql;

    if (field == "username") {
        sql = "SELECT id, username, use_name, role, created_at FROM users WHERE username LIKE ? ORDER BY id";
    } else if (field == "use_name") {
        sql = "SELECT id, username, use_name, role, created_at FROM users WHERE use_name LIKE ? ORDER BY id";
    } else if (field == "role") {
        sql = "SELECT id, username, use_name, role, created_at FROM users WHERE role LIKE ? ORDER BY id";
    } else {
        return false;
    }

    query.prepare(sql);
    query.addBindValue("%" + value + "%");

    if (!query.exec()) {
        qWarning() << "Search failed:" << query.lastError().text();
        return false;
    }

    while (query.next()) {
        QVariantMap user;
        user["id"] = query.value(0).toInt();
        user["username"] = query.value(1).toString();
        user["nickname"] = query.value(2).toString();
        user["role"] = query.value(3).toString();
        user["created_at"] = query.value(4).toString();
        results.append(user);
    }

    return true;
}

bool PermissionDatabaseManager::isSuperAdmin(int userId) {
    QString username, nickname, role, password;
    if (getUserById(userId, username, nickname, role, password)) {
        return role == "super_admin";
    }
    return false;
}

