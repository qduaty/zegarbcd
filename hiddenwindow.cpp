#include "hiddenwindow.h"
#include "ui_mainwindow.h"
#include <windows.h>
#include <wtsapi32.h>
#include <boost/signals2/signal.hpp>
#include <QAbstractNativeEventFilter>
#include <QMenu>
#include <QPainter>
#include <QDate>

class SessionEventFilter : public QAbstractNativeEventFilter {
public:
    bool nativeEventFilter(const QByteArray& eventType, void* message, qintptr *) Q_DECL_OVERRIDE {
        auto msg = static_cast<MSG*>(message);
        // TODO left this code in case the clock needs a nudge also after system wake-up - remove if not needed
        if (msg->message == WM_POWERBROADCAST) {
            qDebug() << "WM_POWERBROADCAST:" << msg->wParam;
            if(msg->wParam == PBT_APMRESUMEAUTOMATIC)
                event("WM_POWERBROADCAST");
        }
        else if (msg->message == WM_WTSSESSION_CHANGE)
        {
            // qDebug() << "WM_WTSSESSION_CHANGE:" << msg->wParam << ' ' << msg->lParam;
            if(msg->wParam == WTS_SESSION_UNLOCK)
                event("WM_WTSSESSION_CHANGE");
        }
        return false;
    }
    boost::signals2::signal<void(QString)> event;
};

constexpr size_t imageSize = 64;

constexpr int gray = 72;
constexpr QColor gridColor(gray,gray,gray,255);
constexpr QColor onColor(255,255,255,255);
constexpr QColor offColor(0,0,0,255);

QIcon generateIcon(int* digits, QSize dataSize)
{
    QPixmap pix(imageSize, imageSize);
    QPainter paint(&pix);
    paint.fillRect(0, 0, imageSize, imageSize, gridColor);
    for(int x = 0; x < dataSize.width(); x++)
        for(int y = 0; y < dataSize.height(); y++)
        {
            const int xsize = imageSize / dataSize.width();
            const int ysize = imageSize / dataSize.height();
            constexpr int margin = imageSize / 16;
            int xpos = x * xsize + margin / 2;
            int ypos = y * ysize + margin / 2;
            QColor color = (digits[x] >> (dataSize.height() - 1 - y)) & 0x1 ? onColor : offColor;
            paint.fillRect(xpos, ypos, xsize - margin, ysize - margin, color);
        }

    return pix;
}

QIcon generate24hourIconFromTime(struct tm * tm) {

    int digits[4];
    digits[0] = tm->tm_hour / 10;
    digits[1] = tm->tm_hour % 10;
    digits[2] = tm->tm_min / 10;
    digits[3] = tm->tm_min % 10;
    return generateIcon(digits, {4, 4});
}

QIcon generate12hourIconFromTime(struct tm * tm) {
    int digits[3];
    digits[0] = (tm->tm_hour % 12);
    if(digits[0] == 0)
        digits[0] = 12;
    digits[1] = tm->tm_min / 10;
    digits[2] = tm->tm_min % 10;
    return generateIcon(digits, {3, 4});
}

/// weekday (1-7); monthday (vert + top to right, 1-31); month (bottom right square, 1-12)
QIcon generate3x4IconFromDate(struct tm * tm) {
    int digits[4];
    digits[0] = (tm->tm_wday);
    digits[1] = tm->tm_mday & 0b111;
    digits[2] = (tm->tm_mon + 1) & 0b11 | (tm->tm_mday >> 1) & 0b100;
    digits[3] = ((tm->tm_mon + 1) >> 2) | (tm->tm_mday >> 2) & 0b100;
    return generateIcon(digits, {4, 3});
}

/// weekday (top row: 1-7); monthday (vert + top to right, 1-31); month (bottom right square, 1-12)
QIcon generate4x3IconFromDate(struct tm * tm) {
    int digits[3];
    digits[0] = tm->tm_mday & 0b111 | ((tm->tm_wday & 0b1) << 3);
    digits[1] = (tm->tm_mon + 1) & 0b11 | (tm->tm_mday >> 1) & 0b100 | ((tm->tm_wday & 0b10) << 2);
    digits[2] = ((tm->tm_mon + 1) >> 2) | (tm->tm_mday >> 2) & 0b100 | ((tm->tm_wday & 0b100) << 1);
    return generateIcon(digits, {3, 4});
}

QIcon generate5minIconFromTime(struct tm * tm) {
    QPixmap pix(imageSize, imageSize);
    QPainter paint(&pix);
    paint.fillRect(0, 0, imageSize, imageSize, gridColor);

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
            int xpos = x * xsize + xmargin / 2;
            int ypos = y * ysize + ymargin / sizes[x];
            QColor color = (digits[x] >> (sizes[x] - 1 - y)) & 0x1 ? onColor : offColor;
            paint.fillRect(xpos, ypos, xsize - xmargin, ysize - ymargin * 2 / sizes[x], color);
        }
    }

    return pix;
}

QIcon generate5min3x3IconFromTime(struct tm * tm) {
    int digits[3];
    auto hour = tm->tm_hour % 8;
    digits[0] = tm->tm_hour / 8 | (hour & 0b1) << 2;
    digits[1] = tm->tm_min / 15 | (hour & 0b10) << 1;
    digits[2] = (tm->tm_min % 15) / 5 | (hour & 0b100);
    return generateIcon(digits, {3, 3});
}

std::function<QIcon (struct tm * tm)>iconGenerators[]{generate24hourIconFromTime, generate12hourIconFromTime, generate5minIconFromTime, generate5min3x3IconFromTime};
constexpr const char *actionTexts[] = {"&24 hours", "&12 hours", "&5 min (1/3 day, hour, quarter, 5 min)", "5 &min (horz hour, vert 1/3 day, quarter, 5 min)"};
constexpr const char *settingNames[] = {"mode24hours", "mode12hours", "mode5min", "mode5minSquare"};

HiddenWindow::HiddenWindow(QWidget *parent):
    QMainWindow{parent},
    ui(new Ui::MainWindow),
    settings("HKEY_CURRENT_USER\\Software\\qduaty\\zegarbcd", QSettings::NativeFormat),
    settingsRunOnStartup("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat),
    trayIcon(new QSystemTrayIcon(this))
{
    ui->setupUi(this);
    auto trayIconMenu = new QMenu(this);

    for(int i = 0; i < std::size(settingNames); i++)
    {
        auto action = new QAction(actionTexts[i], this);
        if(settings.value(settingNames[i]).toBool()) setMode(static_cast<mode>(i));
        connect(action, &QAction::triggered, [this, i] {setMode(static_cast<mode>(i));});
        trayIconMenu->addAction(action);
    }

    auto quitAction = new QAction(tr("&Uninstall"), this);
    connect(quitAction, &QAction::triggered, this, &HiddenWindow::quitAndUnregister);
    trayIconMenu->addAction(quitAction);
    trayIcon->setContextMenu(trayIconMenu);
    timer.setInterval(1000);
    connect(&timer, &QTimer::timeout, [this]{updateTrayIcon("timeout");});
    timer.start();
    WTSRegisterSessionNotification(reinterpret_cast<HWND>(winId()), NOTIFY_FOR_THIS_SESSION);
    RegisterSuspendResumeNotification(reinterpret_cast<HWND>(winId()), DEVICE_NOTIFY_WINDOW_HANDLE);
    auto myEvenfilter = new SessionEventFilter;
    myEvenfilter->event.connect([this](QString arg){updateTrayIcon(arg);});
    QApplication::instance()->installNativeEventFilter(myEvenfilter);
    connect(trayIcon, &QSystemTrayIcon::activated, this, &HiddenWindow::iconActivated);
    registerForStartup();
}

void HiddenWindow::registerForStartup() {
    settingsRunOnStartup.setValue(QApplication::applicationName(),
                                  QCoreApplication::applicationFilePath().replace('/', "\\"));
}

void HiddenWindow::quitAndUnregister() {
    settingsRunOnStartup.remove(QApplication::applicationName());
    settings.clear();
    QCoreApplication::instance()->quit();
}

void HiddenWindow::updateTrayIcon(QString reason) {
    if(trayIcon)
    {
        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        auto tm = std::localtime(&now);
        qDebug() << tm->tm_hour << tm->tm_min << tm->tm_sec << "updating icon: " << reason;
        auto iconGenerator = displayDate ? generate4x3IconFromDate : iconGenerators[static_cast<int>(currentMode)];
        trayIcon->setIcon(iconGenerator(tm));
        trayIcon->setToolTip(QLocale().toString(QDate::currentDate(), "dddd d MMMM yyyy"));
        trayIcon->show();
        if(displayDate && timer.remainingTime() > 6000)
            timer.setInterval(6000);
        else if(tm->tm_sec > 0)
            timer.setInterval((60 - tm->tm_sec) * 1000);
        else
            timer.setInterval(60000);
    }
    displayDate = false;
}

void HiddenWindow::setMode(mode arg)
{
    currentMode = arg;
    for(int i = 0; i < std::size(settingNames); i++)
        settings.setValue(settingNames[i], i == static_cast<int>(arg));
    updateTrayIcon(__func__);
}

void HiddenWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch(reason)
    {
    case QSystemTrayIcon::DoubleClick: // left click
    {
        auto iconGeometry = trayIcon->geometry();
        auto w = geometry().width();
        auto h = geometry().height();
        auto x = iconGeometry.x() + iconGeometry.width() / 2 - w / 2;
        auto y = iconGeometry.y() - h;

        setGeometry(x, y, w, h);

        if(isHidden())
            show();
        else
            hide();
    }
    break;
    case QSystemTrayIcon::Trigger:
        displayDate = !displayDate;
        updateTrayIcon("date");
        break;
    default:
    break;
    }
}
