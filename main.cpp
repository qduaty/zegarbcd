#include <QMessageBox>
#include "hiddenwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(nullptr, QObject::tr("Zegarbcd"),
                              QObject::tr("I couldn't detect any system tray "
                                          "on this system."));
        return 1;
    }

    HiddenWindow w;
    return app.exec();
}
