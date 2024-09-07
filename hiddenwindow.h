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
    void updateTrayIcon();;

    QSystemTrayIcon* trayIcon;
    QTimer timer;


signals:
};

#endif // HIDDENWINDOW_H
