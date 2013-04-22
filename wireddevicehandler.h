#ifndef WIREDDEVICEHANDLER_H
#define WIREDDEVICEHANDLER_H

#include "devicehandler.h"

class WiredDeviceHandler : public DeviceHandler {

public:
    WiredDeviceHandler(QObject *parent = 0, USBHandler *usbhandler = NULL, libusb_device_handle *handle = NULL) : DeviceHandler(parent, usbhandler, handle) {}

    vector<component_info> get_controller_info();
protected:
    void send_led(LEDState state);
    void send_ff(vector<uint64_t> &uids, bool on);
    void read_update();
};

#endif // WIREDDEVICEHANDLER_H
