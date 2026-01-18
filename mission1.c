#include <stdio.h>
#include <stdbool.h>

typedef enum {
    STATE_INIT          = 0b0000,
    STATE_IDLE          = 0b0001,
    STATE_TESTING       = 0b0010,
    STATE_READY         = 0b0011,
    STATE_LAUNCH        = 0b0100,
    STATE_MAX_HEIGHT    = 0b0101,
    STATE_DESCENT       = 0b0110,
    STATE_PARA_DEPLOY   = 0b0111,
    STATE_LANDED        = 0b1000,
    STATE_RECOVERY      = 0b1001,
    STATE_SHUT_DOWN     = 0b1010
} State;

typedef struct {
    double latitude, longitude, altitude;
    struct { double s, m, h; } time;
    struct { double x, y, z; } acceleration;
    double pressure, temperature;
} SensorData;

typedef struct {
    bool ok;
    bool cmd_test;
    bool tests_passed;
    bool reboot;
    bool accl_launch;
    bool max_alt_reached;
    bool descent_cond;
    bool altitude_200m;
    bool pressure_stable;
    bool recovered;
    bool power_off;
} LogicalInputs;

typedef struct {
    int pwr;
    int gun;
    int tel;
    int beacon;
    int location;
    int data;
    int vid_rec;
    int para_trigger;
    int test_mode;
    int ready;
} Outputs;

SensorData update_values(int cycle) {
    SensorData s = {0};
    return s;
}

State update_fsm(State current_state, LogicalInputs in, Outputs *out) {
    
    switch (current_state) {
        case STATE_INIT:
            out->pwr = 1;
            out->gun = 1;
            out->tel = 1;
            out->beacon = 0;
            out->location = 0;
            out->data = 0;
            out->vid_rec = 0;
            out->para_trigger = 0;
            out->test_mode = 0;
            out->ready = 0;
            
            if (in.ok) return STATE_IDLE;
            break;

        case STATE_IDLE:
            out->pwr = 0;
            out->gun = 0;
            out->tel = 1;
            out->beacon = 0;
            out->location = 0;
            out->data = 0;
            out->vid_rec = 0;
            out->para_trigger = 0;
            out->test_mode = 0;
            out->ready = 0;

            if (in.cmd_test) return STATE_TESTING;
            break;

        case STATE_TESTING:
            out->pwr = 1;
            out->gun = 1;
            out->tel = 1;
            out->beacon = 0;
            out->location = 0;
            out->data = 0;
            out->vid_rec = 0;
            out->para_trigger = 0;
            out->test_mode = 1;
            out->ready = 0;

            if (in.tests_passed) return STATE_READY;
            break;

        case STATE_READY:
            out->pwr = 0;
            out->gun = 0;
            out->tel = 1;
            out->beacon = 0;
            out->location = 0;
            out->data = 0;
            out->vid_rec = 0;
            out->para_trigger = 0;
            out->test_mode = 0;
            out->ready = 1;

            if (in.reboot) return STATE_INIT;
            if (in.accl_launch) return STATE_LAUNCH;
            break;

        case STATE_LAUNCH:
            out->pwr = 1;
            out->gun = 0;
            out->tel = 1;
            out->beacon = 1;
            out->location = 0;
            out->data = 0;
            out->vid_rec = 0;
            out->para_trigger = 0;
            out->test_mode = 0;
            out->ready = 0;

            if (in.max_alt_reached) return STATE_MAX_HEIGHT;
            break;

        case STATE_MAX_HEIGHT:
            out->pwr = 1;
            out->gun = 1;
            out->tel = 1;
            out->beacon = 1;
            out->location = 0;
            out->data = 0;
            out->vid_rec = 0;
            out->para_trigger = 0;
            out->test_mode = 0;
            out->ready = 0;

            if (in.descent_cond) return STATE_DESCENT;
            break;

        case STATE_DESCENT:
            out->pwr = 1;
            out->gun = 1;
            out->tel = 1;
            out->beacon = 1;
            out->location = 0;
            out->data = 1;
            out->vid_rec = 1;
            out->para_trigger = 0;
            out->test_mode = 0;
            out->ready = 0;

            if (in.altitude_200m) return STATE_PARA_DEPLOY;
            break;

        case STATE_PARA_DEPLOY:
            out->pwr = 1;
            out->gun = 1;
            out->tel = 1;
            out->beacon = 1;
            out->location = 1;
            out->data = 1;
            out->vid_rec = 1;
            out->para_trigger = 1;
            out->test_mode = 0;
            out->ready = 0;

            if (in.pressure_stable) return STATE_LANDED;
            break;

        case STATE_LANDED:
            out->pwr = 1;
            out->gun = 0;
            out->tel = 1;
            out->beacon = 1;
            out->location = 1;
            out->data = 0;
            out->vid_rec = 0;
            out->para_trigger = 0;
            out->test_mode = 0;
            out->ready = 0;

            if (in.recovered) return STATE_RECOVERY;
            break;

        case STATE_RECOVERY:
            out->pwr = 1;
            out->gun = 0;
            out->tel = 0;
            out->beacon = 0;
            out->location = 0;
            out->data = 0;
            out->vid_rec = 0;
            out->para_trigger = 0;
            out->test_mode = 0;
            out->ready = 0;

            if (in.power_off) return STATE_SHUT_DOWN;
            break;

        case STATE_SHUT_DOWN:
            out->pwr = 0;
            out->gun = 0;
            out->tel = 0;
            out->beacon = 0;
            out->location = 0;
            out->data = 0;
            out->vid_rec = 0;
            out->para_trigger = 0;
            out->test_mode = 0;
            out->ready = 0;
            break;
    }
    return current_state;
}

int main() {
    State current_state = STATE_INIT;
    Outputs out;
    LogicalInputs in = {0};
    
    for (int i = 0; i < 15; i++) {
        update_values(i);

        switch (current_state) {
            case STATE_INIT:        in.ok = true; break;
            case STATE_IDLE:        in.cmd_test = true; break;
            case STATE_TESTING:     in.tests_passed = true; break;
            case STATE_READY:       in.accl_launch = true; break;
            case STATE_LAUNCH:      in.max_alt_reached = true; break;
            case STATE_MAX_HEIGHT:  in.descent_cond = true; break;
            case STATE_DESCENT:     in.altitude_200m = true; break;
            case STATE_PARA_DEPLOY: in.pressure_stable = true; break;
            case STATE_LANDED:      in.recovered = true; break;
            case STATE_RECOVERY:    in.power_off = true; break;
            default: break;
        }

        current_state = update_fsm(current_state, in, &out);
        
        printf("State: %02d | Outputs -> PWR:%d GUN:%d TEL:%d BCN:%d LOC:%d DATA:%d VID:%d PARA:%d TEST:%d RDY:%d\n", 
               current_state, out.pwr, out.gun, out.tel, out.beacon, 
               out.location, out.data, out.vid_rec, out.para_trigger, 
               out.test_mode, out.ready);

        if (current_state == STATE_SHUT_DOWN) break;
    }

    return 0;
}
