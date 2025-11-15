#pragma once
#include "core/datatypes/UserInfo.h"
#include <QObject>

class ILoginService : public QObject {
    Q_OBJECT
public:
    using QObject::QObject;
    virtual ~ILoginService() = default;

public slots:
    virtual void performLogin(const QString& user, const QString& pass) = 0;

signals:
    void loginFinished(bool success, const UserInfo& userInfo);
};