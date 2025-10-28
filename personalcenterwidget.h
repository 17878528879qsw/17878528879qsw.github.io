#ifndef PERSONALCENTERWIDGET_H
#define PERSONALCENTERWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>

class PersonalCenterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PersonalCenterWidget(const QString& username, const QString& nickname,
                                 const QString& avatarPath, const QString& role,
                                 QWidget *parent = nullptr);
    ~PersonalCenterWidget();

signals:
    void avatarChanged(const QString& avatarPath);
    void nicknameChanged(const QString& nickname);

private slots:
    void onChangeAvatarClicked();
    void onNicknameEditClicked();
    void onNicknameConfirmClicked();
    void onChangePasswordClicked();
    void onShowPasswordClicked();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;  // 添加事件过滤器

private:
    void setupUI();
    void updateAvatarDisplay();

    QString m_username;
    QString m_nickname;
    QString m_avatarPath;
    QString m_role;

    // UI组件
    QLabel *avatarDisplay;
    QLineEdit *nicknameEdit;
    QPushButton *nicknameConfirmBtn;
    QLineEdit *usernameDisplay;
    QLineEdit *passwordDisplay;
    QPushButton *showPasswordBtn;

    bool passwordVisible;
};

#endif
