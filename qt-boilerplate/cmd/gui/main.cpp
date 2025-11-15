#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "core/interfaces/ILoginService.h"
#include "services/LoginService.h"
#include "viewmodels/MainWindowViewModel.h"

void registerMetaTypes() {
    qRegisterMetaType<UserInfo>();
}

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    registerMetaTypes();

    LoginService loginService;
    MainWindowViewModel viewModel(&loginService);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("viewModel", &viewModel);
    
    engine.load(QUrl(QStringLiteral("qrc:/qt/qml/MainGuiApp/qml/Main.qml")));
    
    return app.exec();
}