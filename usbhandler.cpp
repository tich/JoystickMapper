#include "usbhandler.h"

USBHandler::USBHandler() {
    context = NULL;
    if(libusb_init(&context) != 0) {
        context = NULL;
    }
}

USBHandler::~USBHandler() {
    if(context) {
        libusb_exit(context);
    }

    for(size_t i = 0; i < serials.size(); i++) {
        delete [] serials[i];
    }

    for(size_t i = 0; i < handles.size(); i++) {
        libusb_release_interface(handles[i], 0);
        libusb_close(handles[i]);
    }
}

libusb_device_handle *USBHandler::find_new_device(uint16_t idVendor, uint16_t idProduct) {
    //Discover devices
    libusb_device_handle *handle = NULL;
    libusb_device **list;
    ssize_t cnt = libusb_get_device_list(NULL, &list);
    ssize_t i = 0;
    int err = 0;
    if (cnt < 0) {
        return NULL;
    }

    for (i = 0; i < cnt; i++) {
        libusb_device *device = list[i];
        if (is_requested_device(device, idVendor, idProduct)) {
            err = libusb_open(device, &handle);
            if (err) {
                fprintf(stderr, "Error opening device: %d\n", err);
                continue;
            }

            //see if already in list
            if(is_already_open(handle)) {
                libusb_close(handle);
                handle = NULL;
            } else {
                break;
            }
        }
    }
    libusb_free_device_list(list, 1);

    if(handle) {
        handles.push_back(handle);
    }

    return handle;
}

bool USBHandler::is_requested_device(libusb_device *dev, uint16_t idVendor, uint16_t idProduct) {
    libusb_device_descriptor desc;
    if(libusb_get_device_descriptor(dev, &desc) != 0) {
        fprintf(stderr, "Failed to get device descriptor\n");
        return false;
    }

    if((desc.idVendor == idVendor) && (desc.idProduct == idProduct)) {
        return true;
    }

    return false;
}

bool USBHandler::is_already_open(libusb_device_handle *handle) {
    unsigned char *serial = new unsigned char(11);

    memset(serial, 0, 11);

    if(libusb_get_string_descriptor_ascii(handle, 0x03, serial, 11)) {
        fprintf(stderr, "Error reading device serial number.\n");
        return true;
    }

    for(size_t i = 0; i < serials.size(); i++) {
        if(strncmp((const char *) serial, (const char *) serials[i], 11) == 0) {
            return true;
        }
    }

    serials.push_back(serial);
    return false;
}

int USBHandler::write_to_device(libusb_device_handle *handle, unsigned char *data, int length, unsigned int timeout) {
    if(!handle || !data) {
        return -1;
    }

    if(length <= 0) {
        return 0;
    }

    int size = 0;
    if(libusb_bulk_transfer(handle, WRITE_ENDPOINT, data, length, &size, timeout) != 0) {
        fprintf(stderr, "Error writing to device\n");
        return -1;
    }

    return size;
}

int USBHandler::read_from_device(libusb_device_handle *handle, unsigned char *data, int length, unsigned int timeout) {
    if(!handle || !data) {
        return -1;
    }

    if(length <= 0) {
        return 0;
    }

    int size = 0;
    if(libusb_bulk_transfer(handle, READ_ENDPOINT, data, length, &size, timeout) != 0) {
        fprintf(stderr, "Error reading from device\n");
        return -1;
    }

    return size;
}

void USBHandler::close_device(libusb_device_handle *handle) {
    if(!handle) {
        return;
    }

    libusb_release_interface(handle, 0);
    libusb_close(handle);

    vector<libusb_device_handle *>::iterator it;
    for(it = handles.begin(); it != handles.end(); it++) {
        if(*it == handle) {
            handles.erase(it);
            break;
        }
    }
}

bool USBHandler::init_device(libusb_device_handle *handle) {
    if(!handle) {
        return false;
    }

    if(libusb_set_configuration(handle, 1)) {
        fprintf(stderr, "Error setting device configuration\n");
        return false;
    }

    if(libusb_claim_interface(handle, 0)) {
        fprintf(stderr, "Error claiming interface\n");
        return false;
    }

    return true;
}

bool USBHandler::check_libusb_version() {
    const libusb_version *version = libusb_get_version();
    if(!version) {
        return false;
    }

    if((version->major < LIBUSB_TARGET_MAJOR) || (version->micro < LIBUSB_TARGET_MICRO)) {
        return false;
    }

    return true;
}
