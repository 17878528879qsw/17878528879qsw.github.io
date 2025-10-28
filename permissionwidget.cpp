// permissionwidget.cpp
#include "permissionwidget.h"
#include "permissiondatabase.h"
#include "currentusermanager.h"
#include <QApplication>
#include <QFont>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QScrollBar>
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
#include <QCheckBox>

PermissionWidget::PermissionWidget(QWidget *parent)
    : QWidget(parent)
    , currentUserId(-1)
{
    setupUI();
    loadUsers();
}

void PermissionWidget::setupUI()
{
    setStyleSheet(R"(
        QWidget {
            font-family: "Microsoft YaHei";
            font-size: 24px;
        }
        QGroupBox {
            font-weight: bold;
            border: 2px solid #CCCCCC;
            border-radius: 10px;
            margin-top: 2ex;
            padding-top: 15px;
        }
        QGroupBox::title {
            subline-offset: -2px;
            padding: 0px 8px;
            font-size: 26px;
        }
        QLineEdit, QComboBox {
            padding: 20px;
            border: 2px solid #CCCCCC;
            border-radius: 6px;
            background-color: white;
            font-size: 24px;
        }
        QPushButton {
            padding: 20px 40px;
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
            font-size: 22px;
        }
        QHeaderView::section {
            background-color: #3498DB;
            color: white;
            padding: 16px;
            font-weight: bold;
            border: none;
            font-size: 24px;
        }
    )");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(50, 50, 50, 50);
    mainLayout->setSpacing(40);

    // 搜索栏
    setupSearchBar();
    mainLayout->addLayout(searchLayout);

    // 用户表格
    setupUserTable();
    mainLayout->addWidget(userTable);

    // 按钮栏
    setupButtons();
    mainLayout->addLayout(buttonLayout);

    // 初始化按钮状态
    modifyBtn->setEnabled(false);
    deleteBtn->setEnabled(false);
}

void PermissionWidget::setupUserTable()
{
    userTable = new QTableWidget(this);
    userTable->setColumnCount(5);
    QStringList headers = {"ID", "用户名", "昵称", "角色", "创建时间"};
    userTable->setHorizontalHeaderLabels(headers);
    userTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    userTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    userTable->verticalHeader()->setVisible(false);
    userTable->horizontalHeader()->setStretchLastSection(true);
    userTable->setAlternatingRowColors(true);
    userTable->setStyleSheet("QTableWidget { border: 2px solid #ccc; }");

    // 设置列宽
    userTable->setColumnWidth(0, 150);   // ID
    userTable->setColumnWidth(1, 300);   // 用户名
    userTable->setColumnWidth(2, 300);   // 昵称
    userTable->setColumnWidth(3, 250);   // 角色
    userTable->setColumnWidth(4, 400);   // 创建时间

    // 启用排序
    userTable->setSortingEnabled(true);

    connect(userTable, &QTableWidget::itemSelectionChanged, this, &PermissionWidget::onUserSelected);

    // 连接表头点击信号
    connect(userTable->horizontalHeader(), &QHeaderView::sectionClicked, this, &PermissionWidget::onSortChanged);
}

void PermissionWidget::setupButtons()
{
    buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(30);

    addBtn = new QPushButton("添加", this);
    modifyBtn = new QPushButton("修改", this);
    deleteBtn = new QPushButton("删除", this);

    connect(addBtn, &QPushButton::clicked, this, &PermissionWidget::onAddUserClicked);
    connect(modifyBtn, &QPushButton::clicked, this, &PermissionWidget::onModifyUserClicked);
    connect(deleteBtn, &QPushButton::clicked, this, &PermissionWidget::onDeleteUserClicked);

    buttonLayout->addStretch();
    buttonLayout->addWidget(addBtn);
    buttonLayout->addWidget(modifyBtn);
    buttonLayout->addWidget(deleteBtn);
}

void PermissionWidget::setupSearchBar()
{
    searchLayout = new QHBoxLayout();
    searchLayout->setSpacing(20);

    QLabel *searchLabel = new QLabel("搜索:", this);
    searchEdit = new QLineEdit(this);
    searchFieldComboBox = new QComboBox(this);
    searchBtn = new QPushButton("搜索", this);
    resetSearchBtn = new QPushButton("重置", this);

    searchFieldComboBox->addItem("用户名", "username");
    searchFieldComboBox->addItem("昵称", "use_name");
    searchFieldComboBox->addItem("角色", "role");

    searchLayout->addWidget(searchLabel);
    searchLayout->addWidget(searchFieldComboBox);
    searchLayout->addWidget(searchEdit);
    searchLayout->addWidget(searchBtn);
    searchLayout->addWidget(resetSearchBtn);

    connect(searchBtn, &QPushButton::clicked, this, &PermissionWidget::onSearchClicked);
    connect(resetSearchBtn, &QPushButton::clicked, this, &PermissionWidget::onResetSearchClicked);
}

void PermissionWidget::loadUsers()
{
    userTable->setRowCount(0);

    PermissionDatabaseManager& db = PermissionDatabaseManager::instance();
    if (!db.connect()) {
        QMessageBox::critical(this, "错误", "无法连接数据库！");
        return;
    }

    QList<QVariantMap> users;
    if (db.getAllUsers(users)) {
        for (const auto& user : users) {
            int row = userTable->rowCount();
            userTable->insertRow(row);

            userTable->setItem(row, 0, new QTableWidgetItem(QString::number(user["id"].toInt())));
            userTable->setItem(row, 1, new QTableWidgetItem(user["username"].toString()));
            userTable->setItem(row, 2, new QTableWidgetItem(user["nickname"].toString()));
            userTable->setItem(row, 3, new QTableWidgetItem(user["role"].toString()));
            userTable->setItem(row, 4, new QTableWidgetItem(user["created_at"].toString()));
        }
    }
}

bool PermissionWidget::checkPermission(const QString& operation) {
    // 获取当前登录用户的角色
    QString currentUserRole = CurrentUserManager::instance().getCurrentRole();

    // 只有超级管理员才能执行操作
    return currentUserRole == "super_admin";
}

void PermissionWidget::onAddUserClicked()
{
    if (!checkPermission("add")) {
        QMessageBox::warning(this, "权限不足", "只有超级管理员才能添加用户！");
        return;
    }

    // 创建一个对话框来添加用户
    QDialog dialog(this);
    dialog.setWindowTitle("添加用户");
    dialog.resize(500, 400);

    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);
    mainLayout->setSpacing(30);

    QFormLayout *formLayout = new QFormLayout();
    formLayout->setSpacing(30);
    formLayout->setLabelAlignment(Qt::AlignRight);

    QLineEdit *usernameEdit = new QLineEdit(&dialog);
    usernameEdit->setStyleSheet("QLineEdit { padding: 15px; font-size: 20px; }");
    QLineEdit *nicknameEdit = new QLineEdit(&dialog);
    nicknameEdit->setStyleSheet("QLineEdit { padding: 15px; font-size: 20px; }");
    QComboBox *roleComboBox = new QComboBox(&dialog);
    roleComboBox->setStyleSheet("QComboBox { padding: 15px; font-size: 20px; }");
    roleComboBox->addItem("用户", "user");
    roleComboBox->addItem("管理员", "admin");
    roleComboBox->addItem("超级管理员", "super_admin");
    QLineEdit *passwordEdit = new QLineEdit(&dialog);
    passwordEdit->setStyleSheet("QLineEdit { padding: 15px; font-size: 20px; }");
    passwordEdit->setEchoMode(QLineEdit::Password);
    QLineEdit *confirmPasswordEdit = new QLineEdit(&dialog);
    confirmPasswordEdit->setStyleSheet("QLineEdit { padding: 15px; font-size: 20px; }");
    confirmPasswordEdit->setEchoMode(QLineEdit::Password);

    formLayout->addRow("用户名:", usernameEdit);
    formLayout->addRow("昵称:", nicknameEdit);
    formLayout->addRow("角色:", roleComboBox);
    formLayout->addRow("密码:", passwordEdit);
    formLayout->addRow("确认密码:", confirmPasswordEdit);

    // 创建按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *okBtn = new QPushButton("确定", &dialog);
    okBtn->setStyleSheet("QPushButton { padding: 15px; font-size: 20px; }");
    QPushButton *cancelBtn = new QPushButton("取消", &dialog);
    cancelBtn->setStyleSheet("QPushButton { padding: 15px; font-size: 20px; }");
    buttonLayout->addStretch();
    buttonLayout->addWidget(okBtn);
    buttonLayout->addWidget(cancelBtn);

    // 将表单布局和按钮布局添加到主布局中
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(buttonLayout);

    connect(okBtn, &QPushButton::clicked, [&]() {
        QString username = usernameEdit->text().trimmed();
        if (username.isEmpty()) {
            QMessageBox::warning(&dialog, "警告", "用户名不能为空！");
            return;
        }

        QString password = passwordEdit->text();
        QString confirmPassword = confirmPasswordEdit->text();
        if (password != confirmPassword) {
            QMessageBox::warning(&dialog, "警告", "两次输入的密码不一致！");
            return;
        }

        PermissionDatabaseManager& db = PermissionDatabaseManager::instance();
        if (!db.connect()) {
            QMessageBox::critical(&dialog, "错误", "无法连接数据库！");
            return;
        }

        bool success = db.insertUser(username, nicknameEdit->text(), roleComboBox->currentData().toString(), password);
        if (success) {
            QMessageBox::information(&dialog, "提示", "用户添加成功！");
            loadUsers();
            dialog.accept();
        } else {
            QMessageBox::critical(&dialog, "失败", "添加用户失败！");
        }
    });

    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);

    dialog.exec();
}

void PermissionWidget::onModifyUserClicked()
{
    if (!checkPermission("modify")) {
        QMessageBox::warning(this, "权限不足", "只有超级管理员才能修改用户！");
        return;
    }

    if (currentUserId <= 0) {
        QMessageBox::warning(this, "警告", "请选择要修改的用户！");
        return;
    }

    // 创建一个对话框来修改用户
    QDialog dialog(this);
    dialog.setWindowTitle("修改用户");
    dialog.resize(500, 400);

    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);
    mainLayout->setSpacing(30);

    QFormLayout *formLayout = new QFormLayout();
    formLayout->setSpacing(30);
    formLayout->setLabelAlignment(Qt::AlignRight);

    QLineEdit *usernameEdit = new QLineEdit(&dialog);
    usernameEdit->setStyleSheet("QLineEdit { padding: 15px; font-size: 20px; }");
    QLineEdit *nicknameEdit = new QLineEdit(&dialog);
    nicknameEdit->setStyleSheet("QLineEdit { padding: 15px; font-size: 20px; }");
    QComboBox *roleComboBox = new QComboBox(&dialog);
    roleComboBox->setStyleSheet("QComboBox { padding: 15px; font-size: 20px; }");
    roleComboBox->addItem("用户", "user");
    roleComboBox->addItem("管理员", "admin");
    roleComboBox->addItem("超级管理员", "super_admin");
    QLineEdit *passwordEdit = new QLineEdit(&dialog);
    passwordEdit->setStyleSheet("QLineEdit { padding: 15px; font-size: 20px; }");
    passwordEdit->setEchoMode(QLineEdit::Password);
    QLineEdit *confirmPasswordEdit = new QLineEdit(&dialog);
    confirmPasswordEdit->setStyleSheet("QLineEdit { padding: 15px; font-size: 20px; }");
    confirmPasswordEdit->setEchoMode(QLineEdit::Password);

    // 获取当前用户信息
    QString username, nickname, role, password;
    PermissionDatabaseManager& db = PermissionDatabaseManager::instance();
    if (db.getUserById(currentUserId, username, nickname, role, password)) {
        usernameEdit->setText(username);
        nicknameEdit->setText(nickname);
        roleComboBox->setCurrentIndex(roleComboBox->findData(role));
        passwordEdit->setText(password);
        confirmPasswordEdit->setText(password);
    }

    formLayout->addRow("用户名:", usernameEdit);
    formLayout->addRow("昵称:", nicknameEdit);
    formLayout->addRow("角色:", roleComboBox);
    formLayout->addRow("密码:", passwordEdit);
    formLayout->addRow("确认密码:", confirmPasswordEdit);

    // 创建按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *okBtn = new QPushButton("确定", &dialog);
    okBtn->setStyleSheet("QPushButton { padding: 15px; font-size: 20px; }");
    QPushButton *cancelBtn = new QPushButton("取消", &dialog);
    cancelBtn->setStyleSheet("QPushButton { padding: 15px; font-size: 20px; }");
    buttonLayout->addStretch();
    buttonLayout->addWidget(okBtn);
    buttonLayout->addWidget(cancelBtn);

    // 将表单布局和按钮布局添加到主布局中
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(buttonLayout);

    connect(okBtn, &QPushButton::clicked, [&]() {
        QString username = usernameEdit->text().trimmed();
        if (username.isEmpty()) {
            QMessageBox::warning(&dialog, "警告", "用户名不能为空！");
            return;
        }

        PermissionDatabaseManager& db = PermissionDatabaseManager::instance();
        if (!db.connect()) {
            QMessageBox::critical(&dialog, "错误", "无法连接数据库！");
            return;
        }

        QString password = passwordEdit->text();
        QString confirmPassword = confirmPasswordEdit->text();
        if (password != confirmPassword && !password.isEmpty()) {
            QMessageBox::warning(&dialog, "警告", "两次输入的密码不一致！");
            return;
        }

        bool success = db.updateUser(currentUserId, username, nicknameEdit->text(), roleComboBox->currentData().toString(),
            passwordEdit->text().isEmpty() ? QString() : passwordEdit->text());

        if (success) {
            QMessageBox::information(&dialog, "提示", "用户修改成功！");
            loadUsers();
            dialog.accept();
        } else {
            QMessageBox::critical(&dialog, "失败", "修改用户失败！");
        }
    });

    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);

    dialog.exec();
}

void PermissionWidget::onDeleteUserClicked()
{
    if (!checkPermission("delete")) {
        QMessageBox::warning(this, "权限不足", "只有超级管理员才能删除用户！");
        return;
    }

    if (currentUserId <= 0) {
        QMessageBox::warning(this, "警告", "请选择要删除的用户！");
        return;
    }

    if (currentUserId == 1) {
        QMessageBox::warning(this, "警告", "不能删除超级管理员用户！");
        return;
    }

    QString username;
    for (int i = 0; i < userTable->rowCount(); ++i) {
        if (userTable->item(i, 0)->text().toInt() == currentUserId) {
            username = userTable->item(i, 1)->text();
            break;
        }
    }

    QString message = QString("确定要删除用户 \"%1\" 吗？").arg(username);
    if (QMessageBox::question(this, "确认删除", message, QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
        return;
    }

    PermissionDatabaseManager& db = PermissionDatabaseManager::instance();
    if (!db.connect()) {
        QMessageBox::critical(this, "错误", "无法连接数据库！");
        return;
    }

    bool success = db.deleteUser(currentUserId);
    if (success) {
        QMessageBox::information(this, "提示", "用户删除成功！");
        loadUsers();
    } else {
        QMessageBox::critical(this, "失败", "删除用户失败！");
    }
}

void PermissionWidget::onUserSelected()
{
    QList<QTableWidgetItem*> items = userTable->selectedItems();
    if (items.isEmpty()) {
        modifyBtn->setEnabled(false);
        deleteBtn->setEnabled(false);
        return;
    }

    int row = items.first()->row();
    currentUserId = userTable->item(row, 0)->text().toInt();

    modifyBtn->setEnabled(true);
    deleteBtn->setEnabled(currentUserId != 1); // 禁止删除超级管理员
}

void PermissionWidget::onSearchClicked()
{
    QString field = searchFieldComboBox->currentData().toString();
    QString value = searchEdit->text().trimmed();

    if (value.isEmpty()) {
        loadUsers(); // 如果搜索内容为空，显示所有用户
        return;
    }

    PermissionDatabaseManager& db = PermissionDatabaseManager::instance();
    if (!db.connect()) {
        QMessageBox::critical(this, "错误", "无法连接数据库！");
        return;
    }

    userTable->setRowCount(0);

    QList<QVariantMap> results;
    if (db.searchUsers(field, value, results)) {
        for (const auto& user : results) {
            int row = userTable->rowCount();
            userTable->insertRow(row);

            userTable->setItem(row, 0, new QTableWidgetItem(QString::number(user["id"].toInt())));
            userTable->setItem(row, 1, new QTableWidgetItem(user["username"].toString()));
            userTable->setItem(row, 2, new QTableWidgetItem(user["nickname"].toString()));
            userTable->setItem(row, 3, new QTableWidgetItem(user["role"].toString()));
            userTable->setItem(row, 4, new QTableWidgetItem(user["created_at"].toString()));
        }
    }
}

void PermissionWidget::onResetSearchClicked()
{
    searchEdit->clear();
    loadUsers(); // 重置搜索，显示所有用户
}

void PermissionWidget::onSortChanged(int column)
{
    //自动排列，点击表的栏目即可
}
