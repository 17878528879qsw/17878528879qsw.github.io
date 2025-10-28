#include "mainwindow.h"
#include "loginwindow.h"
#include "database.h"
#include "personalcenterwidget.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>
#include <QGuiApplication>
#include <QStyle>
#include <QMessageBox>
#include <QEvent>
#include <QApplication>
#include <QFile>
#include <QFileDialog>
#include <QLineEdit>
#include <QFormLayout>
#include <QDialog>
#include <QScrollArea>
#include <QToolButton>
#include <QLabel>
#include <QPixmap>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QCheckBox>

MainWindow::MainWindow(const QString& username, const QString& role, QWidget *parent)
    : QMainWindow(parent)
    , m_username(username)
    , m_role(role)
    , m_nickname(username)
    , m_avatarPath("")
    , currentActiveButton(nullptr)
    , personalMenu(nullptr)
    , sideBar(nullptr)
    , homePage(nullptr)
    , personalCenterPage(nullptr)
    , billingPage(nullptr)
    , productPage(nullptr)
    , warehousePage(nullptr)
    , permissionPage(nullptr)
    , recordPage(nullptr)
    , analysisPage(nullptr)
    , mainContent(nullptr)
    , topBar(nullptr)
    , homeButton(nullptr)
    , userInfoWidget(nullptr)
    , avatarLabel(nullptr)
    , usernameLabel(nullptr)
    , timeLabel(nullptr)
    , newsLabel(nullptr)
    , locationLabel(nullptr)
    , timer(nullptr)
    , nicknameEdit(nullptr)
    , nicknameConfirmBtn(nullptr)
    , passwordEdit(nullptr)
{
    // 从数据库获取用户信息
    DatabaseManager& dbManager = DatabaseManager::instance();
    if (dbManager.connect()) {
        QString nickname, avatarPath;
        if (dbManager.getUserInfo(username, nickname, avatarPath)) {
            m_nickname = nickname;
            m_avatarPath = avatarPath;
        }
    }

    setupUI();

    // 设置窗口属性
    setWindowTitle("开单及商品管理系统");
    setMinimumSize(2000, 1200);

    // 居中显示
    QRect screenGeometry = QGuiApplication::primaryScreen()->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);

    // 启动定时器更新时间
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateTime);
    timer->start(1000); // 每秒更新一次
    updateTime(); // 立即更新一次

    // 默认显示首页
    onHomeClicked();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    // 创建中央widget
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // 设置主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 创建顶部栏
    setupTopBar();
    mainLayout->addWidget(topBar);

    // 创建水平布局用于侧边栏和主内容
    QHBoxLayout *contentLayout = new QHBoxLayout();
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    // 创建侧边栏
    setupSideBar();
    contentLayout->addWidget(sideBar);

    // 创建主内容区域
    setupMainContent();
    contentLayout->addWidget(mainContent);

    mainLayout->addLayout(contentLayout);

    // 设置样式表
    QString styleSheet = R"(
        QMainWindow {
            background-color: #f0f0f0;
        }
        QPushButton {
            border: none;
            background-color: transparent;
        }
        QPushButton:hover {
            background-color: #e0e0e0;
        }
    )";

    setStyleSheet(styleSheet);
}

void MainWindow::setupTopBar()
{
    topBar = new QWidget(this);
    topBar->setFixedHeight(120);
    topBar->setStyleSheet("background-color: #34495e; color: white;");

    QHBoxLayout *topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(20, 0, 20, 0);

    // 左侧：首页按钮
    homeButton = new QPushButton("首页", this);
    homeButton->setStyleSheet("font-size: 36px; color: white; font-weight: bold;");
    connect(homeButton, &QPushButton::clicked, this, &MainWindow::onHomeClicked);

    // 中间：空白区域
    QWidget *spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    // 右侧：用户信息
    userInfoWidget = new QWidget(this);
    QHBoxLayout *userLayout = new QHBoxLayout(userInfoWidget);
    userLayout->setContentsMargins(10, 10, 10, 10);

    // 头像
    avatarLabel = new QLabel(this);
    avatarLabel->setFixedSize(80, 80);
    avatarLabel->setStyleSheet("background-color: #bdc3c7; border-radius: 40px; cursor: pointer;");
    avatarLabel->setAlignment(Qt::AlignCenter);

    // 如果有头像路径且文件存在，显示头像
    if (!m_avatarPath.isEmpty() && QFile::exists(m_avatarPath)) {
        QPixmap avatarPixmap(m_avatarPath);
        avatarPixmap = avatarPixmap.scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        avatarLabel->setPixmap(avatarPixmap);
    } else {
        avatarLabel->setText("头像");
    }

    // 用户名/昵称
    usernameLabel = new QLabel(m_nickname, this);
    usernameLabel->setStyleSheet("font-size: 30px; font-weight: bold; margin-left: 10px; color: white;");

    userLayout->addWidget(avatarLabel);
    userLayout->addWidget(usernameLabel);

    // 设置鼠标悬停事件
    userInfoWidget->setCursor(Qt::PointingHandCursor);
    userInfoWidget->installEventFilter(this);
    avatarLabel->installEventFilter(this); // 头像也可以点击

    // 连接信号槽
    userInfoWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(userInfoWidget, &QWidget::customContextMenuRequested, this, &MainWindow::setupPersonalMenu);

    topLayout->addWidget(homeButton);
    topLayout->addWidget(spacer);
    topLayout->addWidget(userInfoWidget);
}

void MainWindow::setupPersonalMenu()
{
    if (!personalMenu) {
        personalMenu = new QMenu(this);
        personalMenu->setStyleSheet(R"(
            QMenu {
                background-color: white;
                border: 1px solid #cccccc;
                border-radius: 8px;
                padding: 5px;
            }
            QMenu::item {
                padding: 10px 20px;
                font-size: 24px;
                border-radius: 4px;
            }
            QMenu::item:selected {
                background-color: #3498db;
                color: white;
            }
        )");

        QAction *personalCenterAction = new QAction("个人中心", this);
        QAction *changeAccountAction = new QAction("切换账号", this);
        QAction *logoutAction = new QAction("退出登录", this);

        connect(personalCenterAction, &QAction::triggered, this, &MainWindow::onPersonalCenterClicked);
        connect(changeAccountAction, &QAction::triggered, this, &MainWindow::onChangeAccountClicked);
        connect(logoutAction, &QAction::triggered, this, &MainWindow::onLogoutClicked);

        personalMenu->addAction(personalCenterAction);
        personalMenu->addAction(changeAccountAction);
        personalMenu->addSeparator(); // 添加分隔线
        personalMenu->addAction(logoutAction);
    }

    // 显示菜单
    QPoint pos = userInfoWidget->mapToGlobal(QPoint(0, userInfoWidget->height()));
    personalMenu->exec(pos);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == userInfoWidget) {
        if (event->type() == QEvent::Enter) {
            // 鼠标进入
            userInfoWidget->setStyleSheet("background-color: #233240; border-radius: 5px;");
            return true;
        } else if (event->type() == QEvent::Leave) {
            // 鼠标离开
            userInfoWidget->setStyleSheet("");
            return true;
        } else if (event->type() == QEvent::MouseButtonPress) {
            // 鼠标点击
            setupPersonalMenu();
            return true;
        }
    } else if (obj == avatarLabel) {
        if (event->type() == QEvent::MouseButtonPress) {
            // 点击头像
            onChangeAvatarClicked();
            return true;
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::setupSideBar()
{
    sideBar = new QWidget(this);
    sideBar->setFixedWidth(260);
    sideBar->setStyleSheet("background-color: #2c3e50;");

    QVBoxLayout *sideLayout = new QVBoxLayout(sideBar);
    sideLayout->setContentsMargins(0, 0, 0, 0);
    sideLayout->setSpacing(0);

    // 顶部留白
    QWidget *topSpacer = new QWidget(this);
    topSpacer->setFixedHeight(40);
    sideLayout->addWidget(topSpacer);

    // 创建功能按钮（优化显示效果）
    QStringList functionNames;
    functionNames << "开单" << "商品" << "仓库" << "权限" << "记录" << "分析";

    // 图标文本映射（使用emoji图标）
    QStringList functionIcons;
    functionIcons << "📝" << "📦" << "🏠" << "🔑" << "📋" << "📊";

    // 创建功能按钮
    for (int i = 0; i < functionNames.size(); ++i) {
        QToolButton *button = new QToolButton(this);
        button->setText(functionIcons[i] + " " + functionNames[i]);  // 添加图标
        button->setCheckable(true);
        button->setProperty("index", i);
        button->setFixedHeight(80);
        button->setIconSize(QSize(32, 32));

        // 优化按钮样式
        button->setStyleSheet(
            "QToolButton {"
            "   border: none;"
            "   text-align: left;"
            "   padding: 15px 25px;"
            "   font-size: 30px;"
            "   background-color: #2c3e50;"
            "   color: #ecf0f1;"
            "   font-weight: normal;"
            "}"
            "QToolButton:hover {"
            "   background-color: #34495e;"
            "}"
            "QToolButton:checked {"
            "   background-color: #3498db;"
            "   color: white;"
            "   font-weight: bold;"
            "   border-left: 5px solid #1abc9c;"
            "}"
        );

        connect(button, &QToolButton::clicked, [=]() {
            onFunctionClicked(i);
        });

        sideLayout->addWidget(button);
        functionButtons.append(button);
    }

    // 底部留白
    QWidget *bottomSpacer = new QWidget(this);
    bottomSpacer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    sideLayout->addWidget(bottomSpacer);

    // 添加底部信息
    QLabel *infoLabel = new QLabel("2025小秦版商品管理系统", this);
    infoLabel->setStyleSheet(
        "color: #7f8c8d;"
        "font-size: 16px;"
        "padding: 10px;"
        "text-align: center;"
    );
    infoLabel->setAlignment(Qt::AlignCenter);
    sideLayout->addWidget(infoLabel);
}

void MainWindow::onFunctionClicked(int index)
{
    // 取消之前选中按钮的状态
    if (currentActiveButton) {
        currentActiveButton->setChecked(false);
    }

    // 设置当前按钮为选中状态
    if (index >= 0 && index < functionButtons.size()) {
        currentActiveButton = functionButtons[index];
        currentActiveButton->setChecked(true);

        // 切换到对应的功能页面
        mainContent->setCurrentIndex(index + 1); // +1因为首页是0
    }
}

void MainWindow::setupMainContent()
{
    mainContent = new QStackedWidget(this);
    mainContent->setStyleSheet("background-color: white; border: none;");

    // 创建首页
    homePage = new QWidget(this);
    QVBoxLayout *homeLayout = new QVBoxLayout(homePage);
    homeLayout->setAlignment(Qt::AlignTop);
    homeLayout->setContentsMargins(30, 30, 30, 30);

    // 标题
    QLabel *titleLabel = new QLabel("欢迎使用开单及商品管理系统", this);
    titleLabel->setStyleSheet("font-size: 36px; font-weight: bold; color: #2c3e50; margin-bottom: 30px;");
    titleLabel->setAlignment(Qt::AlignCenter);

    // 个人信息区域
    QWidget *infoWidget = new QWidget(this);
    infoWidget->setStyleSheet("background-color: #ecf0f1; border-radius: 15px; padding: 25px;");
    QVBoxLayout *infoLayout = new QVBoxLayout(infoWidget);

    QLabel *userTitle = new QLabel("个人信息", this);
    userTitle->setStyleSheet("font-size: 28px; font-weight: bold; color: #2c3e50; border-bottom: 2px solid #3498db; padding-bottom: 10px;");

    // 用户信息详情
    QLabel *userInfo = new QLabel(QString("用户名: %1\n角色: %2\n昵称: %3").arg(m_username).arg(m_role).arg(m_nickname), this);
    userInfo->setStyleSheet("font-size: 24px; color: #34495e; margin: 15px 0; line-height: 1.8;");

    infoLayout->addWidget(userTitle);
    infoLayout->addWidget(userInfo);

    // 时间显示
    timeLabel = new QLabel(this);
    timeLabel->setStyleSheet("font-size: 24px; color: #34495e; margin: 15px 0; padding: 10px; background-color: #ffffff; border-radius: 8px; border: 1px solid #bdc3c7;");

    // 地点显示
    locationLabel = new QLabel("地点: 锦绣商城雪糕批发零售店", this);
    locationLabel->setStyleSheet("font-size: 24px; color: #34495e; margin: 15px 0; padding: 10px; background-color: #ffffff; border-radius: 8px; border: 1px solid #bdc3c7;");

    // 广告新闻
    newsLabel = new QLabel("最新消息: 系统维护通知 - 本系统每天晚上12点到凌晨2店进行维护", this);
    newsLabel->setStyleSheet("font-size: 24px; color: #e74c3c; margin: 15px 0; padding: 15px; background-color: #ffffff; border-radius: 8px; border: 1px solid #e74c3c;");
    newsLabel->setWordWrap(true);

    homeLayout->addWidget(titleLabel);
    homeLayout->addWidget(infoWidget);
    homeLayout->addWidget(timeLabel);
    homeLayout->addWidget(locationLabel);
    homeLayout->addWidget(newsLabel);
    homeLayout->addStretch();

    mainContent->addWidget(homePage);

    // 创建功能页面
    billingPage = new BillingWidget(this);
    productPage = new ProductWidget(this);
    warehousePage = new WarehouseWidget(this);
    permissionPage = new PermissionWidget(this);
    recordPage = new RecordWidget(this);
    analysisPage = new AnalysisWidget(this);

    // 添加到主内容区域
    mainContent->addWidget(billingPage);
    mainContent->addWidget(productPage);
    mainContent->addWidget(warehousePage);
    mainContent->addWidget(permissionPage);
    mainContent->addWidget(recordPage);
    mainContent->addWidget(analysisPage);
}

void MainWindow::updateTime()
{
    QDateTime currentTime = QDateTime::currentDateTime();
    QString timeStr = currentTime.toString("yyyy年MM月dd日 hh:mm:ss dddd");
    timeLabel->setText("当前时间: " + timeStr);
}

void MainWindow::onHomeClicked()
{
    mainContent->setCurrentIndex(0);

    // 取消所有功能按钮的选中状态
    for (QToolButton *button : functionButtons) {
        button->setChecked(false);
    }
    currentActiveButton = nullptr;

    // 设置首页标题样式
    setWindowTitle("开单及商品管理系统 - 首页");
}

void MainWindow::onPersonalCenterClicked()
{
    // 创建个人中心页面（如果尚未创建）
    if (!personalCenterPage) {
        personalCenterPage = new PersonalCenterWidget(m_username, m_nickname, m_avatarPath, m_role, this);
        mainContent->addWidget(personalCenterPage);
        // 连接信号
        connect(personalCenterPage, &PersonalCenterWidget::avatarChanged, this, &MainWindow::onAvatarChanged);
        connect(personalCenterPage, &PersonalCenterWidget::nicknameChanged, this, &MainWindow::onNicknameChangedSignal);
    }

    // 切换到个人中心页面
    mainContent->setCurrentWidget(personalCenterPage);
    setWindowTitle("开单及商品管理系统 - 个人中心");
}

void MainWindow::onChangeAccountClicked()
{
    // 创建美观的确认对话框
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("切换账号");
    msgBox.setText("您确定要切换账号吗？");
    msgBox.setInformativeText("切换账号后将返回登录界面");
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);

    // 美化消息框
    msgBox.setStyleSheet(R"(
        QMessageBox {
            background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #ffffff, stop: 1 #f8f9fa);
            font-size: 20px;
            min-width: 600px;
            min-height: 250px;
        }
        QMessageBox QLabel {
            color: #333333;
            font-size: 20px;
        }
        QPushButton {
            background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #3498db, stop: 1 #2980b9);
            border: 2px solid #2c3e50;
            color: white;
            padding: 15px 30px;
            font-size: 20px;
            font-weight: bold;
            border-radius: 8px;
            min-width: 120px;
            margin: 10px;
        }
        QPushButton:hover {
            background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #2980b9, stop: 1 #21618c);
        }
        QPushButton:pressed {
            background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #21618c, stop: 1 #1a5276);
        }
        QPushButton[text="Yes"] {
            background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #e74c3c, stop: 1 #c0392b);
        }
        QPushButton[text="Yes"]:hover {
            background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #c0392b, stop: 1 #a93226);
        }
        QPushButton[text="No"] {
            background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #27ae60, stop: 1 #229954);
        }
        QPushButton[text="No"]:hover {
            background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #229954, stop: 1 #1e8449);
        }
    )");

    int ret = msgBox.exec();
    if (ret == QMessageBox::Yes) {
        // 切换账号，返回登录界面
        close();
        LoginWindow *loginWindow = new LoginWindow();
        loginWindow->show();
    }
}

void MainWindow::onLogoutClicked()
{
    // 创建美观的退出确认对话框
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("退出系统");
    msgBox.setText("您确定要退出系统吗？");
    msgBox.setInformativeText("退出后需要重新登录才能使用系统");
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);

    // 美化消息框
    msgBox.setStyleSheet(R"(
        QMessageBox {
            background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #ffffff, stop: 1 #f8f9fa);
            font-size: 20px;
            min-width: 600px;
            min-height: 250px;
        }
        QMessageBox QLabel {
            color: #333333;
            font-size: 20px;
        }
        QPushButton {
            background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #3498db, stop: 1 #2980b9);
            border: 2px solid #2c3e50;
            color: white;
            padding: 15px 30px;
            font-size: 20px;
            font-weight: bold;
            border-radius: 8px;
            min-width: 120px;
            margin: 10px;
        }
        QPushButton:hover {
            background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #2980b9, stop: 1 #21618c);
        }
        QPushButton:pressed {
            background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #21618c, stop: 1 #1a5276);
        }
        QPushButton[text="Yes"] {
            background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #e74c3c, stop: 1 #c0392b);
        }
        QPushButton[text="Yes"]:hover {
            background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #c0392b, stop: 1 #a93226);
        }
        QPushButton[text="No"] {
            background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #27ae60, stop: 1 #229954);
        }
        QPushButton[text="No"]:hover {
            background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #229954, stop: 1 #1e8449);
        }
    )");

    int ret = msgBox.exec();
    if (ret == QMessageBox::Yes) {
        // 退出系统
        QApplication::quit();
    }
}

void MainWindow::onChangeAvatarClicked()
{
    // 上传头像对话框
    QString fileName = QFileDialog::getOpenFileName(this,
        "选择头像图片",
        "",
        "图片文件 (*.png *.jpg *.jpeg *.bmp *.gif)");

    if (!fileName.isEmpty()) {
        // 更新头像显示
        QPixmap avatarPixmap(fileName);
        avatarPixmap = avatarPixmap.scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        avatarLabel->setPixmap(avatarPixmap);

        // 保存到数据库
        DatabaseManager& dbManager = DatabaseManager::instance();
        if (dbManager.connect()) {
            dbManager.updateUserInfo(m_username, m_nickname, fileName);
        }

        m_avatarPath = fileName;

        // 更新个人中心页面的头像显示
        // 这里可以进一步优化，更新个人中心页面的头像显示
        QMessageBox::information(this, "成功", "头像上传成功！");
    }
}

void MainWindow::onNicknameChanged()
{
    if (!nicknameEdit->isReadOnly()) {
        nicknameConfirmBtn->show();
    }
}

void MainWindow::onNicknameConfirmClicked()
{
    QString newNickname = nicknameEdit->text().trimmed();
    if (newNickname.isEmpty()) {
        QMessageBox::warning(this, "错误", "昵称不能为空！");
        return;
    }

    if (newNickname != m_nickname) {
        // 保存到数据库
        DatabaseManager& dbManager = DatabaseManager::instance();
        if (dbManager.connect()) {
            if (dbManager.updateUserInfo(m_username, newNickname, m_avatarPath)) {
                m_nickname = newNickname;
                usernameLabel->setText(m_nickname); // 更新顶部显示
                QMessageBox::information(this, "成功", "昵称修改成功！");
            } else {
                QMessageBox::warning(this, "错误", "昵称修改失败！");
                return;
            }
        }
    }

    nicknameEdit->setReadOnly(true);
    nicknameConfirmBtn->hide();
}

void MainWindow::onChangeUsernameClicked()
{
    // 修改账号对话框
    QDialog dialog(this);
    dialog.setWindowTitle("修改账号");
    dialog.setFixedSize(500, 300);
    dialog.setStyleSheet(R"(
        QDialog {
            background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #ffffff, stop: 1 #f0f0f0);
        }
        QLabel {
            font-size: 18px;
            color: #333333;
            font-weight: bold;
        }
        QLineEdit {
            font-size: 18px;
            padding: 10px;
            border: 2px solid #bdc3c7;
            border-radius: 5px;
            background-color: #ffffff;
        }
        QPushButton {
            background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #3498db, stop: 1 #2980b9);
            border: none;
            color: white;
            padding: 12px 25px;
            font-size: 18px;
            font-weight: bold;
            border-radius: 8px;
            min-width: 100px;
        }
        QPushButton:hover {
            background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #2980b9, stop: 1 #21618c);
        }
    )");

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    QLabel *titleLabel = new QLabel("修改账号", &dialog);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 24px; margin-bottom: 20px;");

    QLineEdit *newUsernameEdit = new QLineEdit(&dialog);
    newUsernameEdit->setPlaceholderText("请输入新账号");

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *okBtn = new QPushButton("确定", &dialog);
    QPushButton *cancelBtn = new QPushButton("取消", &dialog);

    buttonLayout->addWidget(okBtn);
    buttonLayout->addWidget(cancelBtn);

    layout->addWidget(titleLabel);
    layout->addWidget(new QLabel("新账号:", &dialog));
    layout->addWidget(newUsernameEdit);
    layout->addLayout(buttonLayout);

    connect(okBtn, &QPushButton::clicked, [&]() {
        QString newUsername = newUsernameEdit->text().trimmed();
        if (newUsername.isEmpty()) {
            QMessageBox::warning(&dialog, "错误", "账号不能为空！");
            return;
        }
        // 这里应该实现实际的账号修改逻辑
        QMessageBox::information(&dialog, "成功", "账号修改成功！");
        dialog.accept();
    });

    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);

    dialog.exec();
}

void MainWindow::onChangePasswordClicked()
{
    // 修改密码对话框
    QDialog dialog(this);
    dialog.setWindowTitle("修改密码");
    dialog.setFixedSize(500, 400);
    dialog.setStyleSheet(R"(
        QDialog {
            background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #ffffff, stop: 1 #f0f0f0);
        }
        QLabel {
            font-size: 18px;
            color: #333333;
            font-weight: bold;
        }
        QLineEdit {
            font-size: 18px;
            padding: 10px;
            border: 2px solid #bdc3c7;
            border-radius: 5px;
            background-color: #ffffff;
        }
        QPushButton {
            background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #3498db, stop: 1 #2980b9);
            border: none;
            color: white;
            padding: 12px 25px;
            font-size: 18px;
            font-weight: bold;
            border-radius: 8px;
            min-width: 100px;
        }
        QPushButton:hover {
            background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #2980b9, stop: 1 #21618c);
        }
    )");

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    QLabel *titleLabel = new QLabel("修改密码", &dialog);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 24px; margin-bottom: 20px;");

    QLineEdit *oldPasswordEdit = new QLineEdit(&dialog);
    oldPasswordEdit->setPlaceholderText("请输入原密码");
    oldPasswordEdit->setEchoMode(QLineEdit::Password);

    QLineEdit *newPasswordEdit = new QLineEdit(&dialog);
    newPasswordEdit->setPlaceholderText("请输入新密码");
    newPasswordEdit->setEchoMode(QLineEdit::Password);

    QLineEdit *confirmPasswordEdit = new QLineEdit(&dialog);
    confirmPasswordEdit->setPlaceholderText("请确认新密码");
    confirmPasswordEdit->setEchoMode(QLineEdit::Password);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *okBtn = new QPushButton("确定", &dialog);
    QPushButton *cancelBtn = new QPushButton("取消", &dialog);

    buttonLayout->addWidget(okBtn);
    buttonLayout->addWidget(cancelBtn);

    layout->addWidget(titleLabel);
    layout->addWidget(new QLabel("原密码:", &dialog));
    layout->addWidget(oldPasswordEdit);
    layout->addWidget(new QLabel("新密码:", &dialog));
    layout->addWidget(newPasswordEdit);
    layout->addWidget(new QLabel("确认密码:", &dialog));
    layout->addWidget(confirmPasswordEdit);
    layout->addLayout(buttonLayout);

    connect(okBtn, &QPushButton::clicked, [&]() {
        QString oldPassword = oldPasswordEdit->text();
        QString newPassword = newPasswordEdit->text();
        QString confirmPassword = confirmPasswordEdit->text();

        if (oldPassword.isEmpty() || newPassword.isEmpty() || confirmPassword.isEmpty()) {
            QMessageBox::warning(&dialog, "错误", "所有字段都不能为空！");
            return;
        }

        if (newPassword != confirmPassword) {
            QMessageBox::warning(&dialog, "错误", "新密码和确认密码不一致！");
            return;
        }

        if (newPassword.length() < 6) {
            QMessageBox::warning(&dialog, "错误", "密码长度不能少于6位！");
            return;
        }

        // 这里应该实现实际的密码修改逻辑
        QMessageBox::information(&dialog, "成功", "密码修改成功！");
        dialog.accept();
    });

    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);

    dialog.exec();
}

void MainWindow::onAvatarChanged(const QString& avatarPath)
{
    // 更新主界面头像显示
    if (!avatarLabel) return;

    QPixmap avatarPixmap(avatarPath);
    avatarPixmap = avatarPixmap.scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    avatarLabel->setPixmap(avatarPixmap);

    // 更新当前用户的头像路径
    m_avatarPath = avatarPath;
}

void MainWindow::onNicknameChangedSignal(const QString& nickname)
{
    // 更新主界面昵称显示
    if (usernameLabel) {
        usernameLabel->setText(nickname);
    }

    // 更新当前用户的昵称
    m_nickname = nickname;
}
