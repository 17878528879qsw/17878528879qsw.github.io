#include <QApplication>
#include "loginwindow.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 设置应用程序信息
    app.setApplicationName("开单及商品管理系统");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("ProductManagement");

    // 显示登录窗口
    LoginWindow loginWindow;
    loginWindow.show();

    return app.exec();
}
