#include <reg51.h>
#include <intrins.h>
#define STEPPER P3
#define BCD_OUT P2
// Floor button inputs
sbit FLOOR0 = P1^3;
sbit FLOOR1 = P1^2;
sbit FLOOR2 = P1^1;
sbit FLOOR3 = P1^0;
sbit EMERGENCY = P1^7; // Emergency stop button
// Outputs
sbit DOOR_MOTOR1 = P1^4;  // Door motor control pin 1
sbit DOOR_MOTOR2 = P1^5;  // Door motor control pin 2
// LED indicators
sbit EMERGENCY_LED = P2^5; // Blinking LED for emergency
sbit GREEN_LED     = P2^6; // Green LED for door open
sbit RED_LED       = P2^7; // Red LED for door closed
unsigned char steps[] = {0x09, 0x0C, 0x06, 0x03};
unsigned char current_floor = 0;
unsigned char floor_display = 0;
bit dir;
unsigned int step_count = 0;
unsigned int steps_per_floor = 200;
unsigned char step_index = 0;
unsigned int total_steps = 0;
bit emergency_stop = 0;
// ~1ms delay
void delay(unsigned int ms) {
    unsigned int i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < 1275; j++);
}
// Open door using DC motor
void open_door() {
    unsigned int i;
    GREEN_LED = 1;  // Turn ON green LED
    RED_LED = 0;    // Turn OFF red LED
    for (i = 0; i < 120; i++) {
        if (emergency_stop) break;
        DOOR_MOTOR1 = 1;
        DOOR_MOTOR2 = 0;
        delay(10);
    }
    DOOR_MOTOR1 = 0;
    DOOR_MOTOR2 = 0;
}
// Close door using DC motor
void close_door() {
    unsigned int i;
    GREEN_LED = 0;  // Turn OFF green LED
    RED_LED = 1;    // Turn ON red LED
    for (i = 0; i < 120; i++) {
        if (emergency_stop) break;
        DOOR_MOTOR1 = 0;
        DOOR_MOTOR2 = 1;
        delay(10);
    }
    DOOR_MOTOR1 = 0;
    DOOR_MOTOR2 = 0;
}
// Stepper motor control
void step_one(bit direction, unsigned char step_index_local) {
    if (direction)
        STEPPER = steps[step_index_local];
    else
        STEPPER = steps[3 - step_index_local];
    delay(5);
}
// Elevator movement
void go_to_floor(unsigned char target_floor) {
    if (target_floor == current_floor) return;
    total_steps = (target_floor > current_floor ?
                   target_floor - current_floor :
                   current_floor - target_floor) * steps_per_floor;
    dir = (target_floor > current_floor);
    step_index = 0;
    step_count = 0;
    floor_display = current_floor;
    while (step_count < total_steps) {
        if (EMERGENCY == 0) {
            emergency_stop = 1;
            break;
        }
        step_one(dir, step_index);
        step_index = (step_index + 1) % 4;
        step_count++;
        if ((step_count % steps_per_floor) == 0) {
            floor_display = dir ? floor_display + 1 : floor_display - 1;
            BCD_OUT = floor_display;
        }
    }
    if (!emergency_stop) {
        current_floor = target_floor;
        STEPPER = 0x00;
        // Arrival actions
        BCD_OUT = current_floor;
        open_door();
        delay(1000); // 1-second delay between open and close
        close_door();
} 
else {
        // Emergency occurred mid-travel
        current_floor = floor_display;
        BCD_OUT = current_floor;
        STEPPER = 0x00;
    }
}
void main() {
    BCD_OUT = 0x00;
    DOOR_MOTOR1 = 0;
    DOOR_MOTOR2 = 0;
    GREEN_LED = 0;
    RED_LED = 1;       // Assume door is initially closed
    EMERGENCY_LED = 0;
    emergency_stop = 0;
    while (1) {
        // Emergency mode: blink LED and stop everything
        if (EMERGENCY == 0) {
            emergency_stop = 1;
            STEPPER = 0x00;
            DOOR_MOTOR1 = 0;
            DOOR_MOTOR2 = 0;
            // Blink emergency LED until emergency is released
            while (EMERGENCY == 0) {
                EMERGENCY_LED = 1;
                delay(200);
                EMERGENCY_LED = 0;
                delay(200);
            }
            emergency_stop = 0;
            EMERGENCY_LED = 0;  // Stop blinking after emergency clears
            continue;
        }
        emergency_stop = 0;
        // Floor button handling
        if (FLOOR0 == 0) { delay(20); if (FLOOR0 == 0) { go_to_floor(0); while (FLOOR0 == 0); } }
        if (FLOOR1 == 0) { delay(20); if (FLOOR1 == 0) { go_to_floor(1); while (FLOOR1 == 0); } }
        if (FLOOR2 == 0) { delay(20); if (FLOOR2 == 0) { go_to_floor(2); while (FLOOR2 == 0); } }
        if (FLOOR3 == 0) { delay(20); if (FLOOR3 == 0) { go_to_floor(3); while (FLOOR3 == 0); } }
        BCD_OUT = current_floor;
    }
}