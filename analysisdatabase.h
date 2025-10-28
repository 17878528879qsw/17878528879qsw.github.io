#ifndef ANALYSISDATABASE_H
#define ANALYSISDATABASE_H

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QString>
#include <QVariantMap>
#include <QList>

class AnalysisDatabaseManager {
public:
    static AnalysisDatabaseManager& instance();

    bool connect();
    bool getSalesDataByPeriod(const QString& startDate, const QString& endDate, const QString& periodType,
                             QList<QVariantMap>& salesData);
    bool getProductSalesData(const QString& startDate, const QString& endDate,
                           QList<QVariantMap>& productData);
    bool getCustomerSalesData(const QString& startDate, const QString& endDate,
                            QList<QVariantMap>& customerData);
    bool getDetailedReportData(const QString& startDate, const QString& endDate,
                              QList<QVariantMap>& reportData);

    bool deleteOrderItem(const QString& productName);

private:
    AnalysisDatabaseManager();
    ~AnalysisDatabaseManager();
    QSqlDatabase db;
    QString getDatabasePath();
};

#endif
