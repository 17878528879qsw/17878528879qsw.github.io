#include "billingwidget.h"
#include "billingdatabase.h"
#include <QApplication>
#include <QFont>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QScrollBar>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QInputDialog>
#include <QDateTime>
#include <QTextDocument>
#include <QPrinter>
#include <QPrintDialog>
#include <QPainter>
#include <QFile>
#include <QTextStream>
#include <QDesktopServices>
#include <QUrl>
#include <QMessageBox>
#include <QDebug>

// ==================== AddItemDialog 实现 ====================
AddItemDialog::AddItemDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("添加商品");
    resize(800, 600);

    QVBoxLayout *layout = new QVBoxLayout(this);

    QFormLayout *formLayout = new QFormLayout();

    productNameEdit = new QComboBox();
    productNameEdit->setEditable(true);
    brandEdit = new QLineEdit();
    specEdit = new QLineEdit();
    unitEdit = new QLineEdit();
    quantitySpinBox = new QSpinBox();
    quantitySpinBox->setRange(1, 999999);
    priceSpinBox = new QDoubleSpinBox();
    priceSpinBox->setDecimals(2);
    priceSpinBox->setMinimum(0);
    priceSpinBox->setMaximum(999999999.99);
    remarkEdit = new QTextEdit();

    productImageLabel = new QLabel();
    productImageLabel->setFixedSize(100, 100);
    productImageLabel->setStyleSheet("border: 1px solid #ccc; background-color: #f0f0f0;");
    productImageLabel->setAlignment(Qt::AlignCenter);
    productImageLabel->setText("无图");

    // 添加金额显示标签
    amountLabel = new QLabel("金额：¥0.00");

    //刷新
    refreshButton = new QPushButton("刷新商品列表");

    formLayout->addRow("商品名称:", productNameEdit);
    formLayout->addRow("品牌:", brandEdit);
    formLayout->addRow("规格:", specEdit);
    formLayout->addRow("单位:", unitEdit);
    formLayout->addRow("数量:", quantitySpinBox);
    formLayout->addRow("单价:", priceSpinBox);
    formLayout->addRow("金额:", amountLabel);
    formLayout->addRow("备注:", remarkEdit);
    formLayout->addRow("图片:", productImageLabel);
    formLayout->addRow("", refreshButton);

    layout->addLayout(formLayout);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &AddItemDialog::validateAndAccept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttonBox);

    connect(quantitySpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &AddItemDialog::onQuantityChanged);
    connect(priceSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &AddItemDialog::onQuantityChanged);
    connect(productNameEdit, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AddItemDialog::onProductSelected);
    connect(productNameEdit->lineEdit(), &QLineEdit::textEdited, this, &AddItemDialog::onProductNameEdited);

    connect(refreshButton, &QPushButton::clicked, this, &AddItemDialog::refreshAvailableProducts);

    // 初始化金额显示
    updateAmount();
}

void AddItemDialog::validateAndAccept() {
    QString productName = productNameEdit->currentText();
    int quantity = quantitySpinBox->value();

    QVariantMap selectedProduct;
    bool productFound = false;

    for (const auto& product : availableProducts) {
        if (product["name"].toString() == productName) {
            selectedProduct = product;
            productFound = true;
            break;
        }
    }

    if (!productFound) {
        QMessageBox::warning(this, "错误", "未找到商品，请确认商品名称正确！");
        return;
    }

    int stockQuantity = selectedProduct["stock_quantity"].toInt();
    if (quantity > stockQuantity) {
        QMessageBox::warning(this, "库存不足",
                             QString("库存数量为 %1，您要添加的数量为 %2，库存不足！")
                             .arg(stockQuantity).arg(quantity));
        return;
    }

    accept();
}

void AddItemDialog::onQuantityChanged() {
    // 更新金额显示
    updateAmount();
}

void AddItemDialog::refreshAvailableProducts() {
    // 重新从数据库加载商品数据
    BillingDatabaseManager* dbManager = &BillingDatabaseManager::instance();
    QList<QVariantMap> products;
    if (dbManager->getAvailableProducts(products)) {
        availableProducts = products;
        // 更新下拉框
        QString currentText = productNameEdit->currentText();
        productNameEdit->clear();
        for (const auto& product : products) {
            productNameEdit->addItem(product["name"].toString());
        }
        // 保持当前输入的文本
        productNameEdit->setEditText(currentText);
    } else {
        QMessageBox::warning(this, "错误", "刷新商品列表失败！");
    }
}

void AddItemDialog::updateAmount() {
    double amount = quantitySpinBox->value() * priceSpinBox->value();
    amountLabel->setText(QString("金额：¥%1").arg(amount, 0, 'f', 2));
}

void AddItemDialog::onProductSelected(int index) {
    if (index >= 0 && index < availableProducts.size()) {
        QVariantMap product = availableProducts.at(index);
        brandEdit->setText(product["brand"].toString());
        specEdit->setText(product["spec"].toString());
        unitEdit->setText(product["unit"].toString());
        priceSpinBox->setValue(product["wholesale_price"].toDouble());

        int stockQuantity = product["stock_quantity"].toInt();
        quantitySpinBox->setMaximum(stockQuantity);

        // 获取图片路径并显示（从数据库中获取）
        QString imagePath = product.contains("product_image") ? product["product_image"].toString() : "";

        if (!imagePath.isEmpty()) {
            // 检查文件是否存在
            if (QFile::exists(imagePath)) {
                QPixmap pixmap(imagePath);
                if (!pixmap.isNull()) {
                    // 缩放图片以适应标签大小
                    pixmap = pixmap.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                    productImageLabel->setPixmap(pixmap);
                    productImageLabel->setStyleSheet("border: 1px solid #ccc; background-color: white;");
                } else {
                    productImageLabel->setText("图片无效");
                    productImageLabel->setStyleSheet("border: 1px solid #ccc; background-color: #f0f0f0;");
                }
            } else {
                productImageLabel->setText("图片不存在");
                productImageLabel->setStyleSheet("border: 1px solid #ccc; background-color: #f0f0f0;");
            }
        } else {
            productImageLabel->setText("无图");
            productImageLabel->setStyleSheet("border: 1px solid #ccc; background-color: #f0f0f0;");
        }
    }
    updateAmount();
}

void AddItemDialog::onProductNameEdited(const QString &text) {
    brandEdit->clear();
    specEdit->clear();
    unitEdit->clear();
    priceSpinBox->setValue(0.0);
    quantitySpinBox->setMaximum(999999);
    productImageLabel->setText("无图");
    updateAmount();
}

void AddItemDialog::setProductData(const QVariantMap &product) {
    productNameEdit->setCurrentText(product["product_name"].toString());
    brandEdit->setText(product["brand"].toString());
    specEdit->setText(product["specification"].toString());
    unitEdit->setText(product["unit"].toString());
    priceSpinBox->setValue(product["unit_price"].toDouble());
    quantitySpinBox->setValue(product["quantity"].toInt());

    int stockQuantity = product.contains("stock_quantity") ? product["stock_quantity"].toInt() : 999999;
    quantitySpinBox->setMaximum(stockQuantity);

    QString imagePath = product.contains("product_image") ? product["product_image"].toString() : "";
    if (!imagePath.isEmpty() && QFile::exists(imagePath)) {
        QPixmap pixmap(imagePath);
        if (!pixmap.isNull()) {
            pixmap = pixmap.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            productImageLabel->setPixmap(pixmap);
        } else {
            productImageLabel->setText("图片无效");
        }
    } else {
        productImageLabel->setText("无图");
    }
    updateAmount();
}

void AddItemDialog::setProducts(const QList<QVariantMap>& products) {
    availableProducts = products;
    productNameEdit->clear();
    for (const auto& product : products) {
        productNameEdit->addItem(product["name"].toString());
    }
}

QVariantMap AddItemDialog::getItemData() const {
    QVariantMap data;
    data["product_name"] = productNameEdit->currentText();
    data["brand"] = brandEdit->text();
    data["specification"] = specEdit->text();
    data["unit"] = unitEdit->text();
    data["quantity"] = quantitySpinBox->value();
    data["unit_price"] = priceSpinBox->value();
    data["amount"] = quantitySpinBox->value() * priceSpinBox->value();
    data["remarks"] = remarkEdit->toPlainText();

    for (const auto& product : availableProducts) {
        if (product["name"].toString() == data["product_name"].toString()) {
            data["product_id"] = product["id"].toInt();
            data["product_image"] = product["product_image"].toString();
            break;
        }
    }

    return data;
}

// ==================== BillingWidget 实现 ====================

BillingWidget::BillingWidget(QWidget *parent) : QWidget(parent), dbManager(&BillingDatabaseManager::instance()) {
    setupUI();
    initDefaultData();
    loadCustomers();
    loadProducts();
}

void BillingWidget::setupUI() {
    setStyleSheet(R"(
        QWidget {
            font-family: "Microsoft YaHei";
            font-size: 18px;
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
            padding: 10px;
            border: 1px solid #CCCCCC;
            border-radius: 6px;
            background-color: white;
        }
        QPushButton {
            padding: 10px 20px;
            border: 1px solid #3498DB;
            border-radius: 6px;
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
        QTableWidget {
            border: 1px solid #CCCCCC;
            border-radius: 6px;
            gridline-color: #E0E0E0;
        }
        QHeaderView::section {
            background-color: #3498DB;
            color: white;
            padding: 10px;
            font-weight: bold;
            border: none;
        }
    )");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);

    // 客户信息
    QGroupBox *customerGroup = new QGroupBox("客户信息");
    QHBoxLayout *customerLayout = new QHBoxLayout();

    QFormLayout *leftForm = new QFormLayout();
    customerCombo = new QComboBox();
    customerCombo->setEditable(true);
    customerAddressEdit = new QLineEdit();
    contactPersonEdit = new QLineEdit();
    phoneEdit = new QLineEdit();
    leftForm->addRow("客户名称:", customerCombo);
    leftForm->addRow("客户地址:", customerAddressEdit);
    leftForm->addRow("联系人:", contactPersonEdit);
    leftForm->addRow("联系电话:", phoneEdit);

    QFormLayout *rightForm = new QFormLayout();
    salesmanEdit = new QLineEdit();
    paymentMethodCombo = new QComboBox();
    paymentMethodCombo->addItems({"微信支付", "支付宝", "现金", "银行转账"});
    storeEdit = new QLineEdit();
    rightForm->addRow("业务员:", salesmanEdit);
    rightForm->addRow("付款方式:", paymentMethodCombo);
    rightForm->addRow("门店:", storeEdit);

    QWidget *leftWidget = new QWidget();
    leftWidget->setLayout(leftForm);
    QWidget *rightWidget = new QWidget();
    rightWidget->setLayout(rightForm);

    customerLayout->addWidget(leftWidget);
    customerLayout->addWidget(rightWidget);
    customerGroup->setLayout(customerLayout);

    // 商品明细
    QGroupBox *itemsGroup = new QGroupBox("商品明细");
    QVBoxLayout *itemsLayout = new QVBoxLayout();

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    addItemBtn = new QPushButton("添加商品");
    removeItemBtn = new QPushButton("删除商品");
    modifyItemBtn = new QPushButton("修改商品");
    refreshBtn = new QPushButton("刷新"); // 新增刷新按钮
    buttonLayout->addWidget(addItemBtn);
    buttonLayout->addWidget(removeItemBtn);
    buttonLayout->addWidget(modifyItemBtn);
    buttonLayout->addWidget(refreshBtn); // 添加到按钮布局
    buttonLayout->addStretch();

    itemsLayout->addLayout(buttonLayout);

    itemsTable = new QTableWidget();
    itemsTable->setColumnCount(9);
    QStringList headers = {"序号", "商品名称", "品牌", "规格", "单位", "数量", "单价", "金额", "备注"};
    itemsTable->setHorizontalHeaderLabels(headers);
    itemsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    itemsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    itemsTable->verticalHeader()->setVisible(false);
    itemsTable->horizontalHeader()->setStretchLastSection(true);
    itemsTable->setAlternatingRowColors(true);

    itemsTable->setColumnWidth(0, 80);
    itemsTable->setColumnWidth(1, 180);
    itemsTable->setColumnWidth(2, 120);
    itemsTable->setColumnWidth(3, 100);
    itemsTable->setColumnWidth(4, 80);
    itemsTable->setColumnWidth(5, 80);
    itemsTable->setColumnWidth(6, 80);
    itemsTable->setColumnWidth(7, 80);
    itemsTable->setColumnWidth(8, 150);

    itemsLayout->addWidget(itemsTable);
    itemsGroup->setLayout(itemsLayout);

    // 总金额
    QHBoxLayout *totalLayout = new QHBoxLayout();
    totalAmountLabel = new QLabel("合计金额（元）：¥0.00");
    totalAmountWordsLabel = new QLabel("合计金额大写：零元整");
    totalLayout->addWidget(totalAmountLabel);
    totalLayout->addWidget(totalAmountWordsLabel);
    totalLayout->addStretch();

    // 操作按钮
    QHBoxLayout *actionLayout = new QHBoxLayout();
    exportBtn = new QPushButton("导出");
    printBtn = new QPushButton("打印");
    issueBtn = new QPushButton("出单");
    clearBtn = new QPushButton("清空");

    actionLayout->addWidget(exportBtn);
    actionLayout->addWidget(printBtn);
    actionLayout->addWidget(issueBtn);
    actionLayout->addWidget(clearBtn);

    mainLayout->addWidget(customerGroup);
    mainLayout->addWidget(itemsGroup);
    mainLayout->addLayout(totalLayout);
    mainLayout->addLayout(actionLayout);

    // 连接信号槽
    connect(addItemBtn, &QPushButton::clicked, this, &BillingWidget::onAddItemClicked);
    connect(removeItemBtn, &QPushButton::clicked, this, &BillingWidget::onRemoveItemClicked);
    connect(modifyItemBtn, &QPushButton::clicked, this, &BillingWidget::onModifyItemClicked);
    connect(refreshBtn, &QPushButton::clicked, this, &BillingWidget::onRefreshClicked); // 连接刷新按钮
    connect(exportBtn, &QPushButton::clicked, this, &BillingWidget::onExportClicked);
    connect(printBtn, &QPushButton::clicked, this, &BillingWidget::onPrintClicked);
    connect(issueBtn, &QPushButton::clicked, this, &BillingWidget::onIssueClicked);
    connect(clearBtn, &QPushButton::clicked, this, &BillingWidget::onClearClicked);
    connect(customerCombo, &QComboBox::currentTextChanged, this, &BillingWidget::onCustomerSelected);
    connect(customerCombo->lineEdit(), &QLineEdit::textEdited, this, &BillingWidget::onCustomerManualChanged);
}

void BillingWidget::initDefaultData() {
    orderItems.clear();
    totalAmount = 0.0;
    orderNo = "";
    orderDate = QDateTime::currentDateTime().toString("yyyy-MM-dd");
    status = "待处理";
    createdBy = "管理员";

    customerCombo->setCurrentText("");
    customerAddressEdit->clear();
    contactPersonEdit->clear();
    phoneEdit->clear();
    salesmanEdit->clear();
    paymentMethodCombo->setCurrentIndex(0);
    storeEdit->clear();
}

void BillingWidget::loadCustomers() {
    QList<QVariantMap> customers;
    if (dbManager->getAllCustomers(customers)) {
        customerCombo->clear();
        for (const auto& customer : customers) {
            customerCombo->addItem(customer["customer_name"].toString());
        }
    }
}

void BillingWidget::loadProducts() {
    availableProducts.clear();
    if (dbManager->getAvailableProducts(availableProducts)) {
        // 图片路径现在从数据库中获取，不再硬编码
    }
}

void BillingWidget::addOrderItem(const QVariantMap &itemData) {
    int row = itemsTable->rowCount();
    itemsTable->insertRow(row);

    itemsTable->setItem(row, 0, new QTableWidgetItem(QString::number(row + 1)));
    itemsTable->setItem(row, 1, new QTableWidgetItem(itemData["product_name"].toString()));
    itemsTable->setItem(row, 2, new QTableWidgetItem(itemData["brand"].toString()));
    itemsTable->setItem(row, 3, new QTableWidgetItem(itemData["specification"].toString()));
    itemsTable->setItem(row, 4, new QTableWidgetItem(itemData["unit"].toString()));
    itemsTable->setItem(row, 5, new QTableWidgetItem(QString::number(itemData["quantity"].toInt())));
    itemsTable->setItem(row, 6, new QTableWidgetItem(QString::number(itemData["unit_price"].toDouble(), 'f', 2)));
    itemsTable->setItem(row, 7, new QTableWidgetItem(QString::number(itemData["amount"].toDouble(), 'f', 2)));
    itemsTable->setItem(row, 8, new QTableWidgetItem(itemData["remarks"].toString()));

    orderItems.append(itemData);
    updateTotalAmount();
}

void BillingWidget::updateTotalAmount() {
    totalAmount = 0.0;
    for (const auto& item : orderItems) {
        totalAmount += item["amount"].toDouble();
    }

    totalAmountLabel->setText(QString("合计金额（元）：¥%1").arg(totalAmount, 0, 'f', 2));

    QString chineseAmount;
    convertAmountToChinese(totalAmount, chineseAmount);
    totalAmountWordsLabel->setText(QString("合计金额大写：%1").arg(chineseAmount));
}

void BillingWidget::generateOrderNo() {
    orderNo = "PO" + QDateTime::currentDateTime().toString("yyyyMMdd") + QString::number(QDateTime::currentDateTime().toTime_t());
}

void BillingWidget::saveOrderToDatabase() {
    generateOrderNo();

    QVariantMap orderData;
    orderData["order_no"] = orderNo;
    orderData["customer_name"] = customerCombo->currentText();
    orderData["customer_address"] = customerAddressEdit->text();
    orderData["contact_person"] = contactPersonEdit->text();
    orderData["phone"] = phoneEdit->text();
    orderData["salesman"] = salesmanEdit->text();
    orderData["payment_method"] = paymentMethodCombo->currentText();
    orderData["store"] = storeEdit->text();
    orderData["total_amount"] = totalAmount;
    orderData["total_amount_words"] = totalAmountWordsLabel->text().mid(8);
    orderData["order_date"] = orderDate;
    orderData["status"] = status;
    orderData["created_by"] = createdBy;
    orderData["created_at"] = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    orderData["updated_at"] = orderData["created_at"];

    if (dbManager->insertOrder(orderData)) {
        qint64 orderId = dbManager->getLastInsertId();
        for (const auto& item : orderItems) {
            QVariantMap itemData = item;
            itemData["order_id"] = orderId;
            itemData["created_at"] = orderData["created_at"];
            itemData["updated_at"] = orderData["created_at"];

            // 插入订单项
            dbManager->insertOrderItem(orderId, itemData);

            // 更新库存
            int productId = itemData["product_id"].toInt();
            int quantity = itemData["quantity"].toInt();
            dbManager->updateProductStock(productId, quantity);
        }
    } else {
        QMessageBox::warning(this, "保存失败", "订单保存失败，请检查数据！");
    }
}

void BillingWidget::printOrder() {
    QPrinter printer;
    printer.setOrientation(QPrinter::Portrait);
    printer.setColorMode(QPrinter::Color);

    QPrintDialog dialog(&printer, this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    QPainter painter(&printer);
    painter.setFont(QFont("Microsoft YaHei", 16));

    painter.drawText(250, 50, "送货单");

    painter.setFont(QFont("Microsoft YaHei", 12));
    painter.drawText(50, 100, QString("订单编号：%1").arg(orderNo));
    painter.drawText(300, 100, QString("开单日期：%1").arg(orderDate));
    painter.drawText(50, 130, QString("客户名称：%1").arg(customerCombo->currentText()));
    painter.drawText(50, 160, QString("客户地址：%1").arg(customerAddressEdit->text()));
    painter.drawText(50, 190, QString("联系人：%1").arg(contactPersonEdit->text()));
    painter.drawText(300, 190, QString("联系电话：%1").arg(phoneEdit->text()));
    painter.drawText(50, 220, QString("业务员：%1").arg(salesmanEdit->text()));
    painter.drawText(300, 220, QString("门店：%1").arg(storeEdit->text()));
    painter.drawText(50, 250, QString("付款方式：%1").arg(paymentMethodCombo->currentText()));

    int y = 290;
    painter.setFont(QFont("Microsoft YaHei", 10, QFont::Bold));
    painter.drawText(50, y, "序号");
    painter.drawText(100, y, "商品名称");
    painter.drawText(200, y, "品牌");
    painter.drawText(260, y, "规格");
    painter.drawText(320, y, "单位");
    painter.drawText(370, y, "数量");
    painter.drawText(420, y, "单价");
    painter.drawText(470, y, "金额");
    painter.drawText(520, y, "备注");

    painter.drawLine(50, y + 5, 700, y + 5);

    y += 25;
    painter.setFont(QFont("Microsoft YaHei", 10));
    for (int i = 0; i < orderItems.size(); ++i) {
        const auto& item = orderItems[i];
        painter.drawText(50, y, QString::number(i + 1));
        painter.drawText(100, y, item["product_name"].toString());
        painter.drawText(200, y, item["brand"].toString());
        painter.drawText(260, y, item["specification"].toString());
        painter.drawText(320, y, item["unit"].toString());
        painter.drawText(370, y, QString::number(item["quantity"].toInt()));
        painter.drawText(420, y, QString::number(item["unit_price"].toDouble(), 'f', 2));
        painter.drawText(470, y, QString::number(item["amount"].toDouble(), 'f', 2));
        painter.drawText(520, y, item["remarks"].toString());
        y += 25;
    }

    painter.drawLine(50, y, 700, y);
    y += 30;

    painter.setFont(QFont("Microsoft YaHei", 12, QFont::Bold));
    painter.drawText(50, y, QString("合计金额：%1 元").arg(QString::number(totalAmount, 'f', 2)));
    painter.drawText(50, y + 30, QString("合计金额大写：%1").arg(totalAmountWordsLabel->text().mid(8)));
}

void BillingWidget::clearOrder() {
    itemsTable->clearContents();
    itemsTable->setRowCount(0);
    orderItems.clear();
    updateTotalAmount();

    customerCombo->setCurrentIndex(-1);
    customerAddressEdit->clear();
    contactPersonEdit->clear();
    phoneEdit->clear();
    salesmanEdit->clear();
    storeEdit->clear();
}

void BillingWidget::showItemDialog() {
    // 每次打开对话框时都从数据库重新加载商品数据
    loadProducts();

    if (availableProducts.isEmpty()) {
        QMessageBox::warning(this, "提示", "暂无可用商品，请先添加商品！");
        return;
    }

    AddItemDialog dialog(this);
    dialog.setProducts(availableProducts);
    // 每次打开对话框时强制刷新商品列表
    dialog.refreshAvailableProducts();

    if (dialog.exec() == QDialog::Accepted) {
        addOrderItem(dialog.getItemData());
    }
}

void BillingWidget::convertAmountToChinese(double amount, QString &chineseAmount) {
    static const QString digits[] = {"零", "壹", "贰", "叁", "肆", "伍", "陆", "柒", "捌", "玖"};
    static const QString units[] = {"", "拾", "佰", "仟", "万", "拾", "佰", "仟", "亿"};

    // 格式化金额为保留两位小数的字符串
    QString str = QString::number(amount, 'f', 2);
    int dotPos = str.indexOf('.');
    QString integerPart = str.left(dotPos);
    QString decimalPart = str.mid(dotPos + 1);

    // 处理整数部分
    QString result = "";
    int len = integerPart.length();

    for (int i = 0; i < len; ++i) {
        int digit = integerPart[i].digitValue();
        if (digit != 0) {
            result += digits[digit] + units[len - i - 1];
        } else {
            // 只有当下一位不是0时才添加零，并且避免在末尾添加多余的零
            if (i < len - 1) {  // 不是最后一位
                bool hasNextNonZero = false;
                for (int j = i + 1; j < len; ++j) {
                    if (integerPart[j].digitValue() != 0) {
                        hasNextNonZero = true;
                        break;
                    }
                }
                if (hasNextNonZero && !result.isEmpty() && result.right(1) != "零") {
                    result += "零";
                }
            }
        }
    }

    // 清理多余的"零"
    while (result.contains("零零")) {
        result.replace("零零", "零");
    }
    result.replace("零万", "万");
    result.replace("零亿", "亿");
    result.replace("零点", "点");

    // 如果整数部分为空，则设为"零"
    if (result.isEmpty()) {
        result = "零";
    }

    // 处理小数部分
    if (decimalPart[0] != '0' || decimalPart[1] != '0') {
        result += "点";
        result += digits[decimalPart[0].digitValue()];
        if (decimalPart[1] != '0') {
            result += digits[decimalPart[1].digitValue()];
        }
    }

    // 最终清理：避免"零拾"、"零佰"等不合理组合
    result = result.replace("零拾", "拾").replace("零佰", "佰").replace("零仟", "仟");
    result = result.replace("零万", "万").replace("零亿", "亿");

    chineseAmount = result;
}


QString BillingWidget::generateOrderText() {
    QString text = "送货单\n";
    text += QString("订单编号：%1 开单日期：%2\n").arg(orderNo).arg(orderDate);
    text += QString("客户名称：%1\n").arg(customerCombo->currentText());
    text += QString("客户地址：%1\n").arg(customerAddressEdit->text());
    text += QString("联系人：%1\n").arg(contactPersonEdit->text());
    text += QString("联系电话：%1\n").arg(phoneEdit->text());
    text += QString("业务员：%1\n").arg(salesmanEdit->text());
    text += QString("门店：%1\n").arg(storeEdit->text());
    text += QString("付款方式：%1\n").arg(paymentMethodCombo->currentText());

    text += "\n序号 商品名称 品牌 规格 单位 数量 单价 金额 备注\n";
    text += "--------------------------------------------------------\n";
    for (int i = 0; i < orderItems.size(); ++i) {
        const auto& item = orderItems[i];
        text += QString("%1 %2 %3 %4 %5 %6 %7 %8 %9\n")
            .arg(i+1)
            .arg(item["product_name"].toString())
            .arg(item["brand"].toString())
            .arg(item["specification"].toString())
            .arg(item["unit"].toString())
            .arg(item["quantity"].toInt())
            .arg(item["unit_price"].toDouble(), 0, 'f', 2)
            .arg(item["amount"].toDouble(), 0, 'f', 2)
            .arg(item["remarks"].toString());
    }

    text += "\n合计金额：" + QString::number(totalAmount, 'f', 2) + " 元\n";
    text += "合计金额大写：" + totalAmountWordsLabel->text().mid(8) + "\n";

    return text;
}

void BillingWidget::exportToPDF() {
    // 确保订单编号已生成
    if (orderNo.isEmpty()) {
        generateOrderNo();
    }

    QString fileName = QFileDialog::getSaveFileName(this, "导出PDF",
        QString("送货单_%1.pdf").arg(orderNo),
        "PDF Files (*.pdf)");

    if (fileName.isEmpty()) {
        return;
    }

    QPrinter printer;
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    printer.setPageSize(QPrinter::A4);
    printer.setOrientation(QPrinter::Portrait);

    QPainter painter(&printer);
    painter.setFont(QFont("Microsoft YaHei", 16));

    painter.drawText(250, 50, "送货单");

    painter.setFont(QFont("Microsoft YaHei", 12));
    painter.drawText(50, 100, QString("订单编号：%1").arg(orderNo));
    painter.drawText(300, 100, QString("开单日期：%1").arg(orderDate));
    painter.drawText(50, 130, QString("客户名称：%1").arg(customerCombo->currentText()));
    painter.drawText(50, 160, QString("客户地址：%1").arg(customerAddressEdit->text()));
    painter.drawText(50, 190, QString("联系人：%1").arg(contactPersonEdit->text()));
    painter.drawText(300, 190, QString("联系电话：%1").arg(phoneEdit->text()));
    painter.drawText(50, 220, QString("业务员：%1").arg(salesmanEdit->text()));
    painter.drawText(300, 220, QString("门店：%1").arg(storeEdit->text()));
    painter.drawText(50, 250, QString("付款方式：%1").arg(paymentMethodCombo->currentText()));

    int y = 290;
    painter.setFont(QFont("Microsoft YaHei", 10, QFont::Bold));
    painter.drawText(50, y, "序号");
    painter.drawText(100, y, "商品名称");
    painter.drawText(200, y, "品牌");
    painter.drawText(260, y, "规格");
    painter.drawText(320, y, "单位");
    painter.drawText(370, y, "数量");
    painter.drawText(420, y, "单价");
    painter.drawText(470, y, "金额");
    painter.drawText(520, y, "备注");

    painter.drawLine(50, y + 5, 700, y + 5);

    y += 25;
    painter.setFont(QFont("Microsoft YaHei", 10));
    for (int i = 0; i < orderItems.size(); ++i) {
        const auto& item = orderItems[i];
        painter.drawText(50, y, QString::number(i + 1));
        painter.drawText(100, y, item["product_name"].toString());
        painter.drawText(200, y, item["brand"].toString());
        painter.drawText(260, y, item["specification"].toString());
        painter.drawText(320, y, item["unit"].toString());
        painter.drawText(370, y, QString::number(item["quantity"].toInt()));
        painter.drawText(420, y, QString::number(item["unit_price"].toDouble(), 'f', 2));
        painter.drawText(470, y, QString::number(item["amount"].toDouble(), 'f', 2));
        painter.drawText(520, y, item["remarks"].toString());
        y += 25;
    }

    painter.drawLine(50, y, 700, y);
    y += 30;

    painter.setFont(QFont("Microsoft YaHei", 12, QFont::Bold));
    painter.drawText(50, y, QString("合计金额：%1 元").arg(QString::number(totalAmount, 'f', 2)));
    QString chineseAmount;
    convertAmountToChinese(totalAmount, chineseAmount);
    painter.drawText(50, y + 30, QString("合计金额大写：%1").arg(chineseAmount));

    QMessageBox::information(this, "导出成功", "订单已成功导出为PDF文件！");
}

// ==================== Slots ====================

void BillingWidget::onAddItemClicked() {
    showItemDialog();
}

void BillingWidget::onRemoveItemClicked() {
    int row = itemsTable->currentRow();
    if (row >= 0) {
        itemsTable->removeRow(row);
        orderItems.removeAt(row);
        updateTotalAmount();
    } else {
        QMessageBox::warning(this, "警告", "请先选择要删除的商品！");
    }
}

void BillingWidget::onModifyItemClicked() {
    int row = itemsTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "警告", "请先选择要修改的商品！");
        return;
    }

    QVariantMap itemData;
    itemData["product_name"] = itemsTable->item(row, 1)->text();
    itemData["brand"] = itemsTable->item(row, 2)->text();
    itemData["specification"] = itemsTable->item(row, 3)->text();
    itemData["unit"] = itemsTable->item(row, 4)->text();
    itemData["quantity"] = itemsTable->item(row, 5)->text().toInt();
    itemData["unit_price"] = itemsTable->item(row, 6)->text().toDouble();
    itemData["amount"] = itemsTable->item(row, 7)->text().toDouble();
    itemData["remarks"] = itemsTable->item(row, 8)->text();

    AddItemDialog dialog(this);
    dialog.setProductData(itemData);
    dialog.setProducts(availableProducts);
    if (dialog.exec() == QDialog::Accepted) {
        QVariantMap newData = dialog.getItemData();
        orderItems[row] = newData;

        itemsTable->setItem(row, 1, new QTableWidgetItem(newData["product_name"].toString()));
        itemsTable->setItem(row, 2, new QTableWidgetItem(newData["brand"].toString()));
        itemsTable->setItem(row, 3, new QTableWidgetItem(newData["specification"].toString()));
        itemsTable->setItem(row, 4, new QTableWidgetItem(newData["unit"].toString()));
        itemsTable->setItem(row, 5, new QTableWidgetItem(QString::number(newData["quantity"].toInt())));
        itemsTable->setItem(row, 6, new QTableWidgetItem(QString::number(newData["unit_price"].toDouble(), 'f', 2)));
        itemsTable->setItem(row, 7, new QTableWidgetItem(QString::number(newData["amount"].toDouble(), 'f', 2)));
        itemsTable->setItem(row, 8, new QTableWidgetItem(newData["remarks"].toString()));

        updateTotalAmount();
    }
}

void BillingWidget::onExportClicked() {
    if (orderItems.isEmpty()) {
        QMessageBox::warning(this, "警告", "订单为空，无法导出！");
        return;
    }

    exportToPDF();
}

void BillingWidget::onPrintClicked() {
    if (orderItems.isEmpty()) {
        QMessageBox::warning(this, "警告", "订单为空，无法打印！");
        return;
    }

    printOrder();
}

void BillingWidget::onIssueClicked() {
    if (orderItems.isEmpty()) {
        QMessageBox::warning(this, "警告", "订单为空，无法出单！");
        return;
    }

    saveOrderToDatabase();
    QMessageBox::information(this, "出单成功", "订单已保存并出单！\n库存已更新！");
    onPrintClicked(); // 自动打印
}

void BillingWidget::onClearClicked() {
    if (QMessageBox::question(this, "确认", "确定要清空当前订单吗？",
                             QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        clearOrder();
    }
}

void BillingWidget::onCustomerSelected(const QString &name) {
    if (name.isEmpty()) return;

    QVariantMap customer;
    if (dbManager->getCustomerByName(name, customer)) {
        customerAddressEdit->setText(customer["customer_address"].toString());
        contactPersonEdit->setText(customer["contact_person"].toString());
        phoneEdit->setText(customer["phone"].toString());
    }
}

void BillingWidget::onCustomerManualChanged() {
    // 手动输入时，清空其他字段
    customerAddressEdit->clear();
    contactPersonEdit->clear();
    phoneEdit->clear();
}

// 新增刷新按钮槽函数
void BillingWidget::onRefreshClicked() {
    loadCustomers();
    loadProducts();
    QMessageBox::information(this, "刷新成功", "客户和商品数据已刷新！");
}
