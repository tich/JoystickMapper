#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <libusb.h>

#include "common.h"
#include "usbhandler.h"
#include "wireddevicehandler.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private:
    void discover_wired_devices();

private:
    Ui::MainWindow *ui;
    USBHandler *usbhandler;
    QVector<WiredDeviceHandler *> wired_devices;

};

#endif // MAINWINDOW_H
