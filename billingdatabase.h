#ifndef BILLINGDATABASE_H
#define BILLINGDATABASE_H

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QString>
#include <QVariantMap>
#include <QList>

class BillingDatabaseManager {
public:
    static BillingDatabaseManager& instance();

    bool connect();
    bool insertOrder(const QVariantMap& orderData);
    bool insertOrderItem(int orderId, const QVariantMap& itemData);
    bool getCustomerByName(const QString& name, QVariantMap& customer);
    bool insertCustomer(const QVariantMap& customerData);
    bool getProductById(int id, QVariantMap& product);
    bool getAvailableProducts(QList<QVariantMap>& products);
    bool searchAvailableProducts(const QString& field, const QString& value, QList<QVariantMap>& results);
    bool checkStock(int productId, int quantity, int& availableCount);
    bool getAllCustomers(QList<QVariantMap>& customers);
    bool updateProductStock(int productId, int quantity);

    qint64 getLastInsertId() const;

private:
    QSqlDatabase db;
    BillingDatabaseManager();
    ~BillingDatabaseManager();

    QString getDatabasePath() const;
};

#endif
