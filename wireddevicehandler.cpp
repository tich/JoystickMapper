#include "wireddevicehandler.h"

void WiredDeviceHandler::send_led(LEDState state) {
    unsigned char data[2];
    data[0] = COMMAND_SET_LED;
    data[1] = state;
    if(usbhandler->write_to_device(dev, data, 2, 3) != 2) {
        fprintf(stderr, "Set LED: Error writing LED state to device\n");
        return;
    }

    if(usbhandler->read_from_device(dev, data, 2, 10) != 2) {
        fprintf(stderr, "Set LED: Error reading from device\n");
        return;
    }

    if((data[0] != COMMAND_SET_LED) || (data[1] != STATUS_SUCCESS)) {
        fprintf(stderr, "Set LED: Device failed to set LED state\n");
        return;
    }

    return;
}

void WiredDeviceHandler::send_ff(vector<uint64_t> &uids, bool on) {
    int length = 3 + (uids.size() * 6); //3 bytes for command header. 6 bytes per UID
    unsigned char *data = new unsigned char[length];

    data[0] = COMMAND_FORCE_FEEDBACK;
    data[1] = on;
    data[3] = uids.size();

    for(size_t i = 0; i < uids.size(); i++) {
        uint64_t uid = uids[i];
        for(int j = 0; j < 6; j++) {
            data[(3*i) + j] = (uid >> (j * 8)) & 0xFF;
        }
    }

    if(usbhandler->write_to_device(dev, data, length, 3) != length) {
        fprintf(stderr, "Send FF: Error sending ForceFeedback info to device\n");
        delete [] data;
        return;
    }

    if(usbhandler->read_from_device(dev, data, 2, 10) != 2) {
        fprintf(stderr, "Send FF: Error reading from device\n");
        delete [] data;
        return;
    }

    if((data[0] != COMMAND_FORCE_FEEDBACK) || (data[1] != STATUS_SUCCESS)) {
        fprintf(stderr, "Send FF: Device failed to set ForceFeedback state\n");
        delete [] data;
        return;
    }

    delete [] data;

    return;
}

void WiredDeviceHandler::read_update() {
    unsigned char init_data[3];
    unsigned char *data;
    int length;
    vector<component_data> components_data;

    init_data[0] = COMMAND_GET_UPDATE_SIZE;

    if(usbhandler->write_to_device(dev, init_data, 1, 3) != 1) {
        fprintf(stderr, "Read Update: Error sending update size command to device\n");
        return;
    }

    if(usbhandler->read_from_device(dev, init_data, 3, 5) != 3) {
        fprintf(stderr, "Read Update: Error reading response from device\n");
        return;
    }

    length = init_data[2] * DATA_SIZE;

    if(length == 0) {
        return;
    }

    data = new unsigned char[length + 2];

    init_data[0] = COMMAND_GET_UPDATE;

    if(usbhandler->write_to_device(dev, init_data, 1, 3) != 1) {
        fprintf(stderr, "Read Update: Error sending update command to device\n");
        return;
    }

    if(usbhandler->read_from_device(dev, data, length + 2, 10) != (length + 2)) {
        fprintf(stderr, "Read Update: Error reading sensor data from device\n");
        delete [] data;
        return;
    }

    data = data + 2;

    for(int i = 0; i < init_data[2]; i++) {
        component_data comp;
        component_info *info;
        size_t j = 0;
        Data *strct = (Data *) &(data[i * DATA_SIZE]);

        comp.uid = 0;

        //Get component UID
        for(j = 0; j < 6; j++) {
            comp.uid |= strct->parts.uid[j] << (j * 8);
        }

        //Find component information
        for(j = 0; j < components.size(); j++) {
            if(components[j].uid == comp.uid) {
                info = &(components[j]);
                break;
            }
        }

        if(info == NULL) {
            continue;
        }

        j = 0;
        while((strct->parts.sensor_data[j] != 0) && (j < (DATA_SIZE - 8))) {
            if(strct->parts.sensor_data[j] == COMPONENT_ANALOG_STICK) {
                for(int k = 0; k < info->num_analog; k++) {
                    comp.analog.push_back(strct->parts.sensor_data[++j]);
                }
                j++;
            } else if(strct->parts.sensor_data[j] == COMPONENT_BUTTON) {
                comp.buttons = strct->parts.sensor_data[j + 1];
                j += 2;
            } else {
                //error
                break;
            }
        }

        components_data.push_back(comp);
    }

    emit component_data_received(components_data);

    delete [] (data - 2);
    return;
}

vector<component_info> WiredDeviceHandler::get_controller_info() {
    int err = 0;
    unsigned char init_data[3];
    unsigned char *data;
    int length;
    vector<component_info> components;

    init_data[0] = COMMAND_GET_INFO_SIZE;

    if((err = usbhandler->write_to_device(dev, init_data, 1, 3)) != 1) {
        fprintf(stderr, "Read Info: Error sending info size command to device %d\n", err);
        return components;
    }

    if((err = usbhandler->read_from_device(dev, init_data, 3, 1500)) != 3) {
        fprintf(stderr, "Read Info: Error reading response from device %d\n", err);
        return components;
    }

    length = init_data[2] * DATA_SIZE;

    if(length == 0) {
        this->components = components;
        return components;
    }

    data = new unsigned char[length + 2];

    init_data[0] = COMMAND_GET_INFO;

    if((err = usbhandler->write_to_device(dev, init_data, 1, 3)) != 1) {
        fprintf(stderr, "Read Info: Error sending info command to device %d\n", err);
        return components;
    }

    if((err = usbhandler->read_from_device(dev, data, length + 2, 10)) != (length + 2)) {
        fprintf(stderr, "Read Info: Error reading sensor info from device %d\n", err);
        delete [] data;
        return components;
    }

    data = data + 2;    //Skip first two bytes

    for(int i = 0; i < init_data[2]; i++) {
        component_info comp;
        Data *strct = (Data *) &(data[i * DATA_SIZE]);
        for(int j = 0; j < DATA_SIZE - 8; j += 2) {
            bool end = false;
            switch(strct->parts.sensor_data[j]) {
            case COMPONENT_ANALOG_STICK:
                comp.num_analog = strct->parts.sensor_data[j + 1];
                break;
            case COMPONENT_BUTTON:
                comp.num_buttons = strct->parts.sensor_data[j + 1];
                break;
            case COMPONENT_FORCEFEEDBACK:
                comp.num_forcefeedback = strct->parts.sensor_data[j + 1];
                break;
            default:
                end = true;
                break;
            }

            if(end) {
                break;
            }
        }

        comp.uid = 0;
        for(int j = 0; j < 6; j++) {
            comp.uid |= strct->parts.uid[j] << (j * 8);
        }

        components.push_back(comp);
    }

    delete [] (data - 2);

    this->components = components;
    return components;
}
