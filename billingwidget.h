#ifndef BILLINGWIDGET_H
#define BILLINGWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QTableWidget>
#include <QLabel>
#include <QGroupBox>
#include <QTextEdit>
#include <QDateTime>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QPrinter>
#include <QPrintDialog>
#include <QPainter>
#include <QTextDocument>
#include <QVariantMap>
#include <QList>
#include <QDialog>
#include <QDialogButtonBox>
#include "billingdatabase.h"

class AddItemDialog : public QDialog {
    Q_OBJECT

public:
    explicit AddItemDialog(QWidget *parent = nullptr);
    void setProductData(const QVariantMap &product);
    QVariantMap getItemData() const;
    void setProducts(const QList<QVariantMap>& products);

    void refreshAvailableProducts();
    QComboBox* getProductNameCombo() { return productNameEdit; }

private slots:
    void onQuantityChanged();
    void onProductSelected(int index);
    void validateAndAccept();
    void onProductNameEdited(const QString &text);

private:
    QComboBox *productNameEdit;
    QLineEdit *brandEdit;
    QLineEdit *specEdit;
    QLineEdit *unitEdit;
    QSpinBox *quantitySpinBox;
    QDoubleSpinBox *priceSpinBox;
    QTextEdit *remarkEdit;
    QLabel *productImageLabel;
    QLabel *amountLabel;
    QList<QVariantMap> availableProducts;

    QPushButton *refreshButton;

    void updateAmount();
};

class BillingWidget : public QWidget {
    Q_OBJECT

public:
    explicit BillingWidget(QWidget *parent = nullptr);

private slots:
    void onAddItemClicked();
    void onRemoveItemClicked();
    void onModifyItemClicked();
    void onExportClicked();
    void onPrintClicked();
    void onIssueClicked();
    void onClearClicked();
    void onCustomerSelected(const QString &name);
    void onCustomerManualChanged();
    void onRefreshClicked();

private:
    void setupUI();
    void initDefaultData();
    void loadCustomers();
    void loadProducts();
    void addOrderItem(const QVariantMap &itemData);
    void updateTotalAmount();
    void generateOrderNo();
    void saveOrderToDatabase();
    void printOrder();
    void clearOrder();
    void showItemDialog();
    void convertAmountToChinese(double amount, QString &chineseAmount);
    QString generateOrderText();
    void exportToPDF();

    QComboBox *customerCombo;
    QLineEdit *customerAddressEdit;
    QLineEdit *contactPersonEdit;
    QLineEdit *phoneEdit;
    QLineEdit *salesmanEdit;
    QComboBox *paymentMethodCombo;
    QLineEdit *storeEdit;

    QTableWidget *itemsTable;
    QLabel *totalAmountLabel;
    QLabel *totalAmountWordsLabel;

    QPushButton *addItemBtn;
    QPushButton *removeItemBtn;
    QPushButton *modifyItemBtn;
    QPushButton *refreshBtn;
    QPushButton *exportBtn;
    QPushButton *printBtn;
    QPushButton *issueBtn;
    QPushButton *clearBtn;

    QList<QVariantMap> orderItems;
    double totalAmount;
    QString orderNo;
    QString orderDate;
    QString status;
    QString createdBy;

    BillingDatabaseManager *dbManager;
    QList<QVariantMap> availableProducts;
};

#endif
