#ifndef USBHANDLER_H
#define USBHANDLER_H

#include <vector>
#include <libusb.h>
#include <stdio.h>

#include "common.h"

#define READ_ENDPOINT   0x80 | 3
#define WRITE_ENDPOINT  0x02

using namespace std;

class USBHandler
{
public:
    USBHandler();
    ~USBHandler();

    static bool check_libusb_version();

    libusb_device_handle *find_new_device(uint16_t idVendor, uint16_t idProduct);
    void close_device(libusb_device_handle *handle);

    bool init_device(libusb_device_handle *handle);

    int write_to_device(libusb_device_handle *handle, unsigned char *data, int length, unsigned int timeout = 10);
    int read_from_device(libusb_device_handle *handle, unsigned char *data, int length, unsigned int timeout = 10);

private:
    bool is_requested_device(libusb_device *dev, uint16_t idVendor, uint16_t idProduct);
    bool is_already_open(libusb_device_handle *handle);
private:
    libusb_context *context;
    vector<unsigned char *> serials;
    vector<libusb_device_handle *> handles;
};

#endif // USBHANDLER_H
