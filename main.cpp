#include <QSystemTrayIcon>
#include <QApplication>
#include <QMessageBox>
#include <QPainter>
#include <QTimer>

QSystemTrayIcon* trayIcon;
QIcon currentIcon;

QIcon generateIconFromTime() {
    constexpr size_t imageSize = 64;
    QPixmap pix(imageSize, imageSize);
    QPainter paint(&pix);
    constexpr int gray = 64;
    paint.fillRect(0, 0, imageSize, imageSize, QColor(gray,gray,gray,255));

    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    auto tm = std::localtime(&now);
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
    QTimer timer;
    timer.setInterval(1000);
    QTimer::connect(&timer, &QTimer::timeout, [&timer] {
        if(trayIcon)
        {
            trayIcon->setIcon(generateIconFromTime());
            trayIcon->show();
            auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            auto tm = std::localtime(&now);
            if(tm->tm_sec > 0)
                timer.setInterval((60 - tm->tm_sec) * 1000);
            else
                timer.setInterval(60000);
        }
    });
    timer.start();
    return app.exec();
}
