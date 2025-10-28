#ifndef RECORDWIDGET_H
#define RECORDWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
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
#include <QDialog>
#include <QVariantMap>
#include <QList>
#include <QTextEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QDialogButtonBox>

class CustomerDialog : public QDialog
{
    Q_OBJECT

public:
    CustomerDialog(QWidget *parent = nullptr);
    void setCustomerData(const QVariantMap &data);
    QVariantMap getCustomerData() const;

private:
    QLineEdit *nameEdit;
    QLineEdit *addressEdit;
    QLineEdit *contactEdit;
    QLineEdit *phoneEdit;
    QSpinBox *totalOrdersSpinBox;
    QDoubleSpinBox *totalAmountSpinBox;
};

class RecordWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RecordWidget(QWidget *parent = nullptr);

private slots:
    void onSearchOrderClicked();
    void onResetOrderClicked();
    void onSearchCustomerClicked();
    void onResetCustomerClicked();
    void onOrderSelected();
    void onCustomerSelected();
    void onViewOrderDetails();
    void onDeleteOrder();
    void onAddCustomer();
    void onDeleteCustomer();
    void onEditCustomer();
    void onOrderTableHeaderClicked(int logicalIndex);
    void onCustomerTableHeaderClicked(int logicalIndex);
    void onCompleteOrder(); // 新增完成订单槽函数

private:
    void setupUI();
    void setupOrderTable();
    void setupCustomerTable();
    void loadOrders();
    void loadCustomers();
    void clearOrderTable();
    void clearCustomerTable();

    // UI 组件
    QLineEdit *orderSearchEdit;
    QLineEdit *customerSearchEdit;
    QComboBox *orderSearchFieldCombo;
    QComboBox *customerSearchFieldCombo;
    QDateEdit *startDateEdit;
    QDateEdit *endDateEdit;
    QPushButton *orderSearchBtn;
    QPushButton *orderResetBtn;
    QPushButton *customerSearchBtn;
    QPushButton *customerResetBtn;
    QPushButton *viewDetailsBtn;
    QPushButton *deleteOrderBtn;
    QPushButton *completeOrderBtn; // 新增完成订单按钮
    QPushButton *addCustomerBtn;
    QPushButton *deleteCustomerBtn;
    QPushButton *editCustomerBtn;

    QTableWidget *orderTable;
    QTableWidget *customerTable;
    QTabWidget *tabWidget;

    int currentOrderId;
    int currentCustomerId;

    QString currentOrderSortField;
    QString currentOrderSortOrder;
    QString currentCustomerSortField;
    QString currentCustomerSortOrder;
};

#endif // RECORDWIDGET_H
