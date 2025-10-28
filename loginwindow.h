#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QSettings>
#include <QResizeEvent>

class LoginWindow : public QMainWindow
{
    Q_OBJECT

public:
    LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onLoginClicked();
    void onRegisterClicked();
    void onExitClicked();
    void onShowPasswordClicked();
    void onRememberPasswordClicked();

private:
    void setupUI();
    void loadSavedCredentials();
    void saveCredentials();
    void clearCredentials();
    void adjustUI();

    // UI组件
    QWidget *centralWidget;
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QPushButton *showPasswordButton;
    QComboBox *roleComboBox;
    QCheckBox *rememberPasswordCheckBox;
    QPushButton *loginButton;
    QPushButton *registerButton;
    QPushButton *exitButton;
    QLabel *titleLabel;

    bool passwordVisible;
};

#endif
