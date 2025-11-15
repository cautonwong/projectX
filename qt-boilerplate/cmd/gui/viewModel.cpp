#include "MainWindowViewModel.h"

MainWindowViewModel::MainWindowViewModel(ILoginService* service, QObject* parent)
    : QObject(parent), m_service(service) {
    connect(m_service, &ILoginService::loginFinished, this, &MainWindowViewModel::onLoginResult);
}

bool MainWindowViewModel::isLoggedIn() const { return m_isLoggedIn; }
QString MainWindowViewModel::userName() const { return m_userName; }

void MainWindowViewModel::login() {
    if (m_service) m_service->performLogin("admin", "12345");
}

void MainWindowViewModel::onLoginResult(bool success, const UserInfo& info) {
    if (success != m_isLoggedIn) {
        m_isLoggedIn = success;
        emit isLoggedInChanged();
    }
    if (info.name != m_userName) {
        m_userName = success ? info.name : "Guest";
        emit userNameChanged();
    }
}