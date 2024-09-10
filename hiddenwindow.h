#ifndef HIDDENWINDOW_H
#define HIDDENWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QApplication>
#include <QSettings>

namespace Ui {
class MainWindow;
}

class HiddenWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit HiddenWindow(QWidget *parent = nullptr);

private:
    Ui::MainWindow *ui;
    void registerForStartup();
    void quitAndUnregister();
    void updateTrayIcon(QString reason);
    enum class mode: int {mode24Hours, mode12Hours, mode5Min, mode5MinSquare};
    mode currentMode = mode::mode24Hours;
    bool displayDate = false;
    void setMode(mode arg);
    void iconActivated(QSystemTrayIcon::ActivationReason reason);

    QSystemTrayIcon* trayIcon;
    QTimer timer;
    QSettings settings, settingsRunOnStartup;

signals:
};

#endif // HIDDENWINDOW_H
