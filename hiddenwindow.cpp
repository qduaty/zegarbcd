#include "hiddenwindow.h"
#include "ui_mainwindow.h"
#include <QAbstractNativeEventFilter>
#include <windows.h>
#include <boost/signals2/signal.hpp>
#include "generators.h"
#include <QMenu>
#include <wtsapi32.h>
#include <QDate>
#include <QRadioButton>
#include <QSpinBox>
#include <QLineEdit>
#include <QSlider>
#include <QCheckBox>

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
    connect(ui->pixelColorBackground, &ColorToolButton::colorChanged, this, &HiddenWindow::onColorChanged);
    connect(ui->pixelColorOn, &ColorToolButton::colorChanged, this, &HiddenWindow::onColorChanged);
    connect(ui->pixelColorOff, &ColorToolButton::colorChanged, this, &HiddenWindow::onColorChanged);
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
        auto iconGenerator = displayDate ? generate4x3IconFromDate2 : iconGenerators[static_cast<int>(currentMode)];
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
    saveSettings();
    QMainWindow::hideEvent(event);
}

void HiddenWindow::onColorChanged(QColor newValue)
{
    saveSettings();
    updateTrayIcon("colors");
}
