#ifndef REGISTERWINDOW_H
#define REGISTERWINDOW_H

#include <QDialog>
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QMessageBox>

class RegisterWindow : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterWindow(QWidget *parent = nullptr);

private slots:
    void onRegisterClicked();
    void onExitClicked();
    void onRoleChanged(int index);

private:
    void setupUI();
    void adjustUI();

    // UI组件
    QLabel *titleLabel;
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QComboBox *roleComboBox;
    QLineEdit *adminKeyEdit;
    QPushButton *registerButton;
    QPushButton *exitButton;

    QWidget *adminKeyWidget;
};

#endif
