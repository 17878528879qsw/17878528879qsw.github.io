#ifndef RECORDERDATABASE_H
#define RECORDERDATABASE_H

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QString>
#include <QVariantMap>
#include <QList>

class RecordDatabaseManager {
public:
    static RecordDatabaseManager& instance();

    bool connect();
    bool initializeTables();

    // 订单相关操作
    bool insertOrder(const QVariantMap& orderData);
    bool updateOrder(int id, const QVariantMap& orderData);
    bool deleteOrder(int id);
    bool getOrderById(int id, QVariantMap& order);
    bool getAllOrders(QList<QVariantMap>& orders);
    bool searchOrders(const QString& field, const QString& value,
                      const QString& startDate, const QString& endDate,
                      QList<QVariantMap>& results);
    bool getAllOrdersWithSort(const QString& sortField, const QString& sortOrder,
                              QList<QVariantMap>& orders);
    bool completeOrder(int id); // 新增完成订单方法

    // 客户相关操作
    bool insertCustomer(const QVariantMap& customerData);
    bool updateCustomer(int id, const QVariantMap& customerData);
    bool deleteCustomer(int id);
    bool getCustomerById(int id, QVariantMap& customer);
    bool getAllCustomers(QList<QVariantMap>& customers);
    bool searchCustomers(const QString& field, const QString& value,
                         QList<QVariantMap>& results);
    bool getAllCustomersWithSort(const QString& sortField, const QString& sortOrder,
                                 QList<QVariantMap>& customers);

private:
    RecordDatabaseManager();
    ~RecordDatabaseManager();
    QSqlDatabase db;
    QString getDatabasePath();
    bool createOrdersTable();
    bool createCustomersTable();
};

#endif // RECORDERDATABASE_H
