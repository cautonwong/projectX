#include "LoginService.h"
#include <QTimer> // For simulation
#include <QDebug>

void LoginService::performLogin(const QString& user, const QString& pass) {
    qDebug() << "Service: Attempting to log in...";
    // Simulate async network request
    QTimer::singleShot(1500, this, [=](){
        if (user == "admin" && pass == "12345") {
            qDebug() << "Service: Login successful.";
            emit loginFinished(true, {1, "Admin User"});
        } else {
            qDebug() << "Service: Login failed.";
            emit loginFinished(false, {});
        }
    });
}