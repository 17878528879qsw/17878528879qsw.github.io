#include "registerwindow.h"
#include "loginwindow.h"
#include "database.h"
#include <QApplication>
#include <QRegExp>
#include <QRegExpValidator>

RegisterWindow::RegisterWindow(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
    adjustUI();

    setWindowTitle("用户注册");
    setMinimumSize(800, 600);  // 与登录界面相同大小
    setModal(true);
}

void RegisterWindow::setupUI()
{
    // 标题
    titleLabel = new QLabel("用户注册", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(36);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);

    // 用户名输入框
    usernameEdit = new QLineEdit(this);
    usernameEdit->setPlaceholderText("请输入账号（3-20位字母或数字）");
    QRegExp usernameRegex("^[a-zA-Z0-9]{3,20}$");
    usernameEdit->setValidator(new QRegExpValidator(usernameRegex, this));

    // 密码输入框
    passwordEdit = new QLineEdit(this);
    passwordEdit->setPlaceholderText("请输入密码（6-20位字母或数字）");
    passwordEdit->setEchoMode(QLineEdit::Password);
    QRegExp passwordRegex("^[a-zA-Z0-9]{6,20}$");
    passwordEdit->setValidator(new QRegExpValidator(passwordRegex, this));

    // 角色选择下拉框
    roleComboBox = new QComboBox(this);
    roleComboBox->addItem("用户", "user");
    roleComboBox->addItem("管理员", "admin");
    connect(roleComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &RegisterWindow::onRoleChanged);

    // 管理员密钥输入框（默认隐藏）
    adminKeyEdit = new QLineEdit(this);
    adminKeyEdit->setPlaceholderText("请输入管理员密钥");
    adminKeyEdit->setEchoMode(QLineEdit::Password);

    // 创建对齐的密钥行
    adminKeyWidget = new QWidget(this);
    QFormLayout *adminKeyLayout = new QFormLayout(adminKeyWidget);
    adminKeyLayout->setContentsMargins(0, 0, 0, 0);
    adminKeyLayout->addRow("密钥:", adminKeyEdit);
    adminKeyWidget->setVisible(false);

    // 按钮
    registerButton = new QPushButton("注册", this);
    exitButton = new QPushButton("返回", this);

    // 增大按钮字体
    QFont buttonFont;
    buttonFont.setPointSize(14);
    registerButton->setFont(buttonFont);
    exitButton->setFont(buttonFont);

    connect(registerButton, &QPushButton::clicked, this, &RegisterWindow::onRegisterClicked);
    connect(exitButton, &QPushButton::clicked, this, &RegisterWindow::onExitClicked);

    // 布局
    QFormLayout *formLayout = new QFormLayout();
    formLayout->addRow("账号:", usernameEdit);
    formLayout->addRow("密码:", passwordEdit);
    formLayout->addRow("角色:", roleComboBox);
    formLayout->addRow(adminKeyWidget);  // 使用对齐的密钥行

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(registerButton);
    buttonLayout->addWidget(exitButton);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(titleLabel);
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(buttonLayout);

    // 设置样式
    QString styleSheet = R"(
        QDialog {
            background-color: #f5f5f5;
        }
        QLabel {
            color: #333333;
            font-weight: bold;
            font-size: 36px;
        }
        QLineEdit {
            padding: 12px;
            border: 1px solid #cccccc;
            border-radius: 4px;
            background-color: white;
            font-size: 20px;
        }
        QPushButton {
            padding: 12px 20px;
            border: 1px solid #cccccc;
            border-radius: 4px;
            background-color: #2196F3;
            color: white;
            font-weight: bold;
            font-size: 24px;
        }
        QPushButton:hover {
            background-color: #1976D2;
        }
        QComboBox {
            padding: 10px;
            border: 1px solid #cccccc;
            border-radius: 4px;
            background-color: white;
            font-size: 24px;
        }
    )";

    setStyleSheet(styleSheet);
}

void RegisterWindow::adjustUI()
{
    // 根据窗口大小调整组件大小
    int windowWidth = width();
    int windowHeight = height();

    // 调整字体大小
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(qMax(18, windowWidth / 30));
    titleLabel->setFont(titleFont);

    // 调整输入框大小
    usernameEdit->setMinimumHeight(windowHeight / 12);
    passwordEdit->setMinimumHeight(windowHeight / 12);
    adminKeyEdit->setMinimumHeight(windowHeight / 12);

    // 调整按钮大小
    registerButton->setMinimumHeight(windowHeight / 10);
    exitButton->setMinimumHeight(windowHeight / 10);
}

void RegisterWindow::onRegisterClicked()
{
    QString username = usernameEdit->text().trimmed();
    QString password = passwordEdit->text();
    QString role = roleComboBox->currentData().toString();

    // 输入验证
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "注册失败", "账号和密码不能为空！");
        return;
    }

    if (username.length() < 3 || username.length() > 20) {
        QMessageBox::warning(this, "注册失败", "账号长度必须在3-20位之间！");
        return;
    }

    if (password.length() < 6 || password.length() > 20) {
        QMessageBox::warning(this, "注册失败", "密码长度必须在6-20位之间！");
        return;
    }

    // 管理员注册需要验证密钥
    if (role == "admin") {
        QString adminKey = adminKeyEdit->text();
        if (adminKey != "JinLianFuShi123") {
            QMessageBox::warning(this, "注册失败", "管理员密钥错误！");
            return;
        }
    }

    // 连接数据库
    DatabaseManager& dbManager = DatabaseManager::instance();
    if (!dbManager.connect()) {
        QMessageBox::critical(this, "数据库错误", "无法连接到数据库！");
        return;
    }

    // 检查用户名是否已存在
    if (dbManager.userExists(username)) {
        QMessageBox::warning(this, "注册失败", "该账号已存在，请选择其他账号！");
        return;
    }

    // 创建用户
    if (dbManager.createUser(username, password, role)) {
        QMessageBox::information(this, "注册成功", "用户注册成功！");
        onExitClicked(); // 返回登录界面
    } else {
        QMessageBox::critical(this, "注册失败", "注册过程中发生错误，请稍后重试！");
    }
}

void RegisterWindow::onExitClicked()
{
    // 返回登录界面
    LoginWindow *loginWindow = qobject_cast<LoginWindow*>(parent());
    if (loginWindow) {
        loginWindow->show();
    }
    close();
}

void RegisterWindow::onRoleChanged(int index)
{
    QString role = roleComboBox->itemData(index).toString();
    // 如果选择管理员，则显示密钥输入框
    adminKeyWidget->setVisible(role == "admin");
    adjustSize();
}
