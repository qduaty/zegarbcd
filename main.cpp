#include <QSystemTrayIcon>
#include <QApplication>
#include <QMessageBox>
#include <QPainter>
#include <QTimer>
#include <QSettings>
#include <QMenu>
#include <iomanip>
#include <sstream>

QSystemTrayIcon* trayIcon;
QIcon currentIcon;

QIcon generateIconFromTime(struct tm * tm) {
    constexpr size_t imageSize = 64;
    QPixmap pix(imageSize, imageSize);
    QPainter paint(&pix);
    constexpr int gray = 64;
    paint.fillRect(0, 0, imageSize, imageSize, QColor(gray,gray,gray,255));

    int digits[4];
    digits[0] = tm->tm_hour / 10;
    digits[1] = tm->tm_hour % 10;
    digits[2] = tm->tm_min / 10;
    digits[3] = tm->tm_min % 10;
    for(int x = 0; x < 4; x++)
        for(int y = 0; y < 4; y++)
        {
            constexpr int size = imageSize / 4;
            constexpr int margin = imageSize / 16;
            int xpos = x * size + margin;
            int ypos = y * size + margin;
            QColor color;
            if((digits[x] >> (3 - y)) & 0x1)
                color = QColor(255,255,255,255);
            else
                color = QColor(0,0,0,255);
            paint.fillRect(xpos, ypos, size - margin, size - margin, color);
        }

    return pix;
}

void registerForStartup() {
    QSettings settingsRunOnStartup("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    settingsRunOnStartup.setValue(QApplication::applicationName(),
                                  QCoreApplication::applicationFilePath().replace('/', "\\"));
}

void quitAndUnregister() {
    QSettings settingsRunOnStartup("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    settingsRunOnStartup.remove(QApplication::applicationName());
    QCoreApplication::instance()->quit();
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(nullptr, QObject::tr("Zegarbcd"),
                              QObject::tr("I couldn't detect any system tray "
                                          "on this system."));
        return 1;
    }
    trayIcon = new QSystemTrayIcon(nullptr);
    auto trayIconMenu = new QMenu(nullptr);
    auto quitAction = new QAction(app.tr("&Uninstall"), nullptr);
    app.connect(quitAction, &QAction::triggered, quitAndUnregister);

    trayIconMenu->addAction(quitAction);
    trayIcon->setContextMenu(trayIconMenu);

    QTimer timer;
    timer.setInterval(1000);
    QTimer::connect(&timer, &QTimer::timeout, [&timer] {
        if(trayIcon)
        {
            auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            auto tm = std::localtime(&now);
            trayIcon->setIcon(generateIconFromTime(tm));
            std::stringstream buffer;
            buffer << std::put_time(tm, "%A %d %B %Y");
            trayIcon->setToolTip(QString::fromLocal8Bit(buffer.str()));
            trayIcon->show();
            if(tm->tm_sec > 0)
                timer.setInterval((60 - tm->tm_sec) * 1000);
            else
                timer.setInterval(60000);
        }
    });
    timer.start();
    registerForStartup();
    return app.exec();
}
