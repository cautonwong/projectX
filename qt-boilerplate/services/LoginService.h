#pragma once
#include "core/interfaces/ILoginService.h"

class LoginService : public ILoginService {
    Q_OBJECT
public:
    using ILoginService::ILoginService;

public slots:
    void performLogin(const QString& user, const QString& pass) override;
};