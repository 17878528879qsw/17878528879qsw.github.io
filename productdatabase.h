#ifndef PRODUCTDATABASE_H
#define PRODUCTDATABASE_H

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QString>
#include <QVariantMap>

class ProductDatabaseManager {
public:
    static ProductDatabaseManager& instance();

    bool connect();
    bool deleteProduct(int id);
    bool getProductById(int id, QString& name, QString& brand, QString& spec, QString& unit,
                        double& wholesalePrice, double& retailPrice, QString& remark,
                        QString& imagePath, bool& isDeleted);
    bool getAllProducts(QList<QVariantMap>& products);
    bool searchProducts(const QString& field, const QString& value, QList<QVariantMap>& results);
    bool getAllProductsWithSort(const QString& sortField, const QString& sortOrder, QList<QVariantMap>& products);
    bool searchProductsWithSort(const QString& field, const QString& value, const QString& sortField,
                                const QString& sortOrder, QList<QVariantMap>& results);

private:
    ProductDatabaseManager();
    ~ProductDatabaseManager();
    QSqlDatabase db;
    QString getDatabasePath();
};

#endif
