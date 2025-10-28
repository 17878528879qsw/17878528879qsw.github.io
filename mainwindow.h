#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QStackedWidget>
#include <QToolButton>
#include <QMenu>
#include <QAction>
#include <QTimer>
#include <QDateTime>
#include <QPixmap>
#include <QMouseEvent>
#include <QEvent>
#include <QApplication>
#include <QLineEdit>
#include "billingwidget.h"
#include "permissionwidget.h"
#include "productwidget.h"
#include "recordwidget.h"
#include "analysiswidget.h"
#include "warehousewidget.h"

// 前向声明
class PersonalCenterWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const QString& username, const QString& role, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onHomeClicked();
    void onPersonalCenterClicked();
    void onChangeAccountClicked();
    void onLogoutClicked();
    void updateTime();
    void onFunctionClicked(int index);
    bool eventFilter(QObject *obj, QEvent *event);

    // 个人信息相关槽函数
    void onChangeAvatarClicked();
    void onNicknameChanged();
    void onNicknameConfirmClicked();
    void onChangeUsernameClicked();
    void onChangePasswordClicked();

    // 个人中心页面信号处理
    void onAvatarChanged(const QString& avatarPath);
    void onNicknameChangedSignal(const QString& nickname);

private:
    void setupUI();
    void setupTopBar();
    void setupSideBar();
    void setupMainContent();
    void setupPersonalMenu();
    void updateUserInfo();
    void setupPersonalCenterPage();

    // 用户信息
    QString m_username;
    QString m_role;
    QString m_nickname;
    QString m_avatarPath;

    // 顶部栏组件
    QWidget *topBar;
    QPushButton *homeButton;
    QWidget *userInfoWidget;
    QLabel *avatarLabel;
    QLabel *usernameLabel;
    QMenu *personalMenu;

    // 侧边栏组件
    QWidget *sideBar;
    QList<QToolButton*> functionButtons;
    QToolButton *currentActiveButton;

    // 主内容区域
    QStackedWidget *mainContent;

    // 页面组件
    QWidget *homePage;
    PersonalCenterWidget *personalCenterPage;

    // 其他功能页面暂时注释掉
    BillingWidget *billingPage;
    ProductWidget *productPage;
    WarehouseWidget *warehousePage;
    PermissionWidget *permissionPage;
    RecordWidget *recordPage;
    AnalysisWidget *analysisPage;

    // 个人中心页面组件
    QLineEdit *nicknameEdit;
    QPushButton *nicknameConfirmBtn;
    QLineEdit *passwordEdit;

    // 首页内容
    QLabel *timeLabel;
    QLabel *newsLabel;
    QLabel *locationLabel;

    // 定时器
    QTimer *timer;
};

#endif
