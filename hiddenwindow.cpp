#include "hiddenwindow.h"
#include <iomanip>
#include <sstream>
#include <windows.h>
#include <wtsapi32.h>
#include <boost/signals2/signal.hpp>
#include <QAbstractNativeEventFilter>
#include <QMenu>
#include <QPainter>
#include <QDate>

class MyEventFilter : public QAbstractNativeEventFilter {
public:
    bool nativeEventFilter(const QByteArray& eventType, void* message, qintptr *) Q_DECL_OVERRIDE {
        MSG* msg = static_cast< MSG* >( message );
        // TODO left this code in case the clock needs a nudge also after system wake-up - remove if not needed
        // if (msg->message == WM_POWERBROADCAST) {
        //     switch (msg->wParam) {
        //     case PBT_APMPOWERSTATUSCHANGE:
        //         qDebug() << ("PBT_APMPOWERSTATUSCHANGE  received\n");
        //         break;
        //     case PBT_APMRESUMEAUTOMATIC:
        //         qDebug() << ("PBT_APMRESUMEAUTOMATIC  received\n");
        //         break;
        //     case PBT_APMRESUMESUSPEND:
        //         qDebug() << ("PBT_APMRESUMESUSPEND  received\n");
        //         break;
        //     case PBT_APMSUSPEND:
        //         qDebug() << ("PBT_APMSUSPEND  received\n");
        //         break;
        //     }
        // }
        // else
        if (msg->message == WM_WTSSESSION_CHANGE)
        {
            event();
        }
        return false;
    }
    boost::signals2::signal<void()> event;
};

QIcon generate4x4IconFromTime(struct tm * tm) {
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

QIcon generate3x4IconFromTime(struct tm * tm) {
    constexpr size_t imageSize = 64;
    QPixmap pix(imageSize, imageSize);
    QPainter paint(&pix);
    constexpr int gray = 64;
    paint.fillRect(0, 0, imageSize, imageSize, QColor(gray,gray,gray,255));

    int digits[3];
    digits[0] = tm->tm_hour % 12;
    digits[1] = tm->tm_min / 10;
    digits[2] = tm->tm_min % 10;
    for(int x = 0; x < 3; x++)
        for(int y = 0; y < 4; y++)
        {
            constexpr int xsize = imageSize / 3;
            constexpr int ysize = imageSize / 4;
            constexpr int margin = imageSize / 16;
            int xpos = x * xsize + margin;
            int ypos = y * ysize + margin;
            QColor color;
            if((digits[x] >> (3 - y)) & 0x1)
                color = QColor(255,255,255,255);
            else
                color = QColor(0,0,0,255);
            paint.fillRect(xpos, ypos, xsize - margin, ysize - margin, color);
        }

    return pix;
}

HiddenWindow::HiddenWindow(QWidget *parent):
    QMainWindow{parent},
    settings("HKEY_CURRENT_USER\\Software\\qduaty\\zegarbcd", QSettings::NativeFormat),
    settingsRunOnStartup("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat),
    iconGenerator(generate4x4IconFromTime)
{
    trayIcon = new QSystemTrayIcon(this);

    auto trayIconMenu = new QMenu(this);
    auto mode12hourAction = new QAction(tr("&12 hours"), this);
    mode12hourAction->setCheckable(true);
    bool mode12Hours = settings.value("mode12hours").toBool();
    mode12hourAction->setChecked(mode12Hours);
    set12HourMode(mode12Hours);
    connect(mode12hourAction, &QAction::toggled, this, &HiddenWindow::set12HourMode);

    auto quitAction = new QAction(tr("&Uninstall"), this);
    connect(quitAction, &QAction::triggered, this, &HiddenWindow::quitAndUnregister);

    trayIconMenu->addAction(mode12hourAction);
    trayIconMenu->addAction(quitAction);
    trayIcon->setContextMenu(trayIconMenu);
    timer.setInterval(1000);
    connect(&timer, &QTimer::timeout, this, &HiddenWindow::updateTrayIcon);
    timer.start();
    WTSRegisterSessionNotification(reinterpret_cast<HWND>(winId()), NOTIFY_FOR_ALL_SESSIONS);
    auto myEvenfilter = new MyEventFilter;
    myEvenfilter->event.connect(std::bind(&HiddenWindow::updateTrayIcon, this));
    QApplication::instance()->installNativeEventFilter(myEvenfilter);
    registerForStartup();
}

void HiddenWindow::registerForStartup() {
    settingsRunOnStartup.setValue(QApplication::applicationName(),
                                  QCoreApplication::applicationFilePath().replace('/', "\\"));
}

void HiddenWindow::quitAndUnregister() {
    settingsRunOnStartup.remove(QApplication::applicationName());
    QCoreApplication::instance()->quit();
}

void HiddenWindow::updateTrayIcon() {
    if(trayIcon)
    {
        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        auto tm = std::localtime(&now);
        trayIcon->setIcon(iconGenerator(tm));
        trayIcon->setToolTip(QLocale().toString(QDate::currentDate(), "dddd d MMMM yyyy"));
        trayIcon->show();
        if(tm->tm_sec > 0)
            timer.setInterval((60 - tm->tm_sec) * 1000);
        else
            timer.setInterval(60000);
    }
}

void HiddenWindow::set12HourMode(bool arg)
{
    qDebug() << arg;
    if(arg)
        iconGenerator = generate3x4IconFromTime;
    else
        iconGenerator = generate4x4IconFromTime;
    updateTrayIcon();
    settings.setValue("mode12hours", arg);
}
