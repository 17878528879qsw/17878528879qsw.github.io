#ifndef WAREHOUSEDATABASE_H
#define WAREHOUSEDATABASE_H

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QString>
#include <QVariantMap>

class WarehouseDatabaseManager {
public:
    static WarehouseDatabaseManager& instance();

    bool connect();
    bool insertProduct(const QString& name, const QString& brand, const QString& spec,
                       const QString& unit, int stock, int total, double wholesalePrice,
                       double retailPrice, const QString& remark, const QString& imagePath);
    bool updateProduct(int id, const QString& name, const QString& brand, const QString& spec,
                       const QString& unit, int stock, int total, double wholesalePrice,
                       double retailPrice, const QString& remark, const QString& imagePath);
    bool deleteProduct(int id);
    bool getProductById(int id, QString& name, QString& brand, QString& spec, QString& unit,
                        int& stock, int& total, double& wholesalePrice, double& retailPrice,
                        QString& remark, QString& imagePath);
    bool getAllProducts(QList<QVariantMap>& products);
    bool searchProducts(const QString& field, const QString& value, QList<QVariantMap>& results);
    bool getAllProductsWithSort(const QString& sortField, const QString& sortOrder, QList<QVariantMap>& products);  // 新增排序方法
    bool searchProductsWithSort(const QString& field, const QString& value, const QString& sortField, const QString& sortOrder, QList<QVariantMap>& results);  // 新增带排序的搜索方法

private:
    WarehouseDatabaseManager();
    ~WarehouseDatabaseManager();
    QSqlDatabase db;
    QString getDatabasePath();
};

#endif
