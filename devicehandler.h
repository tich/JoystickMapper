#ifndef DEVICEHANDLER_H
#define DEVICEHANDLER_H

#include <QThread>
#include <QMutex>
#include <stdint.h>
#include <vector>

#include "common.h"
#include "usbhandler.h"

using namespace std;

#define COMPONENT_ANALOG_STICK	1
#define COMPONENT_BUTTON		2
#define COMPONENT_FORCEFEEDBACK 3

#define DATA_SIZE (1 + 6 + 1 + 6) //1 byte for message direction,6 bytes for UID, 1 byte for CRC, and 6 bytes for data

typedef union {
    struct {
        unsigned char direction;
        unsigned char uid[6];
        unsigned char sensor_data[DATA_SIZE - 8];	//The rest of the stuff is sensor data
        unsigned char crc;
    } parts;

    unsigned char data[DATA_SIZE];
} Data;

typedef enum {
    COMMAND_GET_INFO = 0,
    COMMAND_GET_INFO_SIZE,
    COMMAND_GET_UPDATE,
    COMMAND_GET_UPDATE_SIZE,
    COMMAND_FORCE_FEEDBACK,
    COMMAND_SET_LED
} DeviceCommands;

typedef enum {
    STATUS_SUCCESS = 0,
    STATUS_ERROR,
    STATUS_UNKNOWN
} CommandStatus;

typedef struct {
    uint64_t uid;
    uint8_t num_analog;
    uint8_t num_buttons;
    uint8_t num_forcefeedback;
} component_info;

typedef struct {
    uint64_t uid;
    vector<uint8_t> analog;
    uint8_t buttons;
} component_data;

typedef enum {
    SET_LED,
    SET_FF
} Operation;

typedef struct {
    Operation op;
    union {
        LEDState state;
        bool on_off;
    };
    vector<uint64_t> uids;
} operation_t;

class DeviceHandler : public QThread {
    Q_OBJECT
public:
    explicit DeviceHandler(QObject *parent = 0, USBHandler *usbhandler = NULL, libusb_device_handle *handle = NULL) : QThread(parent), usbhandler(usbhandler), dev(handle), stopping(false) {}

    void run() {
        vector<operation_t>::iterator it;
        stopping = false;
        while(!stopping) {
            mutex.lock();
            for(it = pending_operations.begin(); it != pending_operations.end();) {
                operation_t op = *it;
                if(op.op == SET_LED) {
                    send_led(op.state);
                } else if(op.op == SET_FF) {
                    send_ff(op.uids, op.on_off);
                }
                pending_operations.erase(it);
            }
            mutex.unlock();

            read_update();
        }
    }

    void set_device_led(LEDState state) {
        operation_t op;
        op.op = SET_LED;
        op.state = state;
        add_operation(op);
    }

    void set_forcefeedback(vector<uint64_t> uids, bool on) {
        operation_t op;
        op.op = SET_FF;
        op.on_off = on;
        op.uids = uids;
        add_operation(op);
    }

    virtual vector<component_info> get_controller_info() = 0;

    void stop() { stopping = true; }

signals:
    void component_data_received(vector<component_data>);

private:
    void add_operation(operation_t &op) {
        mutex.lock();
        pending_operations.push_back(op);
        mutex.unlock();
    }

protected:
    virtual void send_led(LEDState state) = 0;
    virtual void send_ff(vector<uint64_t> &uids, bool on) = 0;
    virtual void read_update() = 0;

protected:
    USBHandler *usbhandler;
    libusb_device_handle *dev;

    vector<component_info> components;

private:
    vector<operation_t> pending_operations;
    QMutex mutex;
    bool stopping;
};

#endif // DEVICEHANDLER_H
