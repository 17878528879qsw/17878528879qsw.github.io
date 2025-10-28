#ifndef WAREHOUSEWIDGET_H
#define WAREHOUSEWIDGET_H

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
#include <QFileDialog>
#include <QMessageBox>
#include <QDialog>
#include <QTableWidgetItem>
#include <QPixmap>

class AddWarehouseDialog : public QDialog {
    Q_OBJECT

public:
    explicit AddWarehouseDialog(QWidget *parent = nullptr);

    // Getter methods
    QString getName() const { return nameEdit->text(); }
    QString getBrand() const { return brandEdit->text(); }
    QString getSpec() const { return specEdit->text(); }
    QString getUnit() const { return unitEdit->text(); }
    int getStock() const { return stockSpinBox->value(); }
    int getTotal() const { return totalSpinBox->value(); }
    double getWholesalePrice() const { return wholesaleSpinBox->value(); }
    double getRetailPrice() const { return retailSpinBox->value(); }
    QString getRemark() const { return remarkEdit->toPlainText(); }
    QString getImagePath() const { return imagePathEdit->text(); }

    // Setter methods
    void setName(const QString& name) { nameEdit->setText(name); }
    void setBrand(const QString& brand) { brandEdit->setText(brand); }
    void setSpec(const QString& spec) { specEdit->setText(spec); }
    void setUnit(const QString& unit) { unitEdit->setText(unit); }
    void setStock(int stock) { stockSpinBox->setValue(stock); }
    void setTotal(int total) { totalSpinBox->setValue(total); }
    void setWholesalePrice(double price) { wholesaleSpinBox->setValue(price); }
    void setRetailPrice(double price) { retailSpinBox->setValue(price); }
    void setRemark(const QString& remark) { remarkEdit->setPlainText(remark); }
    void setImagePath(const QString& path) { imagePathEdit->setText(path); }

private:
    QLineEdit *nameEdit;
    QLineEdit *brandEdit;
    QLineEdit *specEdit;
    QLineEdit *unitEdit;
    QSpinBox *stockSpinBox;
    QSpinBox *totalSpinBox;
    QDoubleSpinBox *wholesaleSpinBox;
    QDoubleSpinBox *retailSpinBox;
    QTextEdit *remarkEdit;
    QLineEdit *imagePathEdit;
    QPushButton *imageSelectBtn;

private slots:
    void onImageSelectClicked();
};

class WarehouseWidget : public QWidget {
    Q_OBJECT

public:
    explicit WarehouseWidget(QWidget *parent = nullptr);

private slots:
    void onAddWarehouseItemClicked();
    void onModifyWarehouseItemClicked();
    void onDeleteWarehouseItemClicked();
    void onSearchTextChanged();
    void onWarehouseItemSelected();
    void showAllProducts();
    void exportToExcel();
    void onSortChanged();

private:
    void setupUI();
    void setupSearchBar();
    void setupWarehouseTable();
    void setupButtons();
    void loadWarehouseItems();
    void updateTableFromResults(const QList<QVariantMap>& results);
    QPixmap createThumbnail(const QString& imagePath, const QSize& size = QSize(100, 100));

    // UI 组件
    QLineEdit *searchEdit;
    QComboBox *searchFieldCombo;
    QPushButton *searchBtn;
    QPushButton *allBtn;
    QPushButton *exportBtn;
    QPushButton *addBtn;
    QPushButton *modifyBtn;
    QPushButton *deleteBtn;

    // 新增排序控件
    QComboBox *sortFieldCombo;
    QComboBox *sortOrderCombo;
    QPushButton *sortBtn;

    QTableWidget *warehouseTable;

    int currentWarehouseId;

    // 布局组件
    QHBoxLayout *searchBarLayout;
    QHBoxLayout *buttonLayout;
};

#endif
