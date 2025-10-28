#include "loginwindow.h"
#include "registerwindow.h"
#include "database.h"
#include "mainwindow.h"
#include "currentusermanager.h"
#include <QApplication>
#include <QPixmap>
#include <QIcon>
#include <QPalette>
#include <QBrush>
#include <QDesktopWidget>
#include <QScreen>
#include <QGuiApplication>
#include <QStyle>

LoginWindow::LoginWindow(QWidget *parent)
    : QMainWindow(parent)
    , passwordVisible(false)
{
    setupUI();
    loadSavedCredentials();
    adjustUI();

    // 设置窗口属性
    setWindowTitle("开单及商品管理系统");
    setMinimumSize(800, 600);

    // 居中显示
    QRect screenGeometry = QGuiApplication::primaryScreen()->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
}

LoginWindow::~LoginWindow()
{
}

void LoginWindow::setupUI()
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // 标题
    titleLabel = new QLabel("开单及商品管理系统", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(64);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);

    // 用户名输入框
    usernameEdit = new QLineEdit(this);
    usernameEdit->setPlaceholderText("请输入账号");

    // 密码输入框
    passwordEdit = new QLineEdit(this);
    passwordEdit->setPlaceholderText("请输入密码");
    passwordEdit->setEchoMode(QLineEdit::Password);

    // 显示/隐藏密码按钮
    showPasswordButton = new QPushButton("显示", this);
    showPasswordButton->setFixedWidth(160);
    showPasswordButton->setStyleSheet("QPushButton { font-size: 24px; }");
    connect(showPasswordButton, &QPushButton::clicked, this, &LoginWindow::onShowPasswordClicked);

    // 角色选择下拉框
    roleComboBox = new QComboBox(this);
    roleComboBox->addItem("用户", "user");
    roleComboBox->addItem("管理员", "admin");
    roleComboBox->addItem("超级管理员", "super_admin");

    // 记住密码复选框
    rememberPasswordCheckBox = new QCheckBox("记住密码", this);
    connect(rememberPasswordCheckBox, &QCheckBox::clicked, this, &LoginWindow::onRememberPasswordClicked);

    // 按钮
    loginButton = new QPushButton("登录", this);
    registerButton = new QPushButton("注册", this);
    exitButton = new QPushButton("退出", this);

    // 增大按钮字体
    QFont buttonFont;
    buttonFont.setPointSize(64);
    loginButton->setFont(buttonFont);
    registerButton->setFont(buttonFont);
    exitButton->setFont(buttonFont);

    connect(loginButton, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
    connect(registerButton, &QPushButton::clicked, this, &LoginWindow::onRegisterClicked);
    connect(exitButton, &QPushButton::clicked, this, &LoginWindow::onExitClicked);

    // 布局 - 密码和显示按钮同行
    QFormLayout *formLayout = new QFormLayout();
    formLayout->addRow("账号:", usernameEdit);

    // 密码行布局
    QHBoxLayout *passwordLayout = new QHBoxLayout();
    passwordLayout->addWidget(passwordEdit);
    passwordLayout->addWidget(showPasswordButton);

    formLayout->addRow("密码:", passwordLayout);
    formLayout->addRow("身份:", roleComboBox);
    formLayout->addRow("", rememberPasswordCheckBox);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(loginButton);
    buttonLayout->addWidget(registerButton);
    buttonLayout->addWidget(exitButton);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->addWidget(titleLabel);
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(buttonLayout);

    // 设置样式
    QString styleSheet = R"(
        QMainWindow {
            background-color: #f0f0f0;
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
            background-color: #4CAF50;
            color: white;
            font-weight: bold;
            font-size: 24px;
        }
        QPushButton:hover {
            background-color: #45a049;
        }
        QComboBox {
            padding: 10px;
            border: 1px solid #cccccc;
            border-radius: 4px;
            background-color: white;
            font-size: 24px;
        }
        QCheckBox {
            color: #333333;
            font-size: 24px;
        }
    )";

    setStyleSheet(styleSheet);
}

void LoginWindow::loadSavedCredentials()
{
    QSettings settings("ProductManagement", "Login");
    QString savedUsername = settings.value("username", "").toString();
    QString savedPassword = settings.value("password", "").toString();
    bool remember = settings.value("remember", false).toBool();

    if (remember) {
        usernameEdit->setText(savedUsername);
        passwordEdit->setText(savedPassword);
        rememberPasswordCheckBox->setChecked(true);
    }
}

void LoginWindow::saveCredentials()
{
    if (rememberPasswordCheckBox->isChecked()) {
        QSettings settings("ProductManagement", "Login");
        settings.setValue("username", usernameEdit->text());
        settings.setValue("password", passwordEdit->text());
        settings.setValue("remember", true);
    }
}

void LoginWindow::clearCredentials()
{
    QSettings settings("ProductManagement", "Login");
    settings.remove("");
}

void LoginWindow::adjustUI()
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

    // 调整按钮大小
    loginButton->setMinimumHeight(windowHeight / 10);
    registerButton->setMinimumHeight(windowHeight / 10);
    exitButton->setMinimumHeight(windowHeight / 10);

    // 调整显示密码按钮大小
    showPasswordButton->setMinimumHeight(windowHeight / 12);
}

void LoginWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    adjustUI();
}

void LoginWindow::onLoginClicked()
{
    QString username = usernameEdit->text().trimmed();
    QString password = passwordEdit->text();
    QString role = roleComboBox->currentData().toString();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "登录失败", "请输入账号和密码！");
        return;
    }

    // 连接数据库
    DatabaseManager& dbManager = DatabaseManager::instance();
    if (!dbManager.connect()) {
        QMessageBox::critical(this, "数据库错误", "无法连接到数据库！");
        return;
    }

    // 验证用户，添加一个动态记录标识
    QString userRole;
    if (dbManager.authenticateUser(username, password, userRole)) {
        if (userRole == role) {
            // 登录成功
            saveCredentials();

            //设置当前登录用户
            CurrentUserManager::instance().setCurrentUser(username, userRole);

            // 跳转到主界面
            MainWindow *mainWindow = new MainWindow(username, userRole);
            mainWindow->show();
            this->close(); // 关闭登录窗口

        } else {
            QMessageBox::warning(this, "登录失败", "身份选择错误，请重新选择！");
        }
    } else {
        QMessageBox::warning(this, "登录失败", "账号或密码错误，请重新输入！");
    }
}

void LoginWindow::onRegisterClicked()
{
    hide();
    RegisterWindow *registerWindow = new RegisterWindow(this);
    registerWindow->show();
}

void LoginWindow::onExitClicked()
{
    QApplication::quit();
}

void LoginWindow::onShowPasswordClicked()
{
    if (passwordVisible) {
        passwordEdit->setEchoMode(QLineEdit::Password);
        showPasswordButton->setText("显示");
        passwordVisible = false;
    } else {
        passwordEdit->setEchoMode(QLineEdit::Normal);
        showPasswordButton->setText("隐藏");
        passwordVisible = true;
    }
}

void LoginWindow::onRememberPasswordClicked()
{
    if (!rememberPasswordCheckBox->isChecked()) {
        clearCredentials();
    }
}
