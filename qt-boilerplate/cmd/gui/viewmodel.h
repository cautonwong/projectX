#pragma once
#include <QObject>
#include <QString>
#include "core/interfaces/ILoginService.h"

class MainWindowViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isLoggedIn READ isLoggedIn NOTIFY isLoggedInChanged)
    Q_PROPERTY(QString userName READ userName NOTIFY userNameChanged)
public:
    explicit MainWindowViewModel(ILoginService* service, QObject* parent = nullptr);
    bool isLoggedIn() const;
    QString userName() const;
public slots:
    Q_INVOKABLE void login();
private slots:
    void onLoginResult(bool success, const UserInfo& info);
signals:
    void isLoggedInChanged();
    void userNameChanged();
private:
    ILoginService* m_service;
    bool m_isLoggedIn = false;
    QString m_userName = "Guest";
};