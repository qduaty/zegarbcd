#include <QMessageBox>
#include <qapplication.h>
#include "hiddenwindow.h"
#include "QSystemTrayIcon.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);
    HiddenWindow w;
    if (QSystemTrayIcon::isSystemTrayAvailable())
    {
        // w.setWindowFlags(Qt::Popup);
    }
    else
    {
        QMessageBox::critical(nullptr, QObject::tr("Zegarbcd"),
                              QObject::tr("I couldn't detect any system tray "
                                          "on this system."));
        return 1;
    }

    return app.exec();
}
