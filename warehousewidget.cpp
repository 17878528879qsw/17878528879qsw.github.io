#include "warehousewidget.h"
#include "warehousedatabase.h"
#include "currentusermanager.h"
#include <QApplication>
#include <QFont>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QScrollBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QPixmap>
#include <QBuffer>
#include <QDesktopServices>
#include <QUrl>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

// 添加对话框
AddWarehouseDialog::AddWarehouseDialog(QWidget *parent) : QDialog(parent),
    nameEdit(new QLineEdit),
    brandEdit(new QLineEdit),
    specEdit(new QLineEdit),
    unitEdit(new QLineEdit),
    stockSpinBox(new QSpinBox),
    totalSpinBox(new QSpinBox),
    wholesaleSpinBox(new QDoubleSpinBox),
    retailSpinBox(new QDoubleSpinBox),
    remarkEdit(new QTextEdit),
    imagePathEdit(new QLineEdit),
    imageSelectBtn(new QPushButton("选择图片")) {

    setWindowTitle("商品信息");
    setFixedSize(800, 700);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QFormLayout *formLayout = new QFormLayout();
    formLayout->setSpacing(15);
    formLayout->setLabelAlignment(Qt::AlignRight);

    formLayout->addRow("商品名称:", nameEdit);
    formLayout->addRow("品牌:", brandEdit);
    formLayout->addRow("规格:", specEdit);
    formLayout->addRow("单位:", unitEdit);
    formLayout->addRow("库存数量:", stockSpinBox);
    formLayout->addRow("销售累计总量:", totalSpinBox);
    formLayout->addRow("批发价:", wholesaleSpinBox);
    formLayout->addRow("零售价:", retailSpinBox);
    formLayout->addRow("备注:", remarkEdit);
    formLayout->addRow("图片路径:", imagePathEdit);
    formLayout->addRow("", imageSelectBtn);

    mainLayout->addLayout(formLayout);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *okBtn = new QPushButton("确定");
    QPushButton *cancelBtn = new QPushButton("取消");

    connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(imageSelectBtn, &QPushButton::clicked, this, &AddWarehouseDialog::onImageSelectClicked);

    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);
    mainLayout->addLayout(btnLayout);

    imagePathEdit->setReadOnly(true);

    // 设置数值范围
    stockSpinBox->setMinimum(0);
    stockSpinBox->setMaximum(999999);
    totalSpinBox->setMinimum(0);
    totalSpinBox->setMaximum(999999);
    wholesaleSpinBox->setMinimum(0.0);
    wholesaleSpinBox->setMaximum(999999.99);
    retailSpinBox->setMinimum(0.0);
    retailSpinBox->setMaximum(999999.99);
}

void AddWarehouseDialog::onImageSelectClicked() {
    QString fileName = QFileDialog::getOpenFileName(this, "选择图片", "", "图片文件 (*.png *.jpg *.jpeg *.bmp)");
    if (!fileName.isEmpty()) {
        imagePathEdit->setText(fileName);
    }
}

// 界面展示
WarehouseWidget::WarehouseWidget(QWidget *parent)
    : QWidget(parent)
    , currentWarehouseId(-1)
{
    setupUI();
    loadWarehouseItems();
}

void WarehouseWidget::setupUI()
{
    setStyleSheet(R"(
        QWidget {
            font-family: "Microsoft YaHei";
            font-size: 24px;
        }
        QGroupBox {
            font-weight: bold;
            border: 1px solid #CCCCCC;
            border-radius: 8px;
            margin-top: 1ex;
            padding-top: 10px;
        }
        QGroupBox::title {
            subline-offset: -2px;
            padding: 0px 5px;
        }
        QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox, QTextEdit {
            padding: 8px;
            border: 1px solid #CCCCCC;
            border-radius: 4px;
            background-color: white;
        }
        QPushButton {
            padding: 8px 16px;
            border: 1px solid #3498DB;
            border-radius: 4px;
            background-color: #3498DB;
            color: white;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #2980B9;
        }
        QPushButton:pressed {
            background-color: #21618C;
        }
        QPushButton:disabled {
            background-color: #BDC3C7;
            border: 1px solid #95A5A6;
        }
        QTableWidget {
            border: 1px solid #CCCCCC;
            border-radius: 4px;
            gridline-color: #E0E0E0;
            selection-background-color: #D6EAF8;
        }
        QHeaderView::section {
            background-color: #3498DB;
            color: white;
            padding: 10px;
            font-weight: bold;
            border: none;
        }
        QTableWidget::item {
            padding: 15px;
            font-size: 20px;
        }
        QTableWidget::item:selected {
            background-color: #D6EAF8;
        }
    )");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    mainLayout->setSpacing(25);

    // 搜索栏 + 按钮栏
    setupSearchBar();
    mainLayout->addLayout(searchBarLayout);

    // 表格
    setupWarehouseTable();
    mainLayout->addWidget(warehouseTable);

    // 操作按钮栏（现在只保留"导出Excel"）
    setupButtons();
    mainLayout->addLayout(buttonLayout);
}

void WarehouseWidget::setupSearchBar()
{
    searchBarLayout = new QHBoxLayout();

    searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText("请输入搜索内容");
    searchEdit->setFixedWidth(200);

    searchFieldCombo = new QComboBox(this);
    searchFieldCombo->addItem("商品名称");
    searchFieldCombo->addItem("品牌");
    searchFieldCombo->addItem("规格");
    searchFieldCombo->addItem("批发价");
    searchFieldCombo->addItem("零售价");

    searchBtn = new QPushButton("搜索", this);
    allBtn = new QPushButton("全部", this);
    exportBtn = new QPushButton("导出Excel", this);

    addBtn = new QPushButton("添加", this);
    modifyBtn = new QPushButton("修改", this);
    deleteBtn = new QPushButton("删除", this);

    // 新增排序控件
    sortFieldCombo = new QComboBox(this);
    sortFieldCombo->addItem("商品名称");
    sortFieldCombo->addItem("品牌");
    sortFieldCombo->addItem("规格");
    sortFieldCombo->addItem("批发价");
    sortFieldCombo->addItem("零售价");
    sortFieldCombo->addItem("库存");
    sortFieldCombo->addItem("销售累计总量");
    sortFieldCombo->addItem("ID");

    sortOrderCombo = new QComboBox(this);
    sortOrderCombo->addItem("升序");
    sortOrderCombo->addItem("降序");

    sortBtn = new QPushButton("排序", this);

    connect(searchBtn, &QPushButton::clicked, this, &WarehouseWidget::onSearchTextChanged);
    connect(allBtn, &QPushButton::clicked, this, &WarehouseWidget::showAllProducts);
    connect(exportBtn, &QPushButton::clicked, this, &WarehouseWidget::exportToExcel);
    connect(addBtn, &QPushButton::clicked, this, &WarehouseWidget::onAddWarehouseItemClicked);
    connect(modifyBtn, &QPushButton::clicked, this, &WarehouseWidget::onModifyWarehouseItemClicked);
    connect(deleteBtn, &QPushButton::clicked, this, &WarehouseWidget::onDeleteWarehouseItemClicked);
    connect(sortBtn, &QPushButton::clicked, this, &WarehouseWidget::onSortChanged);  // 连接排序按钮

    searchBarLayout->addWidget(new QLabel("搜索:"));
    searchBarLayout->addWidget(searchEdit);
    searchBarLayout->addWidget(searchFieldCombo);
    searchBarLayout->addWidget(searchBtn);
    searchBarLayout->addWidget(allBtn);

    // 添加排序控件
    searchBarLayout->addWidget(new QLabel("排序字段:"));
    searchBarLayout->addWidget(sortFieldCombo);
    searchBarLayout->addWidget(new QLabel("排序方式:"));
    searchBarLayout->addWidget(sortOrderCombo);
    searchBarLayout->addWidget(sortBtn);

    searchBarLayout->addWidget(addBtn);
    searchBarLayout->addWidget(modifyBtn);
    searchBarLayout->addWidget(deleteBtn);
    searchBarLayout->addStretch();
    searchBarLayout->addWidget(exportBtn);
}

void WarehouseWidget::setupWarehouseTable()
{
    warehouseTable = new QTableWidget(this);
    warehouseTable->setColumnCount(11);
    QStringList headers = {"ID", "图片", "商品名称", "品牌", "规格", "单位", "库存", "销售累计总量", "批发价", "零售价", "备注"};
    warehouseTable->setHorizontalHeaderLabels(headers);
    warehouseTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    warehouseTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    warehouseTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    warehouseTable->verticalHeader()->setVisible(false);
    warehouseTable->setAlternatingRowColors(true);

    // 设置列宽
    warehouseTable->setColumnWidth(0, 80);   // ID
    warehouseTable->setColumnWidth(1, 160);  // 图片
    warehouseTable->setColumnWidth(2, 220);  // 商品名称
    warehouseTable->setColumnWidth(3, 140);  // 品牌
    warehouseTable->setColumnWidth(4, 140);  // 规格
    warehouseTable->setColumnWidth(5, 140);  // 单位
    warehouseTable->setColumnWidth(6, 140);  // 库存
    warehouseTable->setColumnWidth(7, 180);  // 总数
    warehouseTable->setColumnWidth(8, 140);  // 批发价
    warehouseTable->setColumnWidth(9, 140);  // 零售价
    warehouseTable->setColumnWidth(10, 280); // 备注

    // 设置行高
    warehouseTable->verticalHeader()->setDefaultSectionSize(100);

    connect(warehouseTable, &QTableWidget::itemSelectionChanged, this, &WarehouseWidget::onWarehouseItemSelected);
}

void WarehouseWidget::setupButtons()
{
    buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch(); // 空白占位
}

void WarehouseWidget::loadWarehouseItems()
{
    WarehouseDatabaseManager& db = WarehouseDatabaseManager::instance();
    if (!db.connect()) {
        QMessageBox::critical(this, "错误", "无法连接仓库数据库！");
        return;
    }

    QList<QVariantMap> products;
    QString sortField = sortFieldCombo->currentText();
    QString sortOrder = sortOrderCombo->currentText();

    if (db.getAllProductsWithSort(sortField, sortOrder, products)) {
        updateTableFromResults(products);
    }
}

void WarehouseWidget::updateTableFromResults(const QList<QVariantMap>& results)
{
    warehouseTable->setRowCount(results.size());
    for (int i = 0; i < results.size(); ++i) {
        auto& p = results[i];

        // ID
        QTableWidgetItem *idItem = new QTableWidgetItem(QString::number(p["id"].toInt()));
        idItem->setTextAlignment(Qt::AlignCenter);
        warehouseTable->setItem(i, 0, idItem);

        // 图片
        QString imagePath = p["image"].toString();
        if (!imagePath.isEmpty() && QFile::exists(imagePath)) {
            QPixmap pixmap = createThumbnail(imagePath);
            if (!pixmap.isNull()) {
                QLabel *imageLabel = new QLabel();
                imageLabel->setPixmap(pixmap);
                imageLabel->setAlignment(Qt::AlignCenter);
                warehouseTable->setCellWidget(i, 1, imageLabel);
            } else {
                warehouseTable->setItem(i, 1, new QTableWidgetItem("图片无效"));
            }
        } else {
            warehouseTable->setItem(i, 1, new QTableWidgetItem("无图片"));
        }

        // 商品名称
        QTableWidgetItem *nameItem = new QTableWidgetItem(p["name"].toString());
        warehouseTable->setItem(i, 2, nameItem);

        // 品牌
        QTableWidgetItem *brandItem = new QTableWidgetItem(p["brand"].toString());
        warehouseTable->setItem(i, 3, brandItem);

        // 规格
        QTableWidgetItem *specItem = new QTableWidgetItem(p["spec"].toString());
        warehouseTable->setItem(i, 4, specItem);

        // 单位
        QTableWidgetItem *unitItem = new QTableWidgetItem(p["unit"].toString());
        unitItem->setTextAlignment(Qt::AlignCenter);
        warehouseTable->setItem(i, 5, unitItem);

        // 库存
        QTableWidgetItem *stockItem = new QTableWidgetItem(QString::number(p["stock"].toInt()));
        stockItem->setTextAlignment(Qt::AlignCenter);
        warehouseTable->setItem(i, 6, stockItem);

        // 总数
        QTableWidgetItem *totalItem = new QTableWidgetItem(QString::number(p["total"].toInt()));
        totalItem->setTextAlignment(Qt::AlignCenter);
        warehouseTable->setItem(i, 7, totalItem);

        // 批发价
        QTableWidgetItem *wholesaleItem = new QTableWidgetItem(QString::number(p["wholesale"].toDouble(), 'f', 2));
        wholesaleItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        warehouseTable->setItem(i, 8, wholesaleItem);

        // 零售价
        QTableWidgetItem *retailItem = new QTableWidgetItem(QString::number(p["retail"].toDouble(), 'f', 2));
        retailItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        warehouseTable->setItem(i, 9, retailItem);

        // 备注
        QTableWidgetItem *remarkItem = new QTableWidgetItem(p["remark"].toString());
        warehouseTable->setItem(i, 10, remarkItem);
    }
}

QPixmap WarehouseWidget::createThumbnail(const QString& imagePath, const QSize& size)
{
    QPixmap originalPixmap(imagePath);
    if (originalPixmap.isNull()) {
        return QPixmap();
    }

    return originalPixmap.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

//添加
void WarehouseWidget::onAddWarehouseItemClicked()
{
    // 检查权限 - 只有超级管理员才能修改商品
    QString currentUserRole = CurrentUserManager::instance().getCurrentRole();
    if (currentUserRole != "super_admin") {
        QMessageBox::warning(this, "权限不足", "只有超级管理员才能修改商品！");
        return;
    }

    AddWarehouseDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        WarehouseDatabaseManager& db = WarehouseDatabaseManager::instance();
        if (!db.connect()) return;

        bool success = db.insertProduct(
            dialog.getName(),
            dialog.getBrand(),
            dialog.getSpec(),
            dialog.getUnit(),
            dialog.getStock(),
            dialog.getTotal(),
            dialog.getWholesalePrice(),
            dialog.getRetailPrice(),
            dialog.getRemark(),
            dialog.getImagePath()
        );

        if (success) {
            QMessageBox::information(this, "成功", "商品添加成功！");
            loadWarehouseItems();
        } else {
            QMessageBox::critical(this, "失败", "添加失败，请检查数据！");
        }
    }
}

//修改
void WarehouseWidget::onModifyWarehouseItemClicked()
{
    // 检查权限 - 只有超级管理员才能修改商品
    QString currentUserRole = CurrentUserManager::instance().getCurrentRole();
    if (currentUserRole != "super_admin") {
        QMessageBox::warning(this, "权限不足", "只有超级管理员才能修改商品！");
        return;
    }

    if (currentWarehouseId == -1) {
        QMessageBox::warning(this, "提示", "请先选择要修改的商品！");
        return;
    }

    // 获取当前商品信息
    WarehouseDatabaseManager& db = WarehouseDatabaseManager::instance();
    if (!db.connect()) return;

    QString name, brand, spec, unit, remark, imagePath;
    int stock, total;
    double wholesalePrice, retailPrice;

    if (!db.getProductById(currentWarehouseId, name, brand, spec, unit, stock, total,
                           wholesalePrice, retailPrice, remark, imagePath)) {
        QMessageBox::warning(this, "错误", "未找到该商品！");
        return;
    }

    AddWarehouseDialog dialog(this);
    dialog.setName(name);
    dialog.setBrand(brand);
    dialog.setSpec(spec);
    dialog.setUnit(unit);
    dialog.setStock(stock);
    dialog.setTotal(total);
    dialog.setWholesalePrice(wholesalePrice);
    dialog.setRetailPrice(retailPrice);
    dialog.setRemark(remark);
    dialog.setImagePath(imagePath);

    if (dialog.exec() == QDialog::Accepted) {
        bool success = db.updateProduct(
            currentWarehouseId,
            dialog.getName(),
            dialog.getBrand(),
            dialog.getSpec(),
            dialog.getUnit(),
            dialog.getStock(),
            dialog.getTotal(),
            dialog.getWholesalePrice(),
            dialog.getRetailPrice(),
            dialog.getRemark(),
            dialog.getImagePath()
        );

        if (success) {
            QMessageBox::information(this, "成功", "商品修改成功！");
            loadWarehouseItems();
        } else {
            QMessageBox::critical(this, "失败", "修改失败！");
        }
    }
}

//删除
void WarehouseWidget::onDeleteWarehouseItemClicked()
{
    // 检查权限 - 只有超级管理员才能修改商品
    QString currentUserRole = CurrentUserManager::instance().getCurrentRole();
    if (currentUserRole != "super_admin") {
        QMessageBox::warning(this, "权限不足", "只有超级管理员才能修改商品！");
        return;
    }

    if (currentWarehouseId == -1) {
        QMessageBox::warning(this, "提示", "请先选择要删除的商品！");
        return;
    }

    // 获取商品名称用于确认提示
    QString productName;
    for (int i = 0; i < warehouseTable->rowCount(); ++i) {
        if (warehouseTable->item(i, 0)->text().toInt() == currentWarehouseId) {
            productName = warehouseTable->item(i, 2)->text();
            break;
        }
    }

    QString message = QString("确定要删除商品 \"%1\" 吗？").arg(productName);
    if (QMessageBox::question(this, "确认删除", message, QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
        return;
    }

    WarehouseDatabaseManager& db = WarehouseDatabaseManager::instance();
    if (!db.connect()) return;

    bool success = db.deleteProduct(currentWarehouseId);
    if (success) {
        QMessageBox::information(this, "成功", "商品删除成功！");
        currentWarehouseId = -1;
        loadWarehouseItems();
    } else {
        QMessageBox::critical(this, "失败", "删除失败！");
    }
}

void WarehouseWidget::onSearchTextChanged()
{
    QString field = searchFieldCombo->currentText();
    QString value = searchEdit->text();

    if (value.isEmpty()) {
        loadWarehouseItems();
        return;
    }

    WarehouseDatabaseManager& db = WarehouseDatabaseManager::instance();
    if (!db.connect()) return;

    QList<QVariantMap> results;
    QString sortField = sortFieldCombo->currentText();
    QString sortOrder = sortOrderCombo->currentText();

    if (field == "商品名称") {
        db.searchProductsWithSort("product_name", value, sortField, sortOrder, results);
    } else if (field == "品牌") {
        db.searchProductsWithSort("brand", value, sortField, sortOrder, results);
    } else if (field == "规格") {
        db.searchProductsWithSort("specification", value, sortField, sortOrder, results);
    } else if (field == "批发价") {
        db.searchProductsWithSort("wholesale_price", value, sortField, sortOrder, results);
    } else if (field == "零售价") {
        db.searchProductsWithSort("retail_price", value, sortField, sortOrder, results);
    }

    updateTableFromResults(results);
}

void WarehouseWidget::onWarehouseItemSelected()
{
    QModelIndexList selected = warehouseTable->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        currentWarehouseId = -1;
        return;
    }

    int row = selected.first().row();
    currentWarehouseId = warehouseTable->item(row, 0)->text().toInt();
}

void WarehouseWidget::showAllProducts()
{
    searchEdit->clear();

    WarehouseDatabaseManager& db = WarehouseDatabaseManager::instance();
    if (!db.connect()) return;

    QList<QVariantMap> products;
    QString sortField = sortFieldCombo->currentText();
    QString sortOrder = sortOrderCombo->currentText();

    if (db.getAllProductsWithSort(sortField, sortOrder, products)) {
        updateTableFromResults(products);
    }
}

//导出表格
void WarehouseWidget::exportToExcel()
{
    QString fileName = QFileDialog::getSaveFileName(this, "导出Excel文件",
        QString("商品数据_%1.xlsx").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")),
        "Excel文件 (*.xlsx);;CSV文件 (*.csv)");

    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "导出失败", "无法创建文件！");
        return;
    }

    QTextStream out(&file);

    // 写入表头
    out << "ID,图片路径,商品名称,品牌,规格,单位,库存,总数,批发价,零售价,备注\n";

    // 写入数据
    for (int row = 0; row < warehouseTable->rowCount(); ++row) {
        for (int col = 0; col < warehouseTable->columnCount(); ++col) {
            if (col > 0) out << ",";

            QString text;
            if (col == 1) { // 图片列
                QWidget *widget = warehouseTable->cellWidget(row, col);
                if (widget) {
                    QLabel *label = qobject_cast<QLabel*>(widget);
                    if (label) {
                        text = label->text();
                    } else {
                        text = "无图片";
                    }
                } else {
                    text = warehouseTable->item(row, col)->text();
                }
            } else {
                QTableWidgetItem *item = warehouseTable->item(row, col);
                if (item) {
                    text = item->text();
                } else {
                    text = "";
                }
            }

            // 转义引号和逗号
            text.replace("\"", "\"\"");
            if (text.contains(",") || text.contains("\"") || text.contains("\n")) {
                out << "\"" << text << "\"";
            } else {
                out << text;
            }
        }
        out << "\n";
    }

    file.close();

    QMessageBox::information(this, "导出成功", QString("数据已导出到：%1").arg(fileName));

    if (QMessageBox::question(this, "打开文件", "是否立即打开导出的文件？", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
    }
}

// 排序槽函数
void WarehouseWidget::onSortChanged()
{
    QString field = searchFieldCombo->currentText();
    QString value = searchEdit->text();

    WarehouseDatabaseManager& db = WarehouseDatabaseManager::instance();
    if (!db.connect()) return;

    QList<QVariantMap> results;
    QString sortField = sortFieldCombo->currentText();
    QString sortOrder = sortOrderCombo->currentText();

    if (value.isEmpty()) {
        // 如果没有搜索条件，按排序条件显示所有商品
        if (db.getAllProductsWithSort(sortField, sortOrder, results)) {
            updateTableFromResults(results);
        }
    } else {
        // 如果有搜索条件，按搜索条件和排序条件显示商品
        if (field == "商品名称") {
            db.searchProductsWithSort("product_name", value, sortField, sortOrder, results);
        } else if (field == "品牌") {
            db.searchProductsWithSort("brand", value, sortField, sortOrder, results);
        } else if (field == "规格") {
            db.searchProductsWithSort("specification", value, sortField, sortOrder, results);
        } else if (field == "批发价") {
            db.searchProductsWithSort("wholesale_price", value, sortField, sortOrder, results);
        } else if (field == "零售价") {
            db.searchProductsWithSort("retail_price", value, sortField, sortOrder, results);
        }
        updateTableFromResults(results);
    }
}
