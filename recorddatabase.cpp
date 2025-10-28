#include "recorddatabase.h"
#include <QDir>
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>

RecordDatabaseManager::RecordDatabaseManager() {}

RecordDatabaseManager::~RecordDatabaseManager() {
    if (db.isOpen()) {
        db.close();
    }
}

RecordDatabaseManager& RecordDatabaseManager::instance() {
    static RecordDatabaseManager instance;
    return instance;
}

QString RecordDatabaseManager::getDatabasePath() {
    return "D:/QtProject/product_management.db";
}

bool RecordDatabaseManager::connect() {
    if (db.isOpen()) {
        return true;
    }

    db = QSqlDatabase::addDatabase("QSQLITE", "record_connection");
    QString path = getDatabasePath();
    db.setDatabaseName(path);

    if (!db.open()) {
        qWarning() << "Failed to open database:" << db.lastError().text();
        return false;
    }

    qDebug() << "Connected to product_management.db at:" << path;

    // 初始化表
    if (!initializeTables()) {
        qWarning() << "Failed to initialize tables:" << db.lastError().text();
        return false;
    }

    return true;
}

bool RecordDatabaseManager::initializeTables() {
    return createOrdersTable() && createCustomersTable();
}

bool RecordDatabaseManager::createOrdersTable() {
    QSqlQuery query(db);
    QString sql = R"(
        CREATE TABLE IF NOT EXISTS orders (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            order_no TEXT UNIQUE NOT NULL,
            customer_name TEXT NOT NULL,
            contact_person TEXT,
            phone TEXT,
            total_amount REAL DEFAULT 0,
            status TEXT DEFAULT '待处理',
            order_date TEXT,
            created_at TEXT DEFAULT CURRENT_TIMESTAMP,
            updated_at TEXT DEFAULT CURRENT_TIMESTAMP
        )
    )";

    if (!query.exec(sql)) {
        qWarning() << "Failed to create orders table:" << query.lastError().text();
        return false;
    }

    return true;
}

bool RecordDatabaseManager::createCustomersTable() {
    QSqlQuery query(db);
    QString sql = R"(
        CREATE TABLE IF NOT EXISTS customer_records (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            customer_name TEXT UNIQUE NOT NULL,
            customer_address TEXT,
            contact_person TEXT,
            phone TEXT,
            total_orders INTEGER DEFAULT 0,
            total_amount REAL DEFAULT 0,
            created_at TEXT DEFAULT CURRENT_TIMESTAMP,
            updated_at TEXT DEFAULT CURRENT_TIMESTAMP
        )
    )";

    if (!query.exec(sql)) {
        qWarning() << "Failed to create customers table:" << query.lastError().text();
        return false;
    }

    return true;
}

// 订单相关操作
bool RecordDatabaseManager::insertOrder(const QVariantMap& orderData) {
    QSqlQuery query(db);
    query.prepare("INSERT INTO orders (order_no, customer_name, contact_person, phone, "
                  "total_amount, status, order_date) VALUES (?, ?, ?, ?, ?, ?, ?)");

    query.addBindValue(orderData.value("order_no"));
    query.addBindValue(orderData.value("customer_name"));
    query.addBindValue(orderData.value("contact_person"));
    query.addBindValue(orderData.value("phone"));
    query.addBindValue(orderData.value("total_amount"));
    query.addBindValue(orderData.value("status"));
    query.addBindValue(orderData.value("order_date"));

    return query.exec();
}

bool RecordDatabaseManager::updateOrder(int id, const QVariantMap& orderData) {
    QSqlQuery query(db);
    query.prepare("UPDATE orders SET order_no = ?, customer_name = ?, contact_person = ?, "
                  "phone = ?, total_amount = ?, status = ?, order_date = ?, updated_at = CURRENT_TIMESTAMP "
                  "WHERE id = ?");

    query.addBindValue(orderData.value("order_no"));
    query.addBindValue(orderData.value("customer_name"));
    query.addBindValue(orderData.value("contact_person"));
    query.addBindValue(orderData.value("phone"));
    query.addBindValue(orderData.value("total_amount"));
    query.addBindValue(orderData.value("status"));
    query.addBindValue(orderData.value("order_date"));
    query.addBindValue(id);

    return query.exec();
}

bool RecordDatabaseManager::deleteOrder(int id) {
    QSqlQuery query(db);
    query.prepare("DELETE FROM orders WHERE id = ?");
    query.addBindValue(id);
    return query.exec();
}

bool RecordDatabaseManager::getOrderById(int id, QVariantMap& order) {
    QSqlQuery query(db);
    query.prepare("SELECT * FROM orders WHERE id = ?");
    query.addBindValue(id);

    if (query.exec() && query.next()) {
        order["id"] = query.value("id");
        order["order_no"] = query.value("order_no");
        order["customer_name"] = query.value("customer_name");
        order["contact_person"] = query.value("contact_person");
        order["phone"] = query.value("phone");
        order["total_amount"] = query.value("total_amount");
        order["status"] = query.value("status");
        order["order_date"] = query.value("order_date");
        order["created_at"] = query.value("created_at");
        order["updated_at"] = query.value("updated_at");
        return true;
    }
    return false;
}

bool RecordDatabaseManager::getAllOrders(QList<QVariantMap>& orders) {
    QSqlQuery query(db);
    query.prepare("SELECT * FROM orders ORDER BY id DESC");

    if (!query.exec()) {
        qWarning() << "Failed to get all orders:" << query.lastError().text();
        return false;
    }

    while (query.next()) {
        QVariantMap order;
        order["id"] = query.value("id");
        order["order_no"] = query.value("order_no");
        order["customer_name"] = query.value("customer_name");
        order["contact_person"] = query.value("contact_person");
        order["phone"] = query.value("phone");
        order["total_amount"] = query.value("total_amount");
        order["status"] = query.value("status");
        order["order_date"] = query.value("order_date");
        order["created_at"] = query.value("created_at");
        order["updated_at"] = query.value("updated_at");
        orders.append(order);
    }
    return true;
}

bool RecordDatabaseManager::searchOrders(const QString& field, const QString& value,
                                       const QString& startDate, const QString& endDate,
                                       QList<QVariantMap>& results) {
    QString sql = "SELECT * FROM orders WHERE ";
    QStringList conditions;

    if (!value.isEmpty()) {
        if (field == "订单号") {
            conditions << "order_no LIKE ?";
        } else if (field == "客户名称") {
            conditions << "customer_name LIKE ?";
        } else if (field == "联系人") {
            conditions << "contact_person LIKE ?";
        } else if (field == "联系电话") {
            conditions << "phone LIKE ?";
        } else if (field == "总金额") {
            conditions << "total_amount = ?";
        } else if (field == "订单状态") {
            conditions << "status LIKE ?";
        }
    }

    if (!startDate.isEmpty() && !endDate.isEmpty()) {
        conditions << "order_date BETWEEN ? AND ?";
    }

    if (conditions.isEmpty()) {
        sql = "SELECT * FROM orders";
    } else {
        sql += conditions.join(" AND ");
    }

    sql += " ORDER BY id DESC";

    QSqlQuery query(db);
    query.prepare(sql);

    int paramIndex = 0;
    if (!value.isEmpty()) {
        if (field == "订单号" || field == "客户名称" || field == "联系人" ||
            field == "联系电话" || field == "订单状态") {
            query.addBindValue("%" + value + "%");
            paramIndex++;
        } else if (field == "总金额") {
            bool ok;
            double amount = value.toDouble(&ok);
            if (ok) {
                query.addBindValue(amount);
                paramIndex++;
            }
        }
    }

    if (!startDate.isEmpty() && !endDate.isEmpty()) {
        query.addBindValue(startDate);
        query.addBindValue(endDate);
    }

    if (!query.exec()) {
        qWarning() << "Failed to search orders:" << query.lastError().text();
        return false;
    }

    while (query.next()) {
        QVariantMap order;
        order["id"] = query.value("id");
        order["order_no"] = query.value("order_no");
        order["customer_name"] = query.value("customer_name");
        order["contact_person"] = query.value("contact_person");
        order["phone"] = query.value("phone");
        order["total_amount"] = query.value("total_amount");
        order["status"] = query.value("status");
        order["order_date"] = query.value("order_date");
        order["created_at"] = query.value("created_at");
        order["updated_at"] = query.value("updated_at");
        results.append(order);
    }
    return true;
}

bool RecordDatabaseManager::getAllOrdersWithSort(const QString& sortField, const QString& sortOrder,
                                               QList<QVariantMap>& orders) {
    QString column;
    if (sortField == "ID") {
        column = "id";
    } else if (sortField == "订单号") {
        column = "order_no";
    } else if (sortField == "客户名称") {
        column = "customer_name";
    } else if (sortField == "联系人") {
        column = "contact_person";
    } else if (sortField == "联系电话") {
        column = "phone";
    } else if (sortField == "总金额") {
        column = "total_amount";
    } else if (sortField == "订单状态") {
        column = "status";
    } else if (sortField == "下单时间") {
        column = "order_date";
    } else {
        column = "id";
    }

    QString order = (sortOrder == "降序") ? "DESC" : "ASC";

    QString sql = QString("SELECT * FROM orders ORDER BY %1 %2").arg(column).arg(order);

    QSqlQuery query(db);
    if (!query.exec(sql)) {
        qWarning() << "Failed to get all orders with sort:" << query.lastError().text();
        return false;
    }

    while (query.next()) {
        QVariantMap order;
        order["id"] = query.value("id");
        order["order_no"] = query.value("order_no");
        order["customer_name"] = query.value("customer_name");
        order["contact_person"] = query.value("contact_person");
        order["phone"] = query.value("phone");
        order["total_amount"] = query.value("total_amount");
        order["status"] = query.value("status");
        order["order_date"] = query.value("order_date");
        order["created_at"] = query.value("created_at");
        order["updated_at"] = query.value("updated_at");
        orders.append(order);
    }
    return true;
}

// 新增完成订单方法
bool RecordDatabaseManager::completeOrder(int id) {
    QSqlQuery query(db);
    query.prepare("UPDATE orders SET status = '完成', updated_at = CURRENT_TIMESTAMP WHERE id = ?");
    query.addBindValue(id);
    return query.exec();
}

// 客户相关操作
bool RecordDatabaseManager::insertCustomer(const QVariantMap& customerData) {
    QSqlQuery query(db);
    query.prepare("INSERT INTO customer_records (customer_name, customer_address, contact_person, "
                  "phone, total_orders, total_amount) VALUES (?, ?, ?, ?, ?, ?)");

    query.addBindValue(customerData.value("customer_name"));
    query.addBindValue(customerData.value("customer_address"));
    query.addBindValue(customerData.value("contact_person"));
    query.addBindValue(customerData.value("phone"));
    query.addBindValue(customerData.value("total_orders", 0));
    query.addBindValue(customerData.value("total_amount", 0.0));

    return query.exec();
}

bool RecordDatabaseManager::updateCustomer(int id, const QVariantMap& customerData) {
    QSqlQuery query(db);
    query.prepare("UPDATE customer_records SET customer_name = ?, customer_address = ?, "
                  "contact_person = ?, phone = ?, total_orders = ?, total_amount = ?, "
                  "updated_at = CURRENT_TIMESTAMP WHERE id = ?");

    query.addBindValue(customerData.value("customer_name"));
    query.addBindValue(customerData.value("customer_address"));
    query.addBindValue(customerData.value("contact_person"));
    query.addBindValue(customerData.value("phone"));
    query.addBindValue(customerData.value("total_orders"));
    query.addBindValue(customerData.value("total_amount"));
    query.addBindValue(id);

    return query.exec();
}

bool RecordDatabaseManager::deleteCustomer(int id) {
    QSqlQuery query(db);
    query.prepare("DELETE FROM customer_records WHERE id = ?");
    query.addBindValue(id);
    return query.exec();
}

bool RecordDatabaseManager::getCustomerById(int id, QVariantMap& customer) {
    QSqlQuery query(db);
    query.prepare("SELECT * FROM customer_records WHERE id = ?");
    query.addBindValue(id);

    if (query.exec() && query.next()) {
        customer["id"] = query.value("id");
        customer["customer_name"] = query.value("customer_name");
        customer["customer_address"] = query.value("customer_address");
        customer["contact_person"] = query.value("contact_person");
        customer["phone"] = query.value("phone");
        customer["total_orders"] = query.value("total_orders");
        customer["total_amount"] = query.value("total_amount");
        customer["created_at"] = query.value("created_at");
        customer["updated_at"] = query.value("updated_at");
        return true;
    }
    return false;
}

bool RecordDatabaseManager::getAllCustomers(QList<QVariantMap>& customers) {
    QSqlQuery query(db);
    query.prepare("SELECT * FROM customer_records ORDER BY id DESC");

    if (!query.exec()) {
        qWarning() << "Failed to get all customers:" << query.lastError().text();
        return false;
    }

    while (query.next()) {
        QVariantMap customer;
        customer["id"] = query.value("id");
        customer["customer_name"] = query.value("customer_name");
        customer["customer_address"] = query.value("customer_address");
        customer["contact_person"] = query.value("contact_person");
        customer["phone"] = query.value("phone");
        customer["total_orders"] = query.value("total_orders");
        customer["total_amount"] = query.value("total_amount");
        customer["created_at"] = query.value("created_at");
        customer["updated_at"] = query.value("updated_at");
        customers.append(customer);
    }
    return true;
}

bool RecordDatabaseManager::searchCustomers(const QString& field, const QString& value,
                                          QList<QVariantMap>& results) {
    QString sql;
    if (field == "客户名称") {
        sql = "SELECT * FROM customer_records WHERE customer_name LIKE ? ORDER BY id DESC";
    } else if (field == "客户地址") {
        sql = "SELECT * FROM customer_records WHERE customer_address LIKE ? ORDER BY id DESC";
    } else if (field == "联系人") {
        sql = "SELECT * FROM customer_records WHERE contact_person LIKE ? ORDER BY id DESC";
    } else if (field == "联系电话") {
        sql = "SELECT * FROM customer_records WHERE phone LIKE ? ORDER BY id DESC";
    } else if (field == "总订单数") {
        sql = "SELECT * FROM customer_records WHERE total_orders = ? ORDER BY id DESC";
    } else if (field == "总金额") {
        sql = "SELECT * FROM customer_records WHERE total_amount = ? ORDER BY id DESC";
    } else {
        sql = "SELECT * FROM customer_records ORDER BY id DESC";
    }

    QSqlQuery query(db);
    query.prepare(sql);

    if (field == "客户名称" || field == "客户地址" || field == "联系人" || field == "联系电话") {
        query.addBindValue("%" + value + "%");
    } else if (field == "总订单数") {
        bool ok;
        int orders = value.toInt(&ok);
        if (ok) {
            query.addBindValue(orders);
        }
    } else if (field == "总金额") {
        bool ok;
        double amount = value.toDouble(&ok);
        if (ok) {
            query.addBindValue(amount);
        }
    }

    if (!query.exec()) {
        qWarning() << "Failed to search customers:" << query.lastError().text();
        return false;
    }

    while (query.next()) {
        QVariantMap customer;
        customer["id"] = query.value("id");
        customer["customer_name"] = query.value("customer_name");
        customer["customer_address"] = query.value("customer_address");
        customer["contact_person"] = query.value("contact_person");
        customer["phone"] = query.value("phone");
        customer["total_orders"] = query.value("total_orders");
        customer["total_amount"] = query.value("total_amount");
        customer["created_at"] = query.value("created_at");
        customer["updated_at"] = query.value("updated_at");
        results.append(customer);
    }
    return true;
}

bool RecordDatabaseManager::getAllCustomersWithSort(const QString& sortField, const QString& sortOrder,
                                                  QList<QVariantMap>& customers) {
    QString column;
    if (sortField == "ID") {
        column = "id";
    } else if (sortField == "客户名称") {
        column = "customer_name";
    } else if (sortField == "客户地址") {
        column = "customer_address";
    } else if (sortField == "联系人") {
        column = "contact_person";
    } else if (sortField == "联系电话") {
        column = "phone";
    } else if (sortField == "总订单数") {
        column = "total_orders";
    } else {
        column = "id";
    }

    QString order = (sortOrder == "降序") ? "DESC" : "ASC";

    QString sql = QString("SELECT * FROM customer_records ORDER BY %1 %2").arg(column).arg(order);

    QSqlQuery query(db);
    if (!query.exec(sql)) {
        qWarning() << "Failed to get all customers with sort:" << query.lastError().text();
        return false;
    }

    while (query.next()) {
        QVariantMap customer;
        customer["id"] = query.value("id");
        customer["customer_name"] = query.value("customer_name");
        customer["customer_address"] = query.value("customer_address");
        customer["contact_person"] = query.value("contact_person");
        customer["phone"] = query.value("phone");
        customer["total_orders"] = query.value("total_orders");
        customer["total_amount"] = query.value("total_amount");
        customer["created_at"] = query.value("created_at");
        customer["updated_at"] = query.value("updated_at");
        customers.append(customer);
    }
    return true;
}
