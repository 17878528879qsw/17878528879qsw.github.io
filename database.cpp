#include "database.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QApplication>
#include <QStandardPaths>
#include <QDir>

DatabaseManager::DatabaseManager()
{
}

DatabaseManager::~DatabaseManager()
{
    if (db.isOpen()) {
        db.close();
    }
}

DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager instance;
    return instance;
}

QString DatabaseManager::getDatabasePath()
{
    // 指定数据库文件路径为 D:\QtProject\product_management.db
    QString dbPath = "D:/QtProject/product_management.db";

    // 确保目录存在
    QDir dir("D:/QtProject");
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    return dbPath;
}

bool DatabaseManager::connect()
{
    // 使用SQLite数据库
    db = QSqlDatabase::addDatabase("QSQLITE");
    QString dbPath = getDatabasePath();
    db.setDatabaseName(dbPath);

    if (!db.open()) {
        qDebug() << "Database connection failed:" << db.lastError().text();
        return false;
    }

    qDebug() << "Connected to SQLite database at:" << dbPath;
    initializeDatabase();
    return true;
}

void DatabaseManager::initializeDatabase()
{
    QSqlQuery query(db);

    // 创建用户表（更新表结构，添加昵称和头像字段）
    query.exec("CREATE TABLE IF NOT EXISTS users ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "username TEXT UNIQUE NOT NULL,"
               "password TEXT NOT NULL,"
               "role TEXT NOT NULL CHECK(role IN ('user', 'admin', 'super_admin')),"
               "use_name TEXT,"           // 用户昵称
               "use_img TEXT,"            // 用户头像路径
               "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
               ")");

    // 插入默认超级管理员账户
    query.prepare("INSERT OR IGNORE INTO users (username, password, role, use_name, use_img) VALUES (?, ?, ?, ?, ?)");
    query.addBindValue("superadmin");
    query.addBindValue("superadmin123");
    query.addBindValue("super_admin");
    query.addBindValue("超级管理员");  // 默认昵称
    query.addBindValue("");            // 默认无头像
    if (!query.exec()) {
        qDebug() << "Failed to insert superadmin:" << query.lastError().text();
    }
}

bool DatabaseManager::createUser(const QString& username, const QString& password, const QString& role)
{
    QSqlQuery query(db);
    query.prepare("INSERT INTO users (username, password, role, use_name, use_img) VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(username);
    query.addBindValue(password);
    query.addBindValue(role);
    query.addBindValue(username);      // 默认昵称等于用户名
    query.addBindValue("");            // 默认无头像

    bool result = query.exec();
    if (!result) {
        qDebug() << "Failed to create user:" << query.lastError().text();
    }
    return result;
}

bool DatabaseManager::authenticateUser(const QString& username, const QString& password, QString& role)
{
    QSqlQuery query(db);
    query.prepare("SELECT role FROM users WHERE username = ? AND password = ?");
    query.addBindValue(username);
    query.addBindValue(password);

    if (query.exec() && query.next()) {
        role = query.value(0).toString();
        return true;
    }
    return false;
}

bool DatabaseManager::getUserInfo(const QString& username, QString& nickname, QString& avatarPath)
{
    QSqlQuery query(db);
    query.prepare("SELECT use_name, use_img FROM users WHERE username = ?");
    query.addBindValue(username);

    if (query.exec() && query.next()) {
        nickname = query.value(0).toString();
        avatarPath = query.value(1).toString();

        // 如果昵称为空，使用用户名
        if (nickname.isEmpty()) {
            nickname = username;
        }
        return true;
    }
    return false;
}

bool DatabaseManager::updateUserInfo(const QString& username, const QString& nickname, const QString& avatarPath)
{
    QSqlQuery query(db);
    query.prepare("UPDATE users SET use_name = ?, use_img = ? WHERE username = ?");
    query.addBindValue(nickname);
    query.addBindValue(avatarPath);
    query.addBindValue(username);

    return query.exec();
}

bool DatabaseManager::updateUsername(const QString& oldUsername, const QString& newUsername)
{
    QSqlQuery query(db);
    query.prepare("UPDATE users SET username = ? WHERE username = ?");
    query.addBindValue(newUsername);
    query.addBindValue(oldUsername);

    bool result = query.exec();
    if (!result) {
        qDebug() << "Failed to update username:" << query.lastError().text();
    }
    return result;
}

bool DatabaseManager::updatePassword(const QString& username, const QString& newPassword)
{
    QSqlQuery query(db);
    query.prepare("UPDATE users SET password = ? WHERE username = ?");
    query.addBindValue(newPassword);
    query.addBindValue(username);

    bool result = query.exec();
    if (!result) {
        qDebug() << "Failed to update password:" << query.lastError().text();
    }
    return result;
}

bool DatabaseManager::userExists(const QString& username)
{
    QSqlQuery query(db);
    query.prepare("SELECT COUNT(*) FROM users WHERE username = ?");
    query.addBindValue(username);

    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }
    return false;
}

void DatabaseManager::close()
{
    if (db.isOpen()) {
        db.close();
    }
}
