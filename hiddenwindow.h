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
    enum class mode {mode24Hours, mode12Hours, mode5Min};
    void setMode(mode arg);

    QSystemTrayIcon* trayIcon;
    QTimer timer;
    QSettings settings, settingsRunOnStartup;
    std::function<QIcon (struct tm * tm)>iconGenerator;
    QAction *mode24hourAction, *mode12hourAction, *mode5minAction;

signals:
};

#endif // HIDDENWINDOW_H
