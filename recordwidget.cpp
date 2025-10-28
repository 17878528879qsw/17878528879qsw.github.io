#include "recordwidget.h"
#include "recorddatabase.h"
#include <QApplication>
#include <QFont>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QScrollBar>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QInputDialog>

CustomerDialog::CustomerDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("客户信息");
    resize(400, 300);

    QVBoxLayout *layout = new QVBoxLayout(this);

    QFormLayout *formLayout = new QFormLayout();

    nameEdit = new QLineEdit();
    addressEdit = new QLineEdit();
    contactEdit = new QLineEdit();
    phoneEdit = new QLineEdit();
    totalOrdersSpinBox = new QSpinBox();
    totalOrdersSpinBox->setRange(0, 999999);
    totalAmountSpinBox = new QDoubleSpinBox();
    totalAmountSpinBox->setRange(0, 9999999.99);
    totalAmountSpinBox->setDecimals(2);

    formLayout->addRow("客户名称:", nameEdit);
    formLayout->addRow("客户地址:", addressEdit);
    formLayout->addRow("联系人:", contactEdit);
    formLayout->addRow("联系电话:", phoneEdit);
    formLayout->addRow("总订单数:", totalOrdersSpinBox);
    formLayout->addRow("总金额:", totalAmountSpinBox);

    layout->addLayout(formLayout);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttonBox);
}

void CustomerDialog::setCustomerData(const QVariantMap &data)
{
    nameEdit->setText(data.value("customer_name").toString());
    addressEdit->setText(data.value("customer_address").toString());
    contactEdit->setText(data.value("contact_person").toString());
    phoneEdit->setText(data.value("phone").toString());
    totalOrdersSpinBox->setValue(data.value("total_orders").toInt());
    totalAmountSpinBox->setValue(data.value("total_amount").toDouble());
}

QVariantMap CustomerDialog::getCustomerData() const
{
    QVariantMap data;
    data["customer_name"] = nameEdit->text();
    data["customer_address"] = addressEdit->text();
    data["contact_person"] = contactEdit->text();
    data["phone"] = phoneEdit->text();
    data["total_orders"] = totalOrdersSpinBox->value();
    data["total_amount"] = totalAmountSpinBox->value();
    return data;
}

RecordWidget::RecordWidget(QWidget *parent)
    : QWidget(parent)
    , currentOrderId(-1)
    , currentCustomerId(-1)
    , currentOrderSortField("ID")
    , currentOrderSortOrder("降序")
    , currentCustomerSortField("ID")
    , currentCustomerSortOrder("降序")
{
    // 初始化数据库
    RecordDatabaseManager::instance().connect();

    setupUI();
    loadOrders();
    loadCustomers();
}

void RecordWidget::setupUI()
{
    setStyleSheet(R"(
        QWidget {
            font-family: "Microsoft YaHei";
            font-size: 18px;
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
        }
        QLineEdit, QComboBox, QDateEdit, QSpinBox, QDoubleSpinBox {
            padding: 15px;
            border: 2px solid #CCCCCC;
            border-radius: 8px;
            background-color: white;
            font-size: 16px;
        }
        QPushButton {
            padding: 15px 30px;
            border: 2px solid #3498DB;
            border-radius: 8px;
            background-color: #3498DB;
            color: white;
            font-weight: bold;
            font-size: 16px;
            margin: 5px;
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
            border-radius: 8px;
            gridline-color: #E0E0E0;
            font-size: 16px;
        }
        QHeaderView::section {
            background-color: #3498DB;
            color: white;
            padding: 15px;
            font-weight: bold;
            font-size: 16px;
            border: none;
        }
        QTabWidget::pane {
            border: 2px solid #CCCCCC;
            border-radius: 8px;
        }
        QTabBar::tab {
            background: #E0E0E0;
            border: 2px solid #CCCCCC;
            border-bottom-color: #CCCCCC;
            border-top-left-radius: 8px;
            border-top-right-radius: 8px;
            min-width: 12ex;
            padding: 15px;
            font-size: 16px;
        }
        QTabBar::tab:selected {
            background: #3498DB;
            color: white;
        }
        QLabel {
            font-size: 16px;
            margin: 0 5px;
        }
    )");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setSpacing(30); // 加大组件间距

    tabWidget = new QTabWidget(this);

    // 订单记录页面
    QWidget *orderPage = new QWidget();
    QVBoxLayout *orderLayout = new QVBoxLayout(orderPage);
    orderLayout->setSpacing(20);

    // 订单搜索栏 + 功能按钮（合并）
    QHBoxLayout *orderSearchLayout = new QHBoxLayout();
    orderSearchLayout->setSpacing(15);
    orderSearchEdit = new QLineEdit(this);
    orderSearchEdit->setMaximumWidth(220);
    orderSearchEdit->setPlaceholderText("请输入搜索内容");
    orderSearchFieldCombo = new QComboBox(this);
    orderSearchFieldCombo->addItem("订单号");
    orderSearchFieldCombo->addItem("客户名称");
    orderSearchFieldCombo->addItem("联系人");
    orderSearchFieldCombo->addItem("联系电话");
    orderSearchFieldCombo->addItem("总金额");
    orderSearchFieldCombo->addItem("订单状态");
    startDateEdit = new QDateEdit(this);
    startDateEdit->setDate(QDate::currentDate().addDays(-30));
    endDateEdit = new QDateEdit(this);
    endDateEdit->setDate(QDate::currentDate());
    orderSearchBtn = new QPushButton("搜索", this);
    orderResetBtn = new QPushButton("重置", this);
    viewDetailsBtn = new QPushButton("查看详情", this);
    viewDetailsBtn->setEnabled(false);
    completeOrderBtn = new QPushButton("完成订单", this); // 新增完成订单按钮
    completeOrderBtn->setEnabled(false);
    deleteOrderBtn = new QPushButton("删除订单", this);
    deleteOrderBtn->setEnabled(false);

    connect(orderSearchBtn, &QPushButton::clicked, this, &RecordWidget::onSearchOrderClicked);
    connect(orderResetBtn, &QPushButton::clicked, this, &RecordWidget::onResetOrderClicked);
    connect(viewDetailsBtn, &QPushButton::clicked, this, &RecordWidget::onViewOrderDetails);
    connect(completeOrderBtn, &QPushButton::clicked, this, &RecordWidget::onCompleteOrder); // 连接完成订单槽函数
    connect(deleteOrderBtn, &QPushButton::clicked, this, &RecordWidget::onDeleteOrder);

    orderSearchLayout->addWidget(new QLabel("搜索:"));
    orderSearchLayout->addWidget(orderSearchEdit);
    orderSearchLayout->addWidget(orderSearchFieldCombo);
    orderSearchLayout->addWidget(new QLabel("日期范围:"));
    orderSearchLayout->addWidget(startDateEdit);
    orderSearchLayout->addWidget(new QLabel("至"));
    orderSearchLayout->addWidget(endDateEdit);
    orderSearchLayout->addWidget(orderSearchBtn);
    orderSearchLayout->addWidget(orderResetBtn);
    orderSearchLayout->addWidget(viewDetailsBtn);
    orderSearchLayout->addWidget(completeOrderBtn); // 添加完成订单按钮
    orderSearchLayout->addWidget(deleteOrderBtn);
    orderSearchLayout->addStretch();

    // 订单表格
    setupOrderTable();

    orderLayout->addLayout(orderSearchLayout);
    orderLayout->addWidget(orderTable);

    // 客户记录页面
    QWidget *customerPage = new QWidget();
    QVBoxLayout *customerLayout = new QVBoxLayout(customerPage);
    customerLayout->setSpacing(20);

    // 客户搜索栏 + 功能按钮（合并）
    QHBoxLayout *customerSearchLayout = new QHBoxLayout();
    customerSearchLayout->setSpacing(15);
    customerSearchEdit = new QLineEdit(this);
    customerSearchEdit->setMaximumWidth(220);
    customerSearchEdit->setPlaceholderText("请输入搜索内容");
    customerSearchFieldCombo = new QComboBox(this);
    customerSearchFieldCombo->addItem("客户名称");
    customerSearchFieldCombo->addItem("客户地址");
    customerSearchFieldCombo->addItem("联系人");
    customerSearchFieldCombo->addItem("联系电话");
    customerSearchFieldCombo->addItem("总订单数");
    customerSearchFieldCombo->addItem("总金额");
    customerSearchBtn = new QPushButton("搜索", this);
    customerResetBtn = new QPushButton("重置", this);
    addCustomerBtn = new QPushButton("新增客户", this);
    editCustomerBtn = new QPushButton("修改客户", this);
    deleteCustomerBtn = new QPushButton("删除客户", this);
    editCustomerBtn->setEnabled(false);
    deleteCustomerBtn->setEnabled(false);

    connect(customerSearchBtn, &QPushButton::clicked, this, &RecordWidget::onSearchCustomerClicked);
    connect(customerResetBtn, &QPushButton::clicked, this, &RecordWidget::onResetCustomerClicked);
    connect(addCustomerBtn, &QPushButton::clicked, this, &RecordWidget::onAddCustomer);
    connect(editCustomerBtn, &QPushButton::clicked, this, &RecordWidget::onEditCustomer);
    connect(deleteCustomerBtn, &QPushButton::clicked, this, &RecordWidget::onDeleteCustomer);

    customerSearchLayout->addWidget(new QLabel("搜索:"));
    customerSearchLayout->addWidget(customerSearchEdit);
    customerSearchLayout->addWidget(customerSearchFieldCombo);
    customerSearchLayout->addWidget(customerSearchBtn);
    customerSearchLayout->addWidget(customerResetBtn);
    customerSearchLayout->addWidget(addCustomerBtn);
    customerSearchLayout->addWidget(editCustomerBtn);
    customerSearchLayout->addWidget(deleteCustomerBtn);
    customerSearchLayout->addStretch();

    // 客户表格
    setupCustomerTable();

    customerLayout->addLayout(customerSearchLayout);
    customerLayout->addWidget(customerTable);

    // 添加标签页
    tabWidget->addTab(orderPage, "订单记录");
    tabWidget->addTab(customerPage, "客户记录");

    mainLayout->addWidget(tabWidget);
}

void RecordWidget::setupOrderTable()
{
    orderTable = new QTableWidget(this);
    orderTable->setColumnCount(8);
    QStringList headers = {"ID", "订单号", "客户名称", "联系人", "联系电话", "总金额", "订单状态", "下单时间"};
    orderTable->setHorizontalHeaderLabels(headers);
    orderTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    orderTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    orderTable->verticalHeader()->setVisible(false);
    orderTable->horizontalHeader()->setStretchLastSection(true);
    orderTable->setAlternatingRowColors(true);
    orderTable->setStyleSheet("QTableWidget { border: 2px solid #ccc; }");

    // 设置列宽
    orderTable->setColumnWidth(0, 100);
    orderTable->setColumnWidth(1, 150);
    orderTable->setColumnWidth(2, 180);
    orderTable->setColumnWidth(3, 120);
    orderTable->setColumnWidth(4, 140);
    orderTable->setColumnWidth(5, 120);
    orderTable->setColumnWidth(6, 120);
    orderTable->setColumnWidth(7, 160);

    connect(orderTable, &QTableWidget::itemSelectionChanged, this, &RecordWidget::onOrderSelected);
    connect(orderTable->horizontalHeader(), &QHeaderView::sectionClicked,
            this, &RecordWidget::onOrderTableHeaderClicked);
}

void RecordWidget::setupCustomerTable()
{
    customerTable = new QTableWidget(this);
    customerTable->setColumnCount(6);
    QStringList headers = {"ID", "客户名称", "客户地址", "联系人", "联系电话", "总订单数"};
    customerTable->setHorizontalHeaderLabels(headers);
    customerTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    customerTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    customerTable->verticalHeader()->setVisible(false);
    customerTable->horizontalHeader()->setStretchLastSection(true);
    customerTable->setAlternatingRowColors(true);
    customerTable->setStyleSheet("QTableWidget { border: 2px solid #ccc; }");

    // 设置列宽
    customerTable->setColumnWidth(0, 80);
    customerTable->setColumnWidth(1, 180);
    customerTable->setColumnWidth(2, 220);
    customerTable->setColumnWidth(3, 120);
    customerTable->setColumnWidth(4, 140);
    customerTable->setColumnWidth(5, 100);

    connect(customerTable, &QTableWidget::itemSelectionChanged, this, &RecordWidget::onCustomerSelected);
    connect(customerTable->horizontalHeader(), &QHeaderView::sectionClicked,
            this, &RecordWidget::onCustomerTableHeaderClicked);
}

void RecordWidget::loadOrders()
{
    clearOrderTable();
    QList<QVariantMap> orders;

    if (!RecordDatabaseManager::instance().getAllOrdersWithSort(
            currentOrderSortField, currentOrderSortOrder, orders)) {
        QMessageBox::critical(this, "错误", "加载订单数据失败！");
        return;
    }

    for (const auto& order : orders) {
        int row = orderTable->rowCount();
        orderTable->insertRow(row);

        orderTable->setItem(row, 0, new QTableWidgetItem(order.value("id").toString()));
        orderTable->setItem(row, 1, new QTableWidgetItem(order.value("order_no").toString()));
        orderTable->setItem(row, 2, new QTableWidgetItem(order.value("customer_name").toString()));
        orderTable->setItem(row, 3, new QTableWidgetItem(order.value("contact_person").toString()));
        orderTable->setItem(row, 4, new QTableWidgetItem(order.value("phone").toString()));
        orderTable->setItem(row, 5, new QTableWidgetItem(order.value("total_amount").toString()));
        orderTable->setItem(row, 6, new QTableWidgetItem(order.value("status").toString()));
        orderTable->setItem(row, 7, new QTableWidgetItem(order.value("order_date").toString()));
    }
}

void RecordWidget::loadCustomers()
{
    clearCustomerTable();
    QList<QVariantMap> customers;

    if (!RecordDatabaseManager::instance().getAllCustomersWithSort(
            currentCustomerSortField, currentCustomerSortOrder, customers)) {
        QMessageBox::critical(this, "错误", "加载客户数据失败！");
        return;
    }

    for (const auto& customer : customers) {
        int row = customerTable->rowCount();
        customerTable->insertRow(row);

        customerTable->setItem(row, 0, new QTableWidgetItem(customer.value("id").toString()));
        customerTable->setItem(row, 1, new QTableWidgetItem(customer.value("customer_name").toString()));
        customerTable->setItem(row, 2, new QTableWidgetItem(customer.value("customer_address").toString()));
        customerTable->setItem(row, 3, new QTableWidgetItem(customer.value("contact_person").toString()));
        customerTable->setItem(row, 4, new QTableWidgetItem(customer.value("phone").toString()));
        customerTable->setItem(row, 5, new QTableWidgetItem(customer.value("total_orders").toString()));
    }
}

void RecordWidget::clearOrderTable()
{
    orderTable->setRowCount(0);
}

void RecordWidget::clearCustomerTable()
{
    customerTable->setRowCount(0);
}

void RecordWidget::onSearchOrderClicked()
{
    QString searchText = orderSearchEdit->text().trimmed();
    QString field = orderSearchFieldCombo->currentText();
    QString start = startDateEdit->date().toString("yyyy-MM-dd");
    QString end = endDateEdit->date().toString("yyyy-MM-dd");

    QList<QVariantMap> results;
    if (RecordDatabaseManager::instance().searchOrders(field, searchText, start, end, results)) {
        clearOrderTable();
        for (const auto& order : results) {
            int row = orderTable->rowCount();
            orderTable->insertRow(row);
            orderTable->setItem(row, 0, new QTableWidgetItem(order.value("id").toString()));
            orderTable->setItem(row, 1, new QTableWidgetItem(order.value("order_no").toString()));
            orderTable->setItem(row, 2, new QTableWidgetItem(order.value("customer_name").toString()));
            orderTable->setItem(row, 3, new QTableWidgetItem(order.value("contact_person").toString()));
            orderTable->setItem(row, 4, new QTableWidgetItem(order.value("phone").toString()));
            orderTable->setItem(row, 5, new QTableWidgetItem(order.value("total_amount").toString()));
            orderTable->setItem(row, 6, new QTableWidgetItem(order.value("status").toString()));
            orderTable->setItem(row, 7, new QTableWidgetItem(order.value("order_date").toString()));
        }
    }
}

void RecordWidget::onResetOrderClicked()
{
    orderSearchEdit->clear();
    startDateEdit->setDate(QDate::currentDate().addDays(-30));
    endDateEdit->setDate(QDate::currentDate());
    loadOrders();
}

void RecordWidget::onSearchCustomerClicked()
{
    QString searchText = customerSearchEdit->text().trimmed();
    QString field = customerSearchFieldCombo->currentText();

    QList<QVariantMap> results;
    if (RecordDatabaseManager::instance().searchCustomers(field, searchText, results)) {
        clearCustomerTable();
        for (const auto& customer : results) {
            int row = customerTable->rowCount();
            customerTable->insertRow(row);
            customerTable->setItem(row, 0, new QTableWidgetItem(customer.value("id").toString()));
            customerTable->setItem(row, 1, new QTableWidgetItem(customer.value("customer_name").toString()));
            customerTable->setItem(row, 2, new QTableWidgetItem(customer.value("customer_address").toString()));
            customerTable->setItem(row, 3, new QTableWidgetItem(customer.value("contact_person").toString()));
            customerTable->setItem(row, 4, new QTableWidgetItem(customer.value("phone").toString()));
            customerTable->setItem(row, 5, new QTableWidgetItem(customer.value("total_orders").toString()));
        }
    }
}

void RecordWidget::onResetCustomerClicked()
{
    customerSearchEdit->clear();
    loadCustomers();
}

void RecordWidget::onOrderSelected()
{
    QModelIndexList selected = orderTable->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        currentOrderId = -1;
        viewDetailsBtn->setEnabled(false);
        completeOrderBtn->setEnabled(false);
        deleteOrderBtn->setEnabled(false);
        return;
    }

    currentOrderId = selected.first().data().toInt();
    viewDetailsBtn->setEnabled(true);
    completeOrderBtn->setEnabled(true);
    deleteOrderBtn->setEnabled(true);
}

void RecordWidget::onCustomerSelected()
{
    QModelIndexList selected = customerTable->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        currentCustomerId = -1;
        editCustomerBtn->setEnabled(false);
        deleteCustomerBtn->setEnabled(false);
        return;
    }

    currentCustomerId = selected.first().data().toInt();
    editCustomerBtn->setEnabled(true);
    deleteCustomerBtn->setEnabled(true);
}

void RecordWidget::onViewOrderDetails()
{
    if (currentOrderId == -1) return;

    QVariantMap order;
    if (RecordDatabaseManager::instance().getOrderById(currentOrderId, order)) {
        QString details = QString("订单号：%1\n客户：%2\n联系人：%3\n电话：%4\n金额：%5\n状态：%6\n日期：%7")
                              .arg(order.value("order_no").toString())
                              .arg(order.value("customer_name").toString())
                              .arg(order.value("contact_person").toString())
                              .arg(order.value("phone").toString())
                              .arg(order.value("total_amount").toString())
                              .arg(order.value("status").toString())
                              .arg(order.value("order_date").toString());
        QMessageBox::information(this, "订单详情", details);
    }
}

void RecordWidget::onDeleteOrder()
{
    if (currentOrderId == -1) return;

    if (QMessageBox::question(this, "确认删除", "确定要删除此订单吗？") == QMessageBox::Yes) {
        if (RecordDatabaseManager::instance().deleteOrder(currentOrderId)) {
            loadOrders();
            currentOrderId = -1;
            viewDetailsBtn->setEnabled(false);
            completeOrderBtn->setEnabled(false);
            deleteOrderBtn->setEnabled(false);
        } else {
            QMessageBox::critical(this, "错误", "删除失败！");
        }
    }
}

// 新增完成订单槽函数
void RecordWidget::onCompleteOrder()
{
    if (currentOrderId == -1) return;

    // 获取当前订单信息
    QVariantMap order;
    if (!RecordDatabaseManager::instance().getOrderById(currentOrderId, order)) {
        QMessageBox::critical(this, "错误", "获取订单信息失败！");
        return;
    }

    // 检查订单状态是否已经是完成状态
    if (order.value("status").toString() == "完成") {
        QMessageBox::information(this, "提示", "该订单已经是完成状态！");
        return;
    }

    // 确认完成订单
    if (QMessageBox::question(this, "确认完成", "确定要将此订单标记为完成状态吗？") == QMessageBox::Yes) {
        if (RecordDatabaseManager::instance().completeOrder(currentOrderId)) {
            loadOrders();
            currentOrderId = -1;
            viewDetailsBtn->setEnabled(false);
            completeOrderBtn->setEnabled(false);
            deleteOrderBtn->setEnabled(false);
            QMessageBox::information(this, "成功", "订单已完成！");
        } else {
            QMessageBox::critical(this, "错误", "更新订单状态失败！");
        }
    }
}

void RecordWidget::onAddCustomer()
{
    CustomerDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QVariantMap data = dialog.getCustomerData();
        if (data.value("customer_name").toString().isEmpty()) {
            QMessageBox::warning(this, "输入错误", "客户名称不能为空！");
            return;
        }

        if (RecordDatabaseManager::instance().insertCustomer(data)) {
            loadCustomers();
            QMessageBox::information(this, "成功", "客户添加成功！");
        } else {
            QMessageBox::critical(this, "错误", "添加失败！");
        }
    }
}

void RecordWidget::onDeleteCustomer()
{
    if (currentCustomerId == -1) return;

    if (QMessageBox::question(this, "确认删除", "确定要删除此客户吗？") == QMessageBox::Yes) {
        if (RecordDatabaseManager::instance().deleteCustomer(currentCustomerId)) {
            loadCustomers();
            currentCustomerId = -1;
            editCustomerBtn->setEnabled(false);
            deleteCustomerBtn->setEnabled(false);
        } else {
            QMessageBox::critical(this, "错误", "删除失败！");
        }
    }
}

void RecordWidget::onEditCustomer()
{
    if (currentCustomerId == -1) return;

    QVariantMap customer;
    if (!RecordDatabaseManager::instance().getCustomerById(currentCustomerId, customer)) {
        QMessageBox::critical(this, "错误", "获取客户信息失败！");
        return;
    }

    CustomerDialog dialog(this);
    dialog.setCustomerData(customer);

    if (dialog.exec() == QDialog::Accepted) {
        QVariantMap data = dialog.getCustomerData();
        if (data.value("customer_name").toString().isEmpty()) {
            QMessageBox::warning(this, "输入错误", "客户名称不能为空！");
            return;
        }

        if (RecordDatabaseManager::instance().updateCustomer(currentCustomerId, data)) {
            loadCustomers();
            QMessageBox::information(this, "成功", "客户信息更新成功！");
        } else {
            QMessageBox::critical(this, "错误", "更新失败！");
        }
    }
}

void RecordWidget::onOrderTableHeaderClicked(int logicalIndex)
{
    QStringList headers = {"ID", "订单号", "客户名称", "联系人", "联系电话", "总金额", "订单状态", "下单时间"};
    QString field = headers.at(logicalIndex);

    if (currentOrderSortField == field) {
        currentOrderSortOrder = (currentOrderSortOrder == "升序") ? "降序" : "升序";
    } else {
        currentOrderSortField = field;
        currentOrderSortOrder = "降序";
    }

    loadOrders();
}

void RecordWidget::onCustomerTableHeaderClicked(int logicalIndex)
{
    QStringList headers = {"ID", "客户名称", "客户地址", "联系人", "联系电话", "总订单数"};
    QString field = headers.at(logicalIndex);

    if (currentCustomerSortField == field) {
        currentCustomerSortOrder = (currentCustomerSortOrder == "升序") ? "降序" : "升序";
    } else {
        currentCustomerSortField = field;
        currentCustomerSortOrder = "降序";
    }

    loadCustomers();
}
