#include "productwidget.h"
#include "productdatabase.h"
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

ProductWidget::ProductWidget(QWidget *parent)
    : QWidget(parent)
    , currentProductId(-1)
{
    setupUI();
    loadProducts();
}

void ProductWidget::setupUI()
{
    setStyleSheet(R"(
        QWidget {
            font-family: "Microsoft YaHei";
            font-size: 26px;
        }
        QGroupBox {
            font-weight: bold;
            border: 2px solid #CCCCCC;
            border-radius: 10px;
            margin-top: 1ex;
            padding-top: 15px;
        }
        QGroupBox::title {
            subline-offset: -2px;
            padding: 0px 8px;
            font-size: 28px;
        }
        QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox, QTextEdit {
            padding: 12px;
            border: 2px solid #CCCCCC;
            border-radius: 6px;
            background-color: white;
            font-size: 24px;
        }
        QPushButton {
            padding: 12px 24px;
            border: 2px solid #3498DB;
            border-radius: 6px;
            background-color: #3498DB;
            color: white;
            font-weight: bold;
            font-size: 24px;
        }
        QPushButton:hover {
            background-color: #2980B9;
        }
        QPushButton:pressed {
            background-color: #21618C;
        }
        QPushButton:disabled {
            background-color: #BDC3C7;
            border: 2px solid #95A5A6;
        }
        QTableWidget {
            border: 2px solid #CCCCCC;
            border-radius: 6px;
            gridline-color: #E0E0E0;
            font-size: 24px;
            selection-background-color: #D6EAF8;
        }
        QHeaderView::section {
            background-color: #3498DB;
            color: white;
            padding: 16px;
            font-weight: bold;
            border: none;
            font-size: 26px;
        }
        QTableWidget::item {
            padding: 20px;
            font-size: 24px;
        }
        QTableWidget::item:selected {
            background-color: #D6EAF8;
        }
        QLabel {
            font-size: 24px;
        }
    )");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(60, 60, 60, 60);
    mainLayout->setSpacing(30);

    setupSearchBar();
    mainLayout->addLayout(searchBarLayout);

    setupProductTable();
    mainLayout->addWidget(productTable);

    setupButtons();
    mainLayout->addLayout(buttonLayout);
}

void ProductWidget::setupSearchBar()
{
    searchBarLayout = new QHBoxLayout();
    searchBarLayout->setSpacing(15);

    // 搜索部分
    QHBoxLayout *searchLayout = new QHBoxLayout();
    searchLayout->setSpacing(10);
    searchLayout->addWidget(new QLabel("搜索:"));
    searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText("请输入搜索内容");
    searchBarLayout->setSpacing(15);
    searchEdit->setFixedWidth(250);
    searchLayout->addWidget(searchEdit);

    searchFieldCombo = new QComboBox(this);
    searchFieldCombo->addItem("名称");
    searchFieldCombo->addItem("品牌");
    searchFieldCombo->addItem("ID");
    searchFieldCombo->addItem("状态");
    searchFieldCombo->addItem("批发价");
    searchFieldCombo->addItem("零售价");
    searchLayout->addWidget(searchFieldCombo);

    searchBtn = new QPushButton("搜索", this);
    searchLayout->addWidget(searchBtn);

    resetBtn = new QPushButton("重置", this);
    searchLayout->addWidget(resetBtn);

    // 排序部分
    QHBoxLayout *sortLayout = new QHBoxLayout();
    sortLayout->setSpacing(10);
    sortLayout->addWidget(new QLabel("排序:"));

    sortFieldCombo = new QComboBox(this);
    sortFieldCombo->addItem("名称");
    sortFieldCombo->addItem("品牌");
    sortFieldCombo->addItem("批发价");
    sortFieldCombo->addItem("零售价");
    sortFieldCombo->addItem("ID");
    sortFieldCombo->addItem("状态");
    sortLayout->addWidget(sortFieldCombo);

    sortOrderCombo = new QComboBox(this);
    sortOrderCombo->addItem("升序");
    sortOrderCombo->addItem("降序");
    sortLayout->addWidget(sortOrderCombo);

    sortBtn = new QPushButton("排序", this);
    sortLayout->addWidget(sortBtn);

    // 添加上架/下架按钮
    shelveBtn = new QPushButton("上架", this);
    unshelveBtn = new QPushButton("下架", this);

    connect(searchBtn, &QPushButton::clicked, this, &ProductWidget::onSearchTextChanged);
    connect(resetBtn, &QPushButton::clicked, this, &ProductWidget::onResetClicked);
    connect(sortBtn, &QPushButton::clicked, this, &ProductWidget::onSortChanged);
    connect(shelveBtn, &QPushButton::clicked, this, &ProductWidget::onShelveProductClicked);
    connect(unshelveBtn, &QPushButton::clicked, this, &ProductWidget::onUnshelveProductClicked);

    searchBarLayout->addLayout(searchLayout);
    searchBarLayout->addLayout(sortLayout);
    searchBarLayout->addWidget(shelveBtn);
    searchBarLayout->addWidget(unshelveBtn);
    searchBarLayout->addStretch();
}

void ProductWidget::setupProductTable()
{
    productTable = new QTableWidget(this);
    productTable->setColumnCount(7);
    QStringList headers = {"ID", "图片", "名称", "品牌", "批发价", "零售价", "状态"};
    productTable->setHorizontalHeaderLabels(headers);
    productTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    productTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    productTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    productTable->verticalHeader()->setVisible(false);
    productTable->setAlternatingRowColors(true);

    // 设置列宽
    productTable->setColumnWidth(0, 80);
    productTable->setColumnWidth(1, 220);
    productTable->setColumnWidth(2, 220);
    productTable->setColumnWidth(3, 120);
    productTable->setColumnWidth(4, 80);
    productTable->setColumnWidth(5, 80);
    productTable->setColumnWidth(6, 120);

    // 设置行高
    productTable->verticalHeader()->setDefaultSectionSize(100);

    connect(productTable, &QTableWidget::itemSelectionChanged, this, [this]() {
        QList<QTableWidgetItem*> items = productTable->selectedItems();
        if (items.isEmpty()) {
            currentProductId = -1;
            shelveBtn->setEnabled(false);
            unshelveBtn->setEnabled(false);
        } else {
            currentProductId = items.first()->text().toInt();
            shelveBtn->setEnabled(true);
            unshelveBtn->setEnabled(true);
        }
    });
}

void ProductWidget::setupButtons()
{
    buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
}

void ProductWidget::loadProducts()
{
    ProductDatabaseManager& db = ProductDatabaseManager::instance();
    if (!db.connect()) {
        QMessageBox::critical(this, "错误", "无法连接数据库！");
        return;
    }

    QList<QVariantMap> products;
    QString sortField = sortFieldCombo->currentText();
    QString sortOrder = sortOrderCombo->currentText();

    if (db.getAllProductsWithSort(sortField, sortOrder, products)) {
        updateTableFromResults(products);
    }
}

void ProductWidget::updateTableFromResults(const QList<QVariantMap>& results)
{
    productTable->setRowCount(results.size());
    for (int i = 0; i < results.size(); ++i) {
        auto& p = results[i];

        // ID
        QTableWidgetItem *idItem = new QTableWidgetItem(QString::number(p["id"].toInt()));
        idItem->setTextAlignment(Qt::AlignCenter);
        productTable->setItem(i, 0, idItem);

        // 图片
        QString imagePath = p["imagePath"].toString();
        if (!imagePath.isEmpty() && QFile::exists(imagePath)) {
            QPixmap pixmap = createThumbnail(imagePath, QSize(180, 180));
            if (!pixmap.isNull()) {
                QLabel *imageLabel = new QLabel();
                imageLabel->setPixmap(pixmap);
                imageLabel->setAlignment(Qt::AlignCenter);
                productTable->setCellWidget(i, 1, imageLabel);
            } else {
                QTableWidgetItem *imageItem = new QTableWidgetItem("图片无效");
                imageItem->setTextAlignment(Qt::AlignCenter);
                productTable->setItem(i, 1, imageItem);
            }
        } else {
            QTableWidgetItem *imageItem = new QTableWidgetItem("无图片");
            imageItem->setTextAlignment(Qt::AlignCenter);
            productTable->setItem(i, 1, imageItem);
        }

        // 名称
        QTableWidgetItem *nameItem = new QTableWidgetItem(p["name"].toString());
        nameItem->setTextAlignment(Qt::AlignCenter);
        productTable->setItem(i, 2, nameItem);

        // 品牌
        QTableWidgetItem *brandItem = new QTableWidgetItem(p["brand"].toString());
        brandItem->setTextAlignment(Qt::AlignCenter);
        productTable->setItem(i, 3, brandItem);

        // 批发价
        QTableWidgetItem *wholesaleItem = new QTableWidgetItem(QString::number(p["wholesale_price"].toDouble(), 'f', 2));
        wholesaleItem->setTextAlignment(Qt::AlignCenter);
        productTable->setItem(i, 4, wholesaleItem);

        // 零售价
        QTableWidgetItem *retailItem = new QTableWidgetItem(QString::number(p["retail_price"].toDouble(), 'f', 2));
        retailItem->setTextAlignment(Qt::AlignCenter);
        productTable->setItem(i, 5, retailItem);

        // 状态
        QString status = p["is_deleted"].toBool() ? "已下架" : "已上架";
        QTableWidgetItem *statusItem = new QTableWidgetItem(status);
        statusItem->setTextAlignment(Qt::AlignCenter);
        statusItem->setForeground(p["is_deleted"].toBool() ? QColor(128, 128, 128) : Qt::black);
        productTable->setItem(i, 6, statusItem);

        // 设置行颜色
        if (p["is_deleted"].toBool()) {
            for (int col = 0; col < productTable->columnCount(); ++col) {
                QTableWidgetItem* item = productTable->item(i, col);
                if (item) {
                    item->setBackgroundColor(QColor(240, 240, 240));
                }
            }
        }
    }

    // 调整表格大小以适应内容
    productTable->resizeColumnsToContents();
    productTable->horizontalHeader()->setStretchLastSection(true);
}

QPixmap ProductWidget::createThumbnail(const QString& imagePath, const QSize& size)
{
    QPixmap originalPixmap(imagePath);
    if (originalPixmap.isNull()) {
        return QPixmap();
    }
    return originalPixmap.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

void ProductWidget::onShelveProductClicked()
{
    if (currentProductId <= 0) return;

    QString currentUserRole = CurrentUserManager::instance().getCurrentRole();
    if (currentUserRole != "super_admin" && currentUserRole != "admin") {
        QMessageBox::warning(this, "权限不足", "只有超级管理员或管理员才能修改商品状态！");
        return;
    }

    ProductDatabaseManager& db = ProductDatabaseManager::instance();
    if (!db.connect()) return;

    QSqlQuery query;
    query.prepare("UPDATE warehouse SET is_deleted = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?");
    query.addBindValue(false);
    query.addBindValue(currentProductId);

    bool success = query.exec();
    if (success) {
        QMessageBox::information(this, "成功", "商品已上架！");
        loadProducts();
    } else {
        QMessageBox::critical(this, "失败", "上架失败！");
    }
}

void ProductWidget::onUnshelveProductClicked()
{
    if (currentProductId <= 0) return;

    QString currentUserRole = CurrentUserManager::instance().getCurrentRole();
    if (currentUserRole != "super_admin" && currentUserRole != "admin") {
        QMessageBox::warning(this, "权限不足", "只有超级管理员或管理员才能修改商品状态！");
        return;
    }

    ProductDatabaseManager& db = ProductDatabaseManager::instance();
    if (!db.connect()) return;

    QSqlQuery query;
    query.prepare("UPDATE warehouse SET is_deleted = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?");
    query.addBindValue(true);
    query.addBindValue(currentProductId);

    bool success = query.exec();
    if (success) {
        QMessageBox::information(this, "成功", "商品已下架！");
        loadProducts();
    } else {
        QMessageBox::critical(this, "失败", "下架失败！");
    }
}

void ProductWidget::onSearchTextChanged()
{
    QString field = searchFieldCombo->currentText();
    QString value = searchEdit->text();

    if (value.isEmpty()) {
        loadProducts();
        return;
    }

    ProductDatabaseManager& db = ProductDatabaseManager::instance();
    if (!db.connect()) return;

    QList<QVariantMap> results;
    QString sortField = sortFieldCombo->currentText();
    QString sortOrder = sortOrderCombo->currentText();

    // 将中文字段映射到数据库字段
    QString dbField;
    if (field == "名称") dbField = "product_name";
    else if (field == "品牌") dbField = "brand";
    else if (field == "ID") {
        dbField = "id";
        // 验证ID是否为数字
        bool isNumber;
        value.toInt(&isNumber);
        if (!isNumber) {
            QMessageBox::warning(this, "错误", "ID必须是数字！");
            return;
        }
    }
    else if (field == "状态") {
        dbField = "is_deleted";
        // 将中文状态转换为布尔值
        if (value == "已上架") value = "0";
        else if (value == "已下架") value = "1";
        else {
            QMessageBox::warning(this, "提示", "状态只能是'已上架'或'已下架'");
            return;
        }
    }
    else if (field == "批发价") {
        dbField = "wholesale_price";
        // 验证价格是否为数字
        bool isNumber;
        value.toDouble(&isNumber);
        if (!isNumber) {
            QMessageBox::warning(this, "错误", "价格必须是数字！");
            return;
        }
    }
    else if (field == "零售价") {
        dbField = "retail_price";
        // 验证价格是否为数字
        bool isNumber;
        value.toDouble(&isNumber);
        if (!isNumber) {
            QMessageBox::warning(this, "错误", "价格必须是数字！");
            return;
        }
    }
    else {
        QMessageBox::warning(this, "错误", "不支持的搜索字段！");
        return;
    }

    if (db.searchProductsWithSort(dbField, value, sortField, sortOrder, results)) {
        updateTableFromResults(results);
    } else {
        QMessageBox::warning(this, "提示", "未找到匹配的商品！");
    }
}

void ProductWidget::onResetClicked()
{
    // 清空搜索条件
    searchEdit->clear();
    searchFieldCombo->setCurrentIndex(0);

    // 重置排序条件
    sortFieldCombo->setCurrentIndex(0);
    sortOrderCombo->setCurrentIndex(0);

    // 刷新数据
    loadProducts();
}

void ProductWidget::onSortChanged()
{
    loadProducts();
}
