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

    usbhandler = new USBHandler();
    discover_wired_devices();

    for(int i = 0; i < wired_devices.size(); i++) {
        vector<component_info> info = wired_devices[i]->get_controller_info();
        printf("This device has %d components\n", info.size());
        for(size_t j = 0; j < info.size(); j++) {
            printf("Component %llu has:\n", info[j].uid);
            printf("\t%d Analog Sticks\n", info[j].num_analog);
            printf("\t%d buttons\n", info[j].num_buttons);
            printf("\t%d ForceFeedback rotors\n\n", info[j].num_forcefeedback);
        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;

    for(int i = 0; i < wired_devices.size(); i++) {
        delete wired_devices[i];
    }

    delete usbhandler;
}

void MainWindow::discover_wired_devices() {
    libusb_device_handle *handle = NULL;
    WiredDeviceHandler *wired_handler;

    while((handle = usbhandler->find_new_device(0x03EB, 0x204E)) != NULL) {
        usbhandler->init_device(handle);
        wired_handler = new WiredDeviceHandler(this, usbhandler, handle);
        //connect(wired_handler, &WiredDeviceHandler::component_data_received, this, &MainWindow::data_received);
        connect(wired_handler, &WiredDeviceHandler::finished, wired_handler, &QObject::deleteLater);
        wired_devices.push_back(wired_handler);
    }
}
