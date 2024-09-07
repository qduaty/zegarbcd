#include "hiddenwindow.h"
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

QIcon generate24hourIconFromTime(struct tm * tm) {
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

QIcon generate12hourIconFromTime(struct tm * tm) {
    constexpr size_t imageSize = 64;
    QPixmap pix(imageSize, imageSize);
    QPainter paint(&pix);
    constexpr int gray = 64;
    paint.fillRect(0, 0, imageSize, imageSize, QColor(gray,gray,gray,255));

    int digits[3];
    digits[0] = (tm->tm_hour % 12);
    if(digits[0] == 0)
        digits[0] = 12;
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

QIcon generate5minIconFromTime(struct tm * tm) {
    constexpr size_t imageSize = 64;
    QPixmap pix(imageSize, imageSize);
    QPainter paint(&pix);
    constexpr int gray = 64;
    paint.fillRect(0, 0, imageSize, imageSize, QColor(gray,gray,gray,255));

    int digits[4];
    constexpr int sizes[4] = {2, 3, 2, 2};
    digits[0] = (tm->tm_hour / 8);
    digits[1] = (tm->tm_hour % 8);
    digits[2] = tm->tm_min / 15;
    digits[3] = (tm->tm_min % 15) / 5;

    for(int x = 0; x < 4; x++) {
        for(int y = 0; y < sizes[x]; y++)
        {
            constexpr int xsize = imageSize / 4;
            int ysize = imageSize / sizes[x];
            constexpr int xmargin = imageSize / 16;
            constexpr int ymargin = imageSize / 8;
            int xpos = x * xsize + xmargin;
            int ypos = y * ysize + ymargin * 2 / sizes[x];
            QColor color;
            if((digits[x] >> (sizes[x] - 1 - y)) & 0x1)
                color = QColor(255,255,255,255);
            else
                color = QColor(0,0,0,255);
            paint.fillRect(xpos, ypos, xsize - xmargin, ysize - ymargin * 2 / sizes[x], color);
        }
    }

    return pix;
}

HiddenWindow::HiddenWindow(QWidget *parent):
    QMainWindow{parent},
    settings("HKEY_CURRENT_USER\\Software\\qduaty\\zegarbcd", QSettings::NativeFormat),
    settingsRunOnStartup("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat),
    iconGenerator(generate24hourIconFromTime),
    mode24hourAction(new QAction(tr("&24 hours"), this)),
    mode12hourAction(new QAction(tr("&12 hours"), this)),
    mode5minAction(new QAction(tr("&5 min (1/3 day, hour, quarter, 5 min)"), this))
{
    trayIcon = new QSystemTrayIcon(this);

    auto trayIconMenu = new QMenu(this);
    bool mode24Hours = settings.value("mode24hours").toBool();
    if(mode24Hours) setMode(mode::mode24Hours);
    connect(mode24hourAction, &QAction::triggered, [this] {setMode(mode::mode24Hours);});

    bool mode12Hours = settings.value("mode12hours").toBool();
    if(mode12Hours) setMode(mode::mode12Hours);
    connect(mode12hourAction, &QAction::triggered, [this]{setMode(mode::mode12Hours);});

    bool mode5min = settings.value("mode5min").toBool();
    if(mode5min) setMode(mode::mode5Min);
    connect(mode5minAction, &QAction::triggered, [this]{setMode(mode::mode5Min);});

    auto quitAction = new QAction(tr("&Uninstall"), this);
    connect(quitAction, &QAction::triggered, this, &HiddenWindow::quitAndUnregister);

    trayIconMenu->addAction(mode24hourAction);
    trayIconMenu->addAction(mode12hourAction);
    trayIconMenu->addAction(mode5minAction);
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

void HiddenWindow::setMode(mode arg)
{
    switch(arg) {
    case mode::mode24Hours:
        iconGenerator = generate24hourIconFromTime;
        break;
    case mode::mode12Hours:
        iconGenerator = generate12hourIconFromTime;
        break;
    case mode::mode5Min:
        iconGenerator = generate5minIconFromTime;
        break;
    }
    updateTrayIcon();
    settings.setValue("mode24hours", arg == mode::mode24Hours);
    settings.setValue("mode12hours", arg == mode::mode12Hours);
    settings.setValue("mode5min", arg == mode::mode5Min);
}
