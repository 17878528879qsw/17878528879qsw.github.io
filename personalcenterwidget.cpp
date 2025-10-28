#include "personalcenterwidget.h"
#include "usermanager.h"
#include <QMessageBox>
#include <QPixmap>
#include <QFileDialog>
#include <QDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QEvent>

PersonalCenterWidget::PersonalCenterWidget(const QString& username, const QString& nickname,
                                         const QString& avatarPath, const QString& role,
                                         QWidget *parent)
    : QWidget(parent)
    , m_username(username)
    , m_nickname(nickname)
    , m_avatarPath(avatarPath)
    , m_role(role)
    , avatarDisplay(nullptr)
    , nicknameEdit(nullptr)
    , nicknameConfirmBtn(nullptr)
    , usernameDisplay(nullptr)
    , passwordDisplay(nullptr)
    , showPasswordBtn(nullptr)
    , passwordVisible(false)
{
    setupUI();
}

PersonalCenterWidget::~PersonalCenterWidget()
{
}

void PersonalCenterWidget::setupUI()
{
    // 创建滚动区域
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet("QScrollArea { border: none; }");

    QWidget *contentWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(contentWidget);
    mainLayout->setAlignment(Qt::AlignTop);
    mainLayout->setContentsMargins(50, 30, 50, 30);

    // 标题
    QLabel *titleLabel = new QLabel("个人中心", this);
    titleLabel->setStyleSheet("font-size: 36px; font-weight: bold; color: #2c3e50; margin-bottom: 30px; border-bottom: 2px solid #3498db; padding-bottom: 15px;");
    titleLabel->setAlignment(Qt::AlignCenter);

    // 个人信息表单
    QFrame *infoFrame = new QFrame(this);
    infoFrame->setStyleSheet("QFrame { background-color: #ffffff; border: 1px solid #bdc3c7; border-radius: 10px; padding: 20px; }");
    QVBoxLayout *infoLayout = new QVBoxLayout(infoFrame);

    // 头像行
    QHBoxLayout *avatarLayout = new QHBoxLayout();
    QLabel *avatarTitle = new QLabel("头像:", this);
    avatarTitle->setStyleSheet("font-size: 24px; font-weight: bold; color: #34495e;");

    // 头像显示
    avatarDisplay = new QLabel(this);
    avatarDisplay->setFixedSize(120, 120);
    avatarDisplay->setStyleSheet("border: 2px dashed #bdc3c7; border-radius: 60px; background-color: #ecf0f1;");
    avatarDisplay->setAlignment(Qt::AlignCenter);
    avatarDisplay->setCursor(Qt::PointingHandCursor);
    avatarDisplay->installEventFilter(this); // 安装事件过滤器
    updateAvatarDisplay();

    // 头像上传按钮
    QPushButton *uploadAvatarBtn = new QPushButton("上传头像", this);
    uploadAvatarBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #3498db;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 5px;"
        "   padding: 10px 20px;"
        "   font-size: 18px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #2980b9;"
        "}"
    );
    connect(uploadAvatarBtn, &QPushButton::clicked, this, &PersonalCenterWidget::onChangeAvatarClicked);

    avatarLayout->addWidget(avatarTitle);
    avatarLayout->addWidget(avatarDisplay);
    avatarLayout->addWidget(uploadAvatarBtn);
    avatarLayout->addStretch();

    // 昵称行
    QHBoxLayout *nicknameLayout = new QHBoxLayout();
    QLabel *nicknameTitle = new QLabel("昵称:", this);
    nicknameTitle->setStyleSheet("font-size: 24px; font-weight: bold; color: #34495e;");

    nicknameEdit = new QLineEdit(m_nickname, this);
    nicknameEdit->setStyleSheet(
        "QLineEdit {"
        "   font-size: 20px;"
        "   padding: 10px;"
        "   border: 1px solid #bdc3c7;"
        "   border-radius: 5px;"
        "   background-color: #ffffff;"
        "}"
    );
    nicknameEdit->setReadOnly(true);

    QPushButton *changeNicknameBtn = new QPushButton("修改昵称", this);
    changeNicknameBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #f39c12;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 5px;"
        "   padding: 10px 20px;"
        "   font-size: 18px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #d68910;"
        "}"
    );
    connect(changeNicknameBtn, &QPushButton::clicked, this, [&]() {
        if (nicknameEdit->isReadOnly()) {
            nicknameEdit->setReadOnly(false);
            nicknameEdit->setFocus();
            nicknameEdit->selectAll();
        }
    });

    nicknameConfirmBtn = new QPushButton("✓", this);
    nicknameConfirmBtn->setFixedSize(40, 40);
    nicknameConfirmBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #27ae60;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 20px;"
        "   font-size: 20px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #229954;"
        "}"
    );
    nicknameConfirmBtn->hide();

    connect(nicknameEdit, &QLineEdit::textChanged, this, &PersonalCenterWidget::onNicknameEditClicked);
    connect(nicknameConfirmBtn, &QPushButton::clicked, this, &PersonalCenterWidget::onNicknameConfirmClicked);
    connect(nicknameEdit, &QLineEdit::editingFinished, [=]() {
        if (!nicknameEdit->isReadOnly()) {
            nicknameConfirmBtn->show();
        }
    });
    connect(nicknameEdit, &QLineEdit::returnPressed, this, &PersonalCenterWidget::onNicknameConfirmClicked);

    nicknameLayout->addWidget(nicknameTitle);
    nicknameLayout->addWidget(nicknameEdit);
    nicknameLayout->addWidget(changeNicknameBtn);
    nicknameLayout->addWidget(nicknameConfirmBtn);

    // 账号行
    QHBoxLayout *usernameLayout = new QHBoxLayout();
    QLabel *usernameTitle = new QLabel("账号:", this);
    usernameTitle->setStyleSheet("font-size: 24px; font-weight: bold; color: #34495e;");

    usernameDisplay = new QLineEdit(m_username, this);
    usernameDisplay->setStyleSheet(
        "QLineEdit {"
        "   font-size: 20px;"
        "   padding: 10px;"
        "   border: 1px solid #bdc3c7;"
        "   border-radius: 5px;"
        "   background-color: #f8f9fa;"
        "}"
    );
    usernameDisplay->setReadOnly(true);

    // 移除修改账号按钮，账号不可修改

    usernameLayout->addWidget(usernameTitle);
    usernameLayout->addWidget(usernameDisplay);

    // 密码行
    QHBoxLayout *passwordLayout = new QHBoxLayout();
    QLabel *passwordTitle = new QLabel("密码:", this);
    passwordTitle->setStyleSheet("font-size: 24px; font-weight: bold; color: #34495e;");

    passwordDisplay = new QLineEdit("********", this);
    passwordDisplay->setStyleSheet(
        "QLineEdit {"
        "   font-size: 20px;"
        "   padding: 10px;"
        "   border: 1px solid #bdc3c7;"
        "   border-radius: 5px;"
        "   background-color: #f8f9fa;"
        "}"
    );
    passwordDisplay->setReadOnly(true);
    passwordDisplay->setEchoMode(QLineEdit::Password);

    showPasswordBtn = new QPushButton("显示", this);
    showPasswordBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #9b59b6;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 5px;"
        "   padding: 10px 20px;"
        "   font-size: 18px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #8e44ad;"
        "}"
    );
    connect(showPasswordBtn, &QPushButton::clicked, this, &PersonalCenterWidget::onShowPasswordClicked);

    QPushButton *changePasswordBtn = new QPushButton("修改密码", this);
    changePasswordBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #e74c3c;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 5px;"
        "   padding: 10px 20px;"
        "   font-size: 18px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #c0392b;"
        "}"
    );
    connect(changePasswordBtn, &QPushButton::clicked, this, &PersonalCenterWidget::onChangePasswordClicked);

    passwordLayout->addWidget(passwordTitle);
    passwordLayout->addWidget(passwordDisplay);
    passwordLayout->addWidget(showPasswordBtn);
    passwordLayout->addWidget(changePasswordBtn);

    // 角色行
    QHBoxLayout *roleLayout = new QHBoxLayout();
    QLabel *roleTitle = new QLabel("角色:", this);
    roleTitle->setStyleSheet("font-size: 24px; font-weight: bold; color: #34495e;");

    QLineEdit *roleDisplay = new QLineEdit(m_role, this);
    roleDisplay->setStyleSheet(
        "QLineEdit {"
        "   font-size: 20px;"
        "   padding: 10px;"
        "   border: 1px solid #bdc3c7;"
        "   border-radius: 5px;"
        "   background-color: #f8f9fa;"
        "}"
    );
    roleDisplay->setReadOnly(true);

    roleLayout->addWidget(roleTitle);
    roleLayout->addWidget(roleDisplay);

    // 添加所有行到布局
    infoLayout->addLayout(avatarLayout);
    infoLayout->addLayout(nicknameLayout);
    infoLayout->addLayout(usernameLayout);
    infoLayout->addLayout(passwordLayout);
    infoLayout->addLayout(roleLayout);

    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(infoFrame);
    mainLayout->addStretch();

    scrollArea->setWidget(contentWidget);
    QVBoxLayout *pageLayout = new QVBoxLayout(this);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->addWidget(scrollArea);
}

void PersonalCenterWidget::updateAvatarDisplay()
{
    if (!avatarDisplay) return;

    if (!m_avatarPath.isEmpty() && QFile::exists(m_avatarPath)) {
        QPixmap avatarPixmap(m_avatarPath);
        avatarPixmap = avatarPixmap.scaled(120, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        avatarDisplay->setPixmap(avatarPixmap);
    } else {
        avatarDisplay->setText("点击上传");
    }
}

void PersonalCenterWidget::onChangeAvatarClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        "选择头像图片",
        "",
        "图片文件 (*.png *.jpg *.jpeg *.bmp *.gif)");

    if (!fileName.isEmpty()) {
        UserManager& userManager = UserManager::instance();
        if (userManager.updateUserInfo(m_username, m_nickname, fileName)) {
            m_avatarPath = fileName;
            updateAvatarDisplay();
            emit avatarChanged(fileName);
            QMessageBox::information(this, "成功", "头像上传成功！");
        } else {
            QMessageBox::warning(this, "错误", "头像上传失败！");
        }
    }
}

void PersonalCenterWidget::onNicknameEditClicked()
{
    if (nicknameEdit && !nicknameEdit->isReadOnly()) {
        if (nicknameConfirmBtn) {
            nicknameConfirmBtn->show();
        }
    }
}

void PersonalCenterWidget::onNicknameConfirmClicked()
{
    if (!nicknameEdit || !nicknameConfirmBtn) return;

    QString newNickname = nicknameEdit->text().trimmed();
    if (newNickname.isEmpty()) {
        QMessageBox::warning(this, "错误", "昵称不能为空！");
        return;
    }

    if (newNickname != m_nickname) {
        UserManager& userManager = UserManager::instance();
        if (userManager.updateUserInfo(m_username, newNickname, m_avatarPath)) {
            m_nickname = newNickname;
            emit nicknameChanged(newNickname);
            QMessageBox::information(this, "成功", "昵称修改成功！");
        } else {
            QMessageBox::warning(this, "错误", "昵称修改失败！");
            return;
        }
    }

    nicknameEdit->setReadOnly(true);
    nicknameConfirmBtn->hide();
}

void PersonalCenterWidget::onChangePasswordClicked()
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
    buttonLayout->setSpacing(20);  // 增加按钮间距

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

        UserManager& userManager = UserManager::instance();
        if (userManager.updatePassword(m_username, newPassword)) {
            QMessageBox::information(&dialog, "成功", "密码修改成功！");
            dialog.accept();
        } else {
            QMessageBox::warning(&dialog, "错误", "密码修改失败！");
        }
    });

    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);

    dialog.exec();
}

void PersonalCenterWidget::onShowPasswordClicked()
{
    if (!passwordDisplay || !showPasswordBtn) return;

    if (passwordVisible) {
        passwordDisplay->setEchoMode(QLineEdit::Password);
        showPasswordBtn->setText("显示");
        passwordVisible = false;
    } else {
        passwordDisplay->setEchoMode(QLineEdit::Normal);
        showPasswordBtn->setText("隐藏");
        passwordVisible = true;
    }
}

// 实现事件过滤器，处理头像点击事件
bool PersonalCenterWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == avatarDisplay && event->type() == QEvent::MouseButtonPress) {
        onChangeAvatarClicked();
        return true;
    }
    return QWidget::eventFilter(obj, event);
}
