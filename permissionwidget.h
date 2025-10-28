#ifndef PERMISSIONWIDGET_H
#define PERMISSIONWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QTableWidget>
#include <QLabel>
#include <QGroupBox>
#include <QTextEdit>
#include <QDateTime>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QCheckBox>

class PermissionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PermissionWidget(QWidget *parent = nullptr);

private slots:
    void onAddUserClicked();
    void onModifyUserClicked();
    void onDeleteUserClicked();
    void onUserSelected();
    void onSearchClicked();
    void onResetSearchClicked();
    void onSortChanged(int column);

private:
    void setupUI();
    void setupUserTable();
    void setupButtons();
    void setupSearchBar();
    void loadUsers();
    bool checkPermission(const QString& operation);

    // UI 组件
    QLineEdit *usernameEdit;
    QLineEdit *nicknameEdit;
    QComboBox *roleComboBox;
    QLineEdit *passwordEdit;
    QLineEdit *confirmPasswordEdit;

    QPushButton *addBtn;
    QPushButton *modifyBtn;
    QPushButton *deleteBtn;

    QTableWidget *userTable;

    // 搜索相关组件
    QLineEdit *searchEdit;
    QComboBox *searchFieldComboBox;
    QPushButton *searchBtn;
    QPushButton *resetSearchBtn;

    int currentUserId;

    // 布局组件
    QHBoxLayout *buttonLayout;
    QHBoxLayout *searchLayout;
};

#endif
