import QtQuick 2.15
import QtQuick.Controls 2.15

ApplicationWindow {
    visible: true
    width: 400
    height: 300
    title: "Boilerplate App"

    Text {
        anchors.centerIn: parent
        text: viewModel.isLoggedIn ? `Welcome, ${viewModel.userName}!` : "Please Log In"
        font.pixelSize: 20
    }
    
    Button {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20
        text: "Login"
        enabled: !viewModel.isLoggedIn
        onClicked: viewModel.login()
    }
}