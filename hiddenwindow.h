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
    enum class mode: int {mode24Hours, mode12Hours, mode5Min, mode5MinSquare};
    mode currentMode = mode::mode24Hours;
    void setMode(mode arg);

    QSystemTrayIcon* trayIcon;
    QTimer timer;
    QSettings settings, settingsRunOnStartup;

signals:
};

#endif // HIDDENWINDOW_H
