#include "productdatabase.h"
#include <QDir>
#include <QDebug>
#include <QSqlError>

ProductDatabaseManager::ProductDatabaseManager() {}

ProductDatabaseManager::~ProductDatabaseManager() {
    if (db.isOpen()) {
        db.close();
    }
}

ProductDatabaseManager& ProductDatabaseManager::instance() {
    static ProductDatabaseManager instance;
    return instance;
}

QString ProductDatabaseManager::getDatabasePath() {
    return "D:/QtProject/product_management.db";
}

bool ProductDatabaseManager::connect() {
    if (db.isOpen()) return true;

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

//插入，现在不需要，已经移除添加功能
//bool ProductDatabaseManager::insertProduct(const QString& name, const QString& brand,
//                                         const QString& spec, const QString& unit,
//                                         double wholesalePrice, double retailPrice,
//                                         const QString& remark, const QString& imagePath) {
//    QSqlQuery query(db);
//    query.prepare("INSERT INTO warehouse (product_name, brand, specification, unit, "
//                  "stock_quantity, total_count, wholesale_price, retail_price, remark, "
//                  "created_at, updated_at, is_deleted) "
//                  "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP, ?)");

//    query.addBindValue(name);
//    query.addBindValue(brand);
//    query.addBindValue(spec);
//    query.addBindValue(unit);
//    query.addBindValue(0);
//    query.addBindValue(0);
//    query.addBindValue(wholesalePrice);
//    query.addBindValue(retailPrice);
//    query.addBindValue(remark);
//    query.addBindValue(0); // 默认上架

//    return query.exec();
//}

//更新，不需要了，已经移除了修改功能
//bool ProductDatabaseManager::updateProduct(int id, const QString& name, const QString& brand,
//                                         const QString& spec, const QString& unit,
//                                         double wholesalePrice, double retailPrice,
//                                         const QString& remark, const QString& imagePath,
//                                         bool isDeleted) {
//    QSqlQuery query(db);
//    query.prepare("UPDATE warehouse SET product_name = ?, brand = ?, specification = ?, "
//                  "unit = ?, wholesale_price = ?, retail_price = ?, remark = ?, is_deleted = ?, "
//                  "updated_at = CURRENT_TIMESTAMP WHERE id = ?");

//    query.addBindValue(name);
//    query.addBindValue(brand);
//    query.addBindValue(spec);
//    query.addBindValue(unit);
//    query.addBindValue(wholesalePrice);
//    query.addBindValue(retailPrice);
//    query.addBindValue(remark);
//    query.addBindValue(isDeleted);
//    query.addBindValue(id);

//    return query.exec();
//}

bool ProductDatabaseManager::deleteProduct(int id) {
    QSqlQuery query(db);
    query.prepare("UPDATE warehouse SET is_deleted = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?");
    query.addBindValue(true);
    query.addBindValue(id);
    return query.exec();
}

bool ProductDatabaseManager::getProductById(int id, QString& name, QString& brand, QString& spec,
                                          QString& unit, double& wholesalePrice, double& retailPrice,
                                          QString& remark, QString& imagePath, bool& isDeleted) {
    QSqlQuery query(db);
    query.prepare("SELECT product_name, brand, specification, unit, wholesale_price, retail_price, remark, is_deleted, product_image "
                  "FROM warehouse WHERE id = ?");
    query.addBindValue(id);

    if (query.exec() && query.next()) {
        name = query.value(0).toString();
        brand = query.value(1).toString();
        spec = query.value(2).toString();
        unit = query.value(3).toString();
        wholesalePrice = query.value(4).toDouble();
        retailPrice = query.value(5).toDouble();
        remark = query.value(6).toString();
        isDeleted = query.value(7).toBool();
        imagePath = query.value(8).toString();
        return true;
    }
    return false;
}

bool ProductDatabaseManager::getAllProducts(QList<QVariantMap>& products) {
    QSqlQuery query(db);
    query.exec("SELECT id, product_name, brand, specification, unit, wholesale_price, retail_price, remark, is_deleted, product_image "
               "FROM warehouse ORDER BY id");

    while (query.next()) {
        QVariantMap p;
        p["id"] = query.value(0).toInt();
        p["name"] = query.value(1).toString();
        p["brand"] = query.value(2).toString();
        p["spec"] = query.value(3).toString();
        p["unit"] = query.value(4).toString();
        p["wholesale_price"] = query.value(5).toDouble();
        p["retail_price"] = query.value(6).toDouble();
        p["description"] = query.value(7).toString();
        p["is_deleted"] = query.value(8).toBool();
        p["imagePath"] = query.value(9).toString();
        products.append(p);
    }
    return true;
}

bool ProductDatabaseManager::searchProducts(const QString& field, const QString& value, QList<QVariantMap>& results) {
    QSqlQuery query(db);
    QString sql;

    if (field == "product_name") {
        sql = "SELECT * FROM warehouse WHERE product_name LIKE ? ORDER BY id";
    } else if (field == "brand") {
        sql = "SELECT * FROM warehouse WHERE brand LIKE ? ORDER BY id";
    } else if (field == "id") {
        sql = "SELECT * FROM warehouse WHERE id = ? ORDER BY id";
    } else if (field == "is_deleted") {
        sql = "SELECT * FROM warehouse WHERE is_deleted = ? ORDER BY id";
    } else if (field == "wholesale_price") {
        sql = "SELECT * FROM warehouse WHERE wholesale_price = ? ORDER BY id";
    } else if (field == "retail_price") {
        sql = "SELECT * FROM warehouse WHERE retail_price = ? ORDER BY id";
    } else {
        return false;
    }

    query.prepare(sql);

    if (field == "id" || field == "is_deleted" || field == "wholesale_price" || field == "retail_price") {
        query.addBindValue(value);
    } else {
        query.addBindValue("%" + value + "%");
    }

    if (!query.exec()) {
        qWarning() << "Search failed:" << query.lastError().text();
        return false;
    }

    while (query.next()) {
        QVariantMap p;
        p["id"] = query.value("id");
        p["name"] = query.value("product_name");
        p["brand"] = query.value("brand");
        p["spec"] = query.value("specification");
        p["unit"] = query.value("unit");
        p["wholesale_price"] = query.value("wholesale_price");
        p["retail_price"] = query.value("retail_price");
        p["description"] = query.value("remark");
        p["is_deleted"] = query.value("is_deleted").toBool();
        p["imagePath"] = query.value("product_image").toString();
        results.append(p);
    }

    return true;
}

bool ProductDatabaseManager::getAllProductsWithSort(const QString& sortField, const QString& sortOrder, QList<QVariantMap>& products) {
    QSqlQuery query(db);

    // 将中文字段映射到数据库字段
    QString validSortField;
    if (sortField == "名称") validSortField = "product_name";
    else if (sortField == "品牌") validSortField = "brand";
    else if (sortField == "批发价") validSortField = "wholesale_price";
    else if (sortField == "零售价") validSortField = "retail_price";
    else if (sortField == "ID") validSortField = "id";
    else if (sortField == "状态") validSortField = "is_deleted";
    else validSortField = "id";

    QString validSortOrder = (sortOrder == "降序") ? "DESC" : "ASC";

    // 特殊处理状态字段排序，确保已上架(false)在前，已下架(true)在后
    if (validSortField == "is_deleted" && validSortOrder == "ASC") {
        validSortOrder = "DESC"; // 反转排序，使false(0)在前
    } else if (validSortField == "is_deleted" && validSortOrder == "DESC") {
        validSortOrder = "ASC";  // 反转排序，使true(1)在后
    }

    QString sql = QString("SELECT id, product_name, brand, specification, unit, wholesale_price, retail_price, remark, is_deleted, product_image "
                          "FROM warehouse ORDER BY %1 %2").arg(validSortField).arg(validSortOrder);

    if (!query.exec(sql)) {
        qWarning() << "Query failed:" << query.lastError().text();
        return false;
    }

    while (query.next()) {
        QVariantMap p;
        p["id"] = query.value(0).toInt();
        p["name"] = query.value(1).toString();
        p["brand"] = query.value(2).toString();
        p["spec"] = query.value(3).toString();
        p["unit"] = query.value(4).toString();
        p["wholesale_price"] = query.value(5).toDouble();
        p["retail_price"] = query.value(6).toDouble();
        p["description"] = query.value(7).toString();
        p["is_deleted"] = query.value(8).toBool();
        p["imagePath"] = query.value(9).toString();
        products.append(p);
    }
    return true;
}

bool ProductDatabaseManager::searchProductsWithSort(const QString& field, const QString& value,
                                                  const QString& sortField, const QString& sortOrder,
                                                  QList<QVariantMap>& results) {
    QSqlQuery query(db);
    QString whereClause;
    QVariant whereValue;

    if (field == "product_name") {
        whereClause = "product_name LIKE ?";
        whereValue = "%" + value + "%";
    } else if (field == "brand") {
        whereClause = "brand LIKE ?";
        whereValue = "%" + value + "%";
    } else if (field == "id") {
        whereClause = "id = ?";
        whereValue = value;
    } else if (field == "is_deleted") {
        whereClause = "is_deleted = ?";
        whereValue = value;
    } else if (field == "wholesale_price") {
        whereClause = "wholesale_price = ?";
        whereValue = value;
    } else if (field == "retail_price") {
        whereClause = "retail_price = ?";
        whereValue = value;
    } else {
        return false;
    }

    // 将中文字段映射到数据库字段
    QString validSortField;
    if (sortField == "名称") validSortField = "product_name";
    else if (sortField == "品牌") validSortField = "brand";
    else if (sortField == "批发价") validSortField = "wholesale_price";
    else if (sortField == "零售价") validSortField = "retail_price";
    else if (sortField == "ID") validSortField = "id";
    else if (sortField == "状态") validSortField = "is_deleted";
    else validSortField = "id";

    QString validSortOrder = (sortOrder == "降序") ? "DESC" : "ASC";

    // 特殊处理状态字段排序
    if (validSortField == "is_deleted" && validSortOrder == "ASC") {
        validSortOrder = "DESC"; // 反转排序
    } else if (validSortField == "is_deleted" && validSortOrder == "DESC") {
        validSortOrder = "ASC";  // 反转排序
    }

    QString sql = QString("SELECT * FROM warehouse WHERE %1 ORDER BY %2 %3")
                  .arg(whereClause)
                  .arg(validSortField)
                  .arg(validSortOrder);

    query.prepare(sql);
    query.addBindValue(whereValue);

    if (!query.exec()) {
        qWarning() << "Search failed:" << query.lastError().text();
        return false;
    }

    while (query.next()) {
        QVariantMap p;
        p["id"] = query.value("id");
        p["name"] = query.value("product_name");
        p["brand"] = query.value("brand");
        p["spec"] = query.value("specification");
        p["unit"] = query.value("unit");
        p["wholesale_price"] = query.value("wholesale_price");
        p["retail_price"] = query.value("retail_price");
        p["description"] = query.value("remark");
        p["is_deleted"] = query.value("is_deleted").toBool();
        p["imagePath"] = query.value("product_image").toString();
        results.append(p);
    }

    return true;
}
