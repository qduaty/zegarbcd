#ifndef HIDDENWINDOW_H
#define HIDDENWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QApplication>
#include <QSettings>

class HiddenWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit HiddenWindow(QWidget *parent = nullptr);

private:

    void registerForStartup();
    void quitAndUnregister();
    void updateTrayIcon();
    void set12HourMode(bool arg);

    QSystemTrayIcon* trayIcon;
    QTimer timer;
    QSettings settings, settingsRunOnStartup;
    std::function<QIcon (struct tm * tm)>iconGenerator;

signals:
};

#endif // HIDDENWINDOW_H
