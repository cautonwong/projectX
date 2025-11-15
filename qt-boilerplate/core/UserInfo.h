#pragma once
#include <QMetaType>
#include <QString>

struct UserInfo {
    int id = -1;
    QString name;
};
Q_DECLARE_METATYPE(UserInfo)