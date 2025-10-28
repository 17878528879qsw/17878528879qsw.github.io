#include "warehousedatabase.h"
#include <QDir>
#include <QDebug>
#include <QSqlError>

WarehouseDatabaseManager::WarehouseDatabaseManager() {}

WarehouseDatabaseManager::~WarehouseDatabaseManager() {
    if (db.isOpen()) {
        db.close();
    }
}

WarehouseDatabaseManager& WarehouseDatabaseManager::instance() {
    static WarehouseDatabaseManager instance;
    return instance;
}

QString WarehouseDatabaseManager::getDatabasePath() {
    return "D:/QtProject/product_management.db";
}

bool WarehouseDatabaseManager::connect() {
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

// 插入商品
bool WarehouseDatabaseManager::insertProduct(const QString& name, const QString& brand, const QString& spec,
                                           const QString& unit, int stock, int total, double wholesalePrice,
                                           double retailPrice, const QString& remark, const QString& imagePath) {
    QSqlQuery query(db);
    query.prepare("INSERT INTO warehouse (product_name, brand, specification, unit, "
                  "stock_quantity, total_count, wholesale_price, retail_price, remark, "
                  "created_at, updated_at, is_deleted, product_image) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP, ?, ?)");

    query.addBindValue(name);
    query.addBindValue(brand);
    query.addBindValue(spec);
    query.addBindValue(unit);
    query.addBindValue(stock);
    query.addBindValue(total);
    query.addBindValue(wholesalePrice);
    query.addBindValue(retailPrice);
    query.addBindValue(remark);
    query.addBindValue(0);
    query.addBindValue(imagePath);

    return query.exec();
}

// 更新商品
bool WarehouseDatabaseManager::updateProduct(int id, const QString& name, const QString& brand, const QString& spec,
                                           const QString& unit, int stock, int total, double wholesalePrice,
                                           double retailPrice, const QString& remark, const QString& imagePath) {
    QSqlQuery query(db);
    query.prepare("UPDATE warehouse SET product_name = ?, brand = ?, specification = ?, "
                  "unit = ?, stock_quantity = ?, total_count = ?, wholesale_price = ?, retail_price = ?, "
                  "remark = ?, product_image = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?");

    query.addBindValue(name);
    query.addBindValue(brand);
    query.addBindValue(spec);
    query.addBindValue(unit);
    query.addBindValue(stock);
    query.addBindValue(total);
    query.addBindValue(wholesalePrice);
    query.addBindValue(retailPrice);
    query.addBindValue(remark);
    query.addBindValue(imagePath);
    query.addBindValue(id);

    return query.exec();
}

// 删除商品
bool WarehouseDatabaseManager::deleteProduct(int id) {
    QSqlQuery query(db);
    query.prepare("DELETE FROM warehouse WHERE id = ?");
    query.addBindValue(id);
    return query.exec();
}

// 根据ID获取商品
bool WarehouseDatabaseManager::getProductById(int id, QString& name, QString& brand,
                                            QString& spec, QString& unit, int& stock,
                                            int& total, double& wholesalePrice,
                                            double& retailPrice, QString& remark,
                                            QString& imagePath) {
    QSqlQuery query(db);
    query.prepare("SELECT product_name, brand, specification, unit, stock_quantity, "
                  "total_count, wholesale_price, retail_price, remark, product_image "
                  "FROM warehouse WHERE id = ?");
    query.addBindValue(id);

    if (query.exec() && query.next()) {
        name = query.value(0).toString();
        brand = query.value(1).toString();
        spec = query.value(2).toString();
        unit = query.value(3).toString();
        stock = query.value(4).toInt();
        total = query.value(5).toInt();
        wholesalePrice = query.value(6).toDouble();
        retailPrice = query.value(7).toDouble();
        remark = query.value(8).toString();
        imagePath = query.value(9).toString();
        return true;
    }
    return false;
}

// 获取所有商品
bool WarehouseDatabaseManager::getAllProducts(QList<QVariantMap>& products) {
    QSqlQuery query(db);
    query.exec("SELECT id, product_name, brand, specification, unit, stock_quantity, "
               "total_count, wholesale_price, retail_price, remark, product_image "
               "FROM warehouse ORDER BY id");

    while (query.next()) {
        QVariantMap product;
        product["id"] = query.value(0).toInt();
        product["name"] = query.value(1).toString();
        product["brand"] = query.value(2).toString();
        product["spec"] = query.value(3).toString();
        product["unit"] = query.value(4).toString();
        product["stock"] = query.value(5).toInt();
        product["total"] = query.value(6).toInt();
        product["wholesale"] = query.value(7).toDouble();
        product["retail"] = query.value(8).toDouble();
        product["remark"] = query.value(9).toString();
        product["image"] = query.value(10).toString();
        products.append(product);
    }
    return true;
}

// 搜索商品（修复：表名 + 数值字段处理）
bool WarehouseDatabaseManager::searchProducts(const QString& field, const QString& value, QList<QVariantMap>& results)
{
    QSqlQuery query(db);
    QString sql;

    if (field == "product_name") {
        sql = "SELECT * FROM warehouse WHERE product_name LIKE ? ORDER BY id";
    } else if (field == "brand") {
        sql = "SELECT * FROM warehouse WHERE brand LIKE ? ORDER BY id";
    } else if (field == "specification") {
        sql = "SELECT * FROM warehouse WHERE specification LIKE ? ORDER BY id";
    } else if (field == "wholesale_price") {
        bool ok;
        double price = value.toDouble(&ok);
        if (!ok) return false;
        sql = "SELECT * FROM warehouse WHERE wholesale_price = ? ORDER BY id";
    } else if (field == "retail_price") {
        bool ok;
        double price = value.toDouble(&ok);
        if (!ok) return false;
        sql = "SELECT * FROM warehouse WHERE retail_price = ? ORDER BY id";
    } else {
        return false;
    }

    query.prepare(sql);
    if (field == "wholesale_price" || field == "retail_price") {
        query.addBindValue(value.toDouble());
    } else {
        query.addBindValue("%" + value + "%");
    }

    if (!query.exec()) {
        qWarning() << "Search failed:" << query.lastError().text();
        return false;
    }

    while (query.next()) {
        QVariantMap item;
        item["id"] = query.value("id");
        item["name"] = query.value("product_name");
        item["brand"] = query.value("brand");
        item["spec"] = query.value("specification");
        item["unit"] = query.value("unit");
        item["stock"] = query.value("stock_quantity");
        item["total"] = query.value("total_count");
        item["wholesale"] = query.value("wholesale_price");
        item["retail"] = query.value("retail_price");
        item["remark"] = query.value("remark");
        item["image"] = query.value("product_image");
        results.append(item);
    }

    return true;
}

// 获取所有商品（带排序）
bool WarehouseDatabaseManager::getAllProductsWithSort(const QString& sortField, const QString& sortOrder, QList<QVariantMap>& products) {
    QSqlQuery query(db);

    // 验证排序字段和顺序
    QString validSortField;
    if (sortField == "商品名称") {
        validSortField = "product_name";
    } else if (sortField == "品牌") {
        validSortField = "brand";
    } else if (sortField == "规格") {
        validSortField = "specification";
    } else if (sortField == "批发价") {
        validSortField = "wholesale_price";
    } else if (sortField == "零售价") {
        validSortField = "retail_price";
    } else if (sortField == "库存") {
        validSortField = "stock_quantity";
    } else if (sortField == "总数") {
        validSortField = "total_count";
    } else {
        validSortField = "id";  // 默认按ID排序
    }

    QString validSortOrder = (sortOrder == "降序") ? "DESC" : "ASC";

    QString sql = QString("SELECT id, product_name, brand, specification, unit, stock_quantity, "
                          "total_count, wholesale_price, retail_price, remark, product_image "
                          "FROM warehouse ORDER BY %1 %2").arg(validSortField).arg(validSortOrder);

    if (!query.exec(sql)) {
        qWarning() << "Query failed:" << query.lastError().text();
        return false;
    }

    while (query.next()) {
        QVariantMap product;
        product["id"] = query.value(0).toInt();
        product["name"] = query.value(1).toString();
        product["brand"] = query.value(2).toString();
        product["spec"] = query.value(3).toString();
        product["unit"] = query.value(4).toString();
        product["stock"] = query.value(5).toInt();
        product["total"] = query.value(6).toInt();
        product["wholesale"] = query.value(7).toDouble();
        product["retail"] = query.value(8).toDouble();
        product["remark"] = query.value(9).toString();
        product["image"] = query.value(10).toString();
        products.append(product);
    }
    return true;
}

// 搜索商品（带排序）
bool WarehouseDatabaseManager::searchProductsWithSort(const QString& field, const QString& value,
                                                     const QString& sortField, const QString& sortOrder,
                                                     QList<QVariantMap>& results)
{
    QSqlQuery query(db);
    QString sql;
    QString whereClause;

    if (field == "product_name") {
        whereClause = "product_name LIKE ?";
    } else if (field == "brand") {
        whereClause = "brand LIKE ?";
    } else if (field == "specification") {
        whereClause = "specification LIKE ?";
    } else if (field == "wholesale_price") {
        bool ok;
        double price = value.toDouble(&ok);
        if (!ok) return false;
        whereClause = "wholesale_price = ?";
    } else if (field == "retail_price") {
        bool ok;
        double price = value.toDouble(&ok);
        if (!ok) return false;
        whereClause = "retail_price = ?";
    } else {
        return false;
    }

    // 验证排序字段和顺序
    QString validSortField;
    if (sortField == "商品名称") {
        validSortField = "product_name";
    } else if (sortField == "品牌") {
        validSortField = "brand";
    } else if (sortField == "规格") {
        validSortField = "specification";
    } else if (sortField == "批发价") {
        validSortField = "wholesale_price";
    } else if (sortField == "零售价") {
        validSortField = "retail_price";
    } else if (sortField == "库存") {
        validSortField = "stock_quantity";
    } else if (sortField == "总数") {
        validSortField = "total_count";
    } else {
        validSortField = "id";  // 默认按ID排序
    }

    QString validSortOrder = (sortOrder == "降序") ? "DESC" : "ASC";

    sql = QString("SELECT * FROM warehouse WHERE %1 ORDER BY %2 %3")
              .arg(whereClause)
              .arg(validSortField)
              .arg(validSortOrder);

    query.prepare(sql);
    if (field == "wholesale_price" || field == "retail_price") {
        query.addBindValue(value.toDouble());
    } else {
        query.addBindValue("%" + value + "%");
    }

    if (!query.exec()) {
        qWarning() << "Search failed:" << query.lastError().text();
        return false;
    }

    while (query.next()) {
        QVariantMap item;
        item["id"] = query.value("id");
        item["name"] = query.value("product_name");
        item["brand"] = query.value("brand");
        item["spec"] = query.value("specification");
        item["unit"] = query.value("unit");
        item["stock"] = query.value("stock_quantity");
        item["total"] = query.value("total_count");
        item["wholesale"] = query.value("wholesale_price");
        item["retail"] = query.value("retail_price");
        item["remark"] = query.value("remark");
        item["image"] = query.value("product_image");
        results.append(item);
    }

    return true;
}
