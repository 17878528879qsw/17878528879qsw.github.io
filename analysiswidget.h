#ifndef ANALYSISWIDGET_H
#define ANALYSISWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QLabel>
#include <QGroupBox>
#include <QTabWidget>
#include <QDateTime>
#include <QHeaderView>
#include <QMessageBox>
#include <QChartView>
#include <QBarSeries>
#include <QPieSeries>
#include <QLineSeries>
#include <QBarSet>
#include <QChart>
#include <QValueAxis>
#include <QBarCategoryAxis>
#include <QScatterSeries>
#include <QTextEdit>
#include <QDialog>
#include <QDialogButtonBox>

QT_CHARTS_USE_NAMESPACE

class AddReportDialog : public QDialog {
    Q_OBJECT

public:
    explicit AddReportDialog(QWidget *parent = nullptr);
    QVariantMap getReportData() const;

private:
    QLineEdit *productNameEdit;
    QLineEdit *quantityEdit;
    QLineEdit *amountEdit;
    QLineEdit *customerEdit;
    QDateEdit *dateEdit;
};

class EditReportDialog : public QDialog {
    Q_OBJECT

public:
    explicit EditReportDialog(const QVariantMap &data, QWidget *parent = nullptr);
    QVariantMap getReportData() const;

private:
    QLineEdit *productNameEdit;
    QLineEdit *quantityEdit;
    QLineEdit *amountEdit;
    QLineEdit *customerEdit;
    QDateEdit *dateEdit;
};

class AnalysisWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AnalysisWidget(QWidget *parent = nullptr);

private slots:
    void onGenerateReportClicked();
    void onDeleteReportClicked();
    void onExportReportClicked();
    void onTableSelectionChanged();

private:
    void setupUI();
    void setupFilterBar();
    void setupSalesChart();
    void setupProductChart();
    void setupCustomerChart();
    void setupReportTable();

    void updateSalesChart(const QString& startDate, const QString& endDate, const QString& analysisType);
    void updateProductChart(const QString& startDate, const QString& endDate);
    void updateCustomerChart(const QString& startDate, const QString& endDate);
    void updateReportTable(const QString& startDate, const QString& endDate);

    void addDataLabels(const QList<QVariantMap>& salesData, const QString& analysisType);

    void clearChart(QChart *chart);
    QString formatPeriod(const QString &period, const QString &type);

    QDateEdit *startDateEdit;
    QDateEdit *endDateEdit;
    QComboBox *analysisTypeCombo;
    QPushButton *generateReportBtn;
    QPushButton *exportReportBtn;

    QChartView *salesChartView;
    QChartView *productChartView;
    QChartView *customerChartView;

    QTableWidget *reportTable;
    QPushButton *deleteReportBtn;

    QTabWidget *tabWidget;

    QHBoxLayout *filterBarLayout;
};

#endif
