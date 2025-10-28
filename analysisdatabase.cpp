#include "analysisdatabase.h"
#include <QDir>
#include <QDebug>
#include <QSqlError>

AnalysisDatabaseManager::AnalysisDatabaseManager() {}

AnalysisDatabaseManager::~AnalysisDatabaseManager() {
    if (db.isOpen()) {
        db.close();
    }
}

AnalysisDatabaseManager& AnalysisDatabaseManager::instance() {
    static AnalysisDatabaseManager instance;
    return instance;
}

QString AnalysisDatabaseManager::getDatabasePath() {
    return "D:/QtProject/product_management.db";
}

bool AnalysisDatabaseManager::connect() {
    if (db.isOpen()) return true;

    db = QSqlDatabase::addDatabase("QSQLITE", "analysis_connection");
    QString path = getDatabasePath();
    QDir dir;
    if (!dir.exists(QDir(path).path())) {
        dir.mkpath(".");
    }
    db.setDatabaseName(path);

    if (!db.open()) {
        qWarning() << "链接失败：" << db.lastError().text();
        return false;
    }

    qDebug() << "链接成功：" << path;
    return true;
}

bool AnalysisDatabaseManager::getSalesDataByPeriod(const QString& startDate, const QString& endDate,
                                                 const QString& periodType, QList<QVariantMap>& salesData)
{
    if (!connect()) return false;

    QSqlQuery query(db);
    QString sql;

    if (periodType == "按日统计") {
        sql = "SELECT date(order_date) as period, SUM(total_amount) as amount "
              "FROM orders "
              "WHERE date(order_date) BETWEEN date(?) AND date(?) "
              "GROUP BY date(order_date) "
              "ORDER BY period";
    } else if (periodType == "按周统计") {
        sql = "SELECT strftime('%Y-%W', order_date) as period, SUM(total_amount) as amount "
              "FROM orders "
              "WHERE date(order_date) BETWEEN date(?) AND date(?) "
              "GROUP BY strftime('%Y-%W', order_date) "
              "ORDER BY period";
    } else { // 按月统计
        sql = "SELECT strftime('%Y-%m', order_date) as period, SUM(total_amount) as amount "
              "FROM orders "
              "WHERE date(order_date) BETWEEN date(?) AND date(?) "
              "GROUP BY strftime('%Y-%m', order_date) "
              "ORDER BY period";
    }

    query.prepare(sql);
    query.bindValue(0, startDate);
    query.bindValue(1, endDate);

    if (!query.exec()) {
        qWarning() << "Query failed:" << query.lastError().text();
        return false;
    }

    int rowCount = 0;
    while (query.next()) {
        rowCount++;
        QVariantMap row;
        row["period"] = query.value("period");
        row["amount"] = query.value("amount");
        salesData.append(row);
    }

    return true;
}

bool AnalysisDatabaseManager::getProductSalesData(const QString& startDate, const QString& endDate,
                                                QList<QVariantMap>& productData) {
    if (!connect()) return false;

    QSqlQuery query(db);
    QString sql = "SELECT oi.product_name, SUM(oi.quantity) as quantity, SUM(oi.amount) as amount "
                  "FROM order_items oi "
                  "JOIN orders o ON oi.order_id = o.id "
                  "WHERE o.order_date BETWEEN ? AND ? "
                  "GROUP BY oi.product_name "
                  "ORDER BY amount DESC";

    query.prepare(sql);
    query.addBindValue(startDate);
    query.addBindValue(endDate);

    if (!query.exec()) {
        qWarning() << "Query product sales data failed:" << query.lastError().text();
        return false;
    }

    while (query.next()) {
        QVariantMap data;
        data["product_name"] = query.value("product_name");
        data["quantity"] = query.value("quantity");
        data["amount"] = query.value("amount");
        productData.append(data);
    }

    return true;
}

bool AnalysisDatabaseManager::getCustomerSalesData(const QString& startDate, const QString& endDate,
                                                 QList<QVariantMap>& customerData) {
    if (!connect()) return false;

    QSqlQuery query(db);
    QString sql = "SELECT cr.customer_name, COUNT(o.id) as order_count, SUM(o.total_amount) as amount "
                  "FROM orders o "
                  "JOIN customer_records cr ON o.customer_name = cr.customer_name "
                  "WHERE o.order_date BETWEEN ? AND ? "
                  "GROUP BY cr.customer_name "
                  "ORDER BY amount DESC";

    query.prepare(sql);
    query.addBindValue(startDate);
    query.addBindValue(endDate);

    if (!query.exec()) {
        qWarning() << "Query customer sales data failed:" << query.lastError().text();
        return false;
    }

    while (query.next()) {
        QVariantMap data;
        data["customer_name"] = query.value("customer_name");
        data["order_count"] = query.value("order_count");
        data["amount"] = query.value("amount");
        customerData.append(data);
    }

    return true;
}

bool AnalysisDatabaseManager::getDetailedReportData(const QString& startDate, const QString& endDate,
                                                  QList<QVariantMap>& reportData) {
    if (!connect()) return false;

    QSqlQuery query(db);
    QString sql = "SELECT oi.product_name, SUM(oi.quantity) as quantity, SUM(oi.amount) as amount, "
                  "COUNT(DISTINCT oi.order_id) as order_count "
                  "FROM order_items oi "
                  "JOIN orders o ON oi.order_id = o.id "
                  "WHERE o.order_date BETWEEN ? AND ? "
                  "GROUP BY oi.product_name "
                  "ORDER BY amount DESC";

    query.prepare(sql);
    query.addBindValue(startDate);
    query.addBindValue(endDate);

    if (!query.exec()) {
        qWarning() << "Query detailed report data failed:" << query.lastError().text();
        return false;
    }

    double totalAmount = 0;
    QList<QVariantMap> tempData;

    while (query.next()) {
        QVariantMap data;
        data["product_name"] = query.value("product_name");
        data["quantity"] = query.value("quantity");
        data["amount"] = query.value("amount");
        data["order_count"] = query.value("order_count");
        tempData.append(data);
        totalAmount += query.value("amount").toDouble();
    }

    for (int i = 0; i < tempData.size(); ++i) {
        auto& data = tempData[i];
        double amount = data["amount"].toDouble();
        double percentage = (totalAmount > 0) ? (amount / totalAmount * 100) : 0;
        data["percentage"] = percentage;
        data["rank"] = i + 1;
        reportData.append(data);
    }

    return true;
}

bool AnalysisDatabaseManager::deleteOrderItem(const QString& productName)
{
    if (!connect()) return false;

    QSqlQuery query(db);
    QString sql = "DELETE FROM order_items WHERE product_name = ?";

    query.prepare(sql);
    query.addBindValue(productName);

    if (!query.exec()) {
        qWarning() << "Delete order item failed:" << query.lastError().text();
        return false;
    }

    return true;
}
