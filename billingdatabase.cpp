#include "billingdatabase.h"
#include <QDir>
#include <QDebug>
#include <QSqlError>

BillingDatabaseManager::BillingDatabaseManager() {}

BillingDatabaseManager::~BillingDatabaseManager() {
    if (db.isOpen()) {
        db.close();
    }
}

BillingDatabaseManager& BillingDatabaseManager::instance() {
    static BillingDatabaseManager instance;
    return instance;
}

QString BillingDatabaseManager::getDatabasePath() const {
    return "D:/QtProject/product_management.db";
}

bool BillingDatabaseManager::connect() {
    if (db.isOpen()) return true;

    // 检查是否已经存在连接
    QStringList connections = QSqlDatabase::connectionNames();
    if (connections.contains("billing_connection")) {
        db = QSqlDatabase::database("billing_connection");
    } else {
        db = QSqlDatabase::addDatabase("QSQLITE", "billing_connection");
    }

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

    qDebug() << "Connected to billing database at:" << path;
    return true;
}

bool BillingDatabaseManager::insertOrder(const QVariantMap& orderData) {
    if (!connect()) return false;

    QSqlQuery query(db);
    query.prepare("INSERT INTO orders (order_no, customer_name, customer_address, contact_person, phone, salesman, payment_method, store, total_amount, total_amount_words, order_date, remarks, status, created_by, created_at, updated_at) "
                  "VALUES (:order_no, :customer_name, :customer_address, :contact_person, :phone, :salesman, :payment_method, :store, :total_amount, :total_amount_words, :order_date, :remarks, :status, :created_by, :created_at, :updated_at)");
    query.bindValue(":order_no", orderData["order_no"]);
    query.bindValue(":customer_name", orderData["customer_name"]);
    query.bindValue(":customer_address", orderData["customer_address"]);
    query.bindValue(":contact_person", orderData["contact_person"]);
    query.bindValue(":phone", orderData["phone"]);
    query.bindValue(":salesman", orderData["salesman"]);
    query.bindValue(":payment_method", orderData["payment_method"]);
    query.bindValue(":store", orderData["store"]);
    query.bindValue(":total_amount", orderData["total_amount"].toDouble());
    query.bindValue(":total_amount_words", orderData["total_amount_words"]);
    query.bindValue(":order_date", orderData["order_date"]);
    query.bindValue(":remarks", orderData["remarks"]);
    query.bindValue(":status", orderData["status"]);
    query.bindValue(":created_by", orderData["created_by"]);
    query.bindValue(":created_at", orderData["created_at"]);
    query.bindValue(":updated_at", orderData["updated_at"]);

    if (!query.exec()) {
        qWarning() << "Insert order failed:" << query.lastError().text();
        return false;
    }

    return true;
}

bool BillingDatabaseManager::insertOrderItem(int orderId, const QVariantMap& itemData) {
    if (!connect()) return false;

    QSqlQuery query(db);
    query.prepare("INSERT INTO order_items (order_id, product_id, product_name, product_specification, unit, quantity, unit_price, amount, remarks, created_at, updated_at) "
                  "VALUES (:order_id, :product_id, :product_name, :product_specification, :unit, :quantity, :unit_price, :amount, :remarks, :created_at, :updated_at)");
    query.bindValue(":order_id", orderId);
    query.bindValue(":product_id", itemData["product_id"].toInt());
    query.bindValue(":product_name", itemData["product_name"]);
    query.bindValue(":product_specification", itemData["specification"]);
    query.bindValue(":unit", itemData["unit"]);
    query.bindValue(":quantity", itemData["quantity"].toInt());
    query.bindValue(":unit_price", itemData["unit_price"].toDouble());
    query.bindValue(":amount", itemData["amount"].toDouble());
    query.bindValue(":remarks", itemData["remarks"]);
    query.bindValue(":created_at", itemData["created_at"]);
    query.bindValue(":updated_at", itemData["updated_at"]);

    return query.exec();
}

bool BillingDatabaseManager::getCustomerByName(const QString& name, QVariantMap& customer) {
    if (!connect()) return false;

    QSqlQuery query(db);
    query.prepare("SELECT * FROM customer_records WHERE customer_name = ?");
    query.addBindValue(name);

    if (query.exec() && query.next()) {
        customer["id"] = query.value("id");
        customer["customer_name"] = query.value("customer_name");
        customer["customer_address"] = query.value("customer_address");
        customer["contact_person"] = query.value("contact_person");
        customer["phone"] = query.value("phone");
        return true;
    }
    return false;
}

bool BillingDatabaseManager::getAllCustomers(QList<QVariantMap>& customers) {
    if (!connect()) return false;

    QSqlQuery query(db);
    query.prepare("SELECT * FROM customer_records");

    if (!query.exec()) {
        qWarning() << "Query customers failed:" << query.lastError().text();
        return false;
    }

    while (query.next()) {
        QVariantMap customer;
        customer["id"] = query.value("id");
        customer["customer_name"] = query.value("customer_name");
        customer["customer_address"] = query.value("customer_address");
        customer["contact_person"] = query.value("contact_person");
        customer["phone"] = query.value("phone");
        customers.append(customer);
    }

    return true;
}

bool BillingDatabaseManager::insertCustomer(const QVariantMap& customerData) {
    if (!connect()) return false;

    QSqlQuery query(db);
    query.prepare("INSERT INTO customer_records (customer_name, customer_address, contact_person, phone, total_orders, total_amount, last_order_date, created_at, updated_at) "
                  "VALUES (:name, :address, :contact, :phone, :orders, :amount, :last_date, :created_at, :updated_at)");
    query.bindValue(":name", customerData["customer_name"]);
    query.bindValue(":address", customerData["customer_address"]);
    query.bindValue(":contact", customerData["contact_person"]);
    query.bindValue(":phone", customerData["phone"]);
    query.bindValue(":orders", customerData["total_orders"].toInt());
    query.bindValue(":amount", customerData["total_amount"].toDouble());
    query.bindValue(":last_date", customerData["last_order_date"]);
    query.bindValue(":created_at", customerData["created_at"]);
    query.bindValue(":updated_at", customerData["updated_at"]);

    return query.exec();
}

bool BillingDatabaseManager::getProductById(int id, QVariantMap& product) {
    if (!connect()) return false;

    QSqlQuery query(db);
    query.prepare("SELECT id, product_name, brand, specification, unit, stock_quantity, wholesale_price, retail_price, product_image, is_deleted FROM warehouse WHERE id = ? AND is_deleted = 0 AND stock_quantity > 0");
    query.addBindValue(id);

    if (query.exec() && query.next()) {
        product["id"] = query.value("id");
        product["name"] = query.value("product_name");
        product["brand"] = query.value("brand");
        product["spec"] = query.value("specification");
        product["unit"] = query.value("unit");
        product["stock_quantity"] = query.value("stock_quantity").toInt();
        product["wholesale_price"] = query.value("wholesale_price").toDouble();
        product["retail_price"] = query.value("retail_price").toDouble();
        product["product_image"] = query.value("product_image").toString();
        product["is_deleted"] = query.value("is_deleted").toBool();
        return true;
    }
    return false;
}

bool BillingDatabaseManager::getAvailableProducts(QList<QVariantMap>& products) {
    if (!connect()) return false;

    QSqlQuery query(db);
    // 查询所有上架且有库存的商品 (is_deleted = 0 表示上架状态)
    query.prepare("SELECT id, product_name, brand, specification, unit, stock_quantity, wholesale_price, retail_price, product_image FROM warehouse WHERE is_deleted = 0 AND stock_quantity > 0 ORDER BY product_name");

    if (!query.exec()) {
        qWarning() << "Query available products failed:" << query.lastError().text();
        return false;
    }

    products.clear();
    while (query.next()) {
        QVariantMap product;
        product["id"] = query.value("id");
        product["name"] = query.value("product_name");
        product["brand"] = query.value("brand");
        product["spec"] = query.value("specification");
        product["unit"] = query.value("unit");
        product["stock_quantity"] = query.value("stock_quantity").toInt();
        product["wholesale_price"] = query.value("wholesale_price").toDouble();
        product["retail_price"] = query.value("retail_price").toDouble();
        product["product_image"] = query.value("product_image").toString();
        products.append(product);
    }

    qDebug() << "Loaded" << products.size() << "available products";
    return true;
}

bool BillingDatabaseManager::searchAvailableProducts(const QString& field, const QString& value, QList<QVariantMap>& results) {

    QSqlQuery query(db);
    QString sql;

    if (field == "product_name") {
        sql = "SELECT id, product_name, brand, specification, unit, stock_quantity, wholesale_price, retail_price, product_image FROM warehouse WHERE product_name LIKE ? AND is_deleted = 0 AND stock_quantity > 0 ORDER BY product_name";
    } else if (field == "brand") {
        sql = "SELECT id, product_name, brand, specification, unit, stock_quantity, wholesale_price, retail_price, product_image FROM warehouse WHERE brand LIKE ? AND is_deleted = 0 AND stock_quantity > 0 ORDER BY product_name";
    } else {
        return false;
    }

    query.prepare(sql);
    query.addBindValue("%" + value + "%");

    if (!query.exec()) {
        qWarning() << "Search available products failed:" << query.lastError().text();
        return false;
    }

    while (query.next()) {
        QVariantMap item;
        item["id"] = query.value("id");
        item["name"] = query.value("product_name");
        item["brand"] = query.value("brand");
        item["spec"] = query.value("specification");
        item["unit"] = query.value("unit");
        item["stock_quantity"] = query.value("stock_quantity").toInt();
        item["wholesale_price"] = query.value("wholesale_price").toDouble();
        item["retail_price"] = query.value("retail_price").toDouble();
        item["product_image"] = query.value("product_image").toString();
        results.append(item);
    }

    return true;
}

bool BillingDatabaseManager::checkStock(int productId, int quantity, int& availableCount) {
    if (!connect()) return false;

    QSqlQuery query(db);
    query.prepare("SELECT stock_quantity FROM warehouse WHERE id = ? AND is_deleted = 0");
    query.addBindValue(productId);

    if (query.exec() && query.next()) {
        int stock = query.value("stock_quantity").toInt();
        availableCount = stock;
        return availableCount >= quantity;
    }
    return false;
}

// 实现 getLastInsertId 方法
qint64 BillingDatabaseManager::getLastInsertId() const {
    // 通过执行查询获取最后插入的ID
    QSqlQuery query(db);
    if (query.exec("SELECT last_insert_rowid()") && query.next()) {
        return query.value(0).toLongLong();
    }
    return -1;  // 返回-1表示获取失败
}

// 新增更新库存方法
bool BillingDatabaseManager::updateProductStock(int productId, int quantity) {
    if (!connect()) return false;

    QSqlQuery query(db);
    query.prepare("UPDATE warehouse SET "
                  "stock_quantity = stock_quantity - ?, "
                  "total_count = total_count + ? "
                  "WHERE id = ?");
    query.addBindValue(quantity); // 减少可用库存
    query.addBindValue(quantity); // 增加已售总数
    query.addBindValue(productId);

    return query.exec();
}
