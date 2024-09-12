#include "hiddenwindow.h"
#include "ui_mainwindow.h"
#include <windows.h>
#include <wtsapi32.h>
#include <boost/signals2/signal.hpp>
#include <QAbstractNativeEventFilter>
#include <QMenu>
#include <QPainter>
#include <QDate>
#include <QCheckBox>
#include <QRadioButton>
#include <QLineEdit>
#include <QSlider>
#include <QWindow>
#include <QSpinBox>

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

QIcon generateIcon(int* digits, QSize dataSize)
{
    QSettings settings("HKEY_CURRENT_USER\\Software\\qduaty\\zegarbcd", QSettings::NativeFormat);
    settings.beginGroup("Preferences");
    auto gridColor = ColorToolButton::colorFromInt(settings.value("pixelColorBackground", 0x505050).toInt());
    auto onColor = ColorToolButton::colorFromInt(settings.value("pixelColorOn", 0xffffff).toInt());
    auto offColor = ColorToolButton::colorFromInt(settings.value("pixelColorOff", 0x000000).toInt());
    int iconSize = settings.value("iconSize", 256).toInt();
    QPixmap pix(iconSize, iconSize);
    QPainter paint(&pix);
    paint.fillRect(0, 0, iconSize, iconSize, gridColor);
    for(int x = 0; x < dataSize.width(); x++)
        for(int y = 0; y < dataSize.height(); y++)
        {
            int xsize = iconSize / dataSize.width();
            int ysize = iconSize / dataSize.height();
            int margin = iconSize / 16;
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
    QSettings settings("HKEY_CURRENT_USER\\Software\\qduaty\\zegarbcd", QSettings::NativeFormat);
    settings.beginGroup("Preferences");
    auto gridColor = ColorToolButton::colorFromInt(settings.value("pixelColorBackground", 0x505050).toInt());
    auto onColor = ColorToolButton::colorFromInt(settings.value("pixelColorOn", 0xffffff).toInt());
    auto offColor = ColorToolButton::colorFromInt(settings.value("pixelColorOff", 0x000000).toInt());
    int iconSize = settings.value("iconSize", 64).toInt();
    QPixmap pix(iconSize, iconSize);
    QPainter paint(&pix);
    paint.fillRect(0, 0, iconSize, iconSize, gridColor);

    int digits[4];
    constexpr int sizes[4] = {2, 3, 2, 2};
    digits[0] = (tm->tm_hour / 8);
    digits[1] = (tm->tm_hour % 8);
    digits[2] = tm->tm_min / 15;
    digits[3] = (tm->tm_min % 15) / 5;

    for(int x = 0; x < 4; x++) {
        for(int y = 0; y < sizes[x]; y++)
        {
            int xsize = iconSize / 4;
            int ysize = iconSize / sizes[x];
            int xmargin = iconSize / 16;
            int ymargin = iconSize / 8;
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

void HiddenWindow::saveSettings()
{
    settings.beginGroup("Preferences");

    for(auto child: findChildren<QCheckBox*>())
        settings.setValue(child->objectName(), child->isChecked());
    for(auto child: findChildren<QRadioButton*>())
        settings.setValue(child->objectName(), child->isChecked());
    for(auto child: findChildren<QLineEdit*>())
        if(!dynamic_cast<QSpinBox*>(child->parent()) && !dynamic_cast<QDoubleSpinBox*>(child->parent()))
            settings.setValue(child->objectName(), child->text());
    for(auto child: findChildren<QSlider*>())
        settings.setValue(child->objectName(), child->value());
    for(auto child: findChildren<QSpinBox*>())
        settings.setValue(child->objectName(), child->value());
    for(auto child: findChildren<QDoubleSpinBox*>())
        settings.setValue(child->objectName(), child->value());
    for(auto child: findChildren<QTabWidget*>())
        settings.setValue(child->objectName(), child->currentIndex());
    for(auto child: findChildren<ColorToolButton*>())
        settings.setValue(child->objectName(), child->value());
    settings.endGroup();
}

void HiddenWindow::loadSettings()
{
    settings.beginGroup("Preferences");
    if(settings.value("first_time", true).toBool())
    {
        saveSettings();
        settings.setValue("first_time", false);
    }
    else
    {
        for(auto child: findChildren<QCheckBox*>())
            child->setChecked(settings.value(child->objectName(), false).toBool());
        for(auto child: findChildren<QRadioButton*>())
            child->setChecked(settings.value(child->objectName(), false).toBool());
        for(auto child: findChildren<QLineEdit*>())
            if(!dynamic_cast<QSpinBox*>(child->parent()) && !dynamic_cast<QDoubleSpinBox*>(child->parent()))
                child->setText(settings.value(child->objectName(), "").toString());
        for(auto child: findChildren<QSlider*>())
            child->setValue(settings.value(child->objectName(), 0).toInt());
        for(auto child: findChildren<QSpinBox*>())
            child->setValue(settings.value(child->objectName(), 0).toInt());
        for(auto child: findChildren<QDoubleSpinBox*>())
            child->setValue(settings.value(child->objectName(), 0).toDouble());
        for(auto child: findChildren<QTabWidget*>())
            child->setCurrentIndex(settings.value(child->objectName(), 0).toInt());
        for(auto child: findChildren<ColorToolButton*>())
            child->setValue(settings.value(child->objectName(), 0).toInt());
    }
    settings.endGroup();
}

void HiddenWindow::showEvent(QShowEvent *event)
{
    loadSettings();
    QMainWindow::showEvent(event);
}

void HiddenWindow::hideEvent(QHideEvent *event)
{
    qDebug() << __func__;
    saveSettings();
    updateTrayIcon("colors");
    QMainWindow::hideEvent(event);
}
