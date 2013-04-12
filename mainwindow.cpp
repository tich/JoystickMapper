#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    if(!USBHandler::check_libusb_version()) {
        fprintf(stderr, "The version of libusbx on your system is too old.\n");
        fprintf(stderr, "This program requires libusbx version %d.%d or greater\n", LIBUSB_TARGET_MAJOR, LIBUSB_TARGET_MICRO);
        exit(EXIT_FAILURE);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
