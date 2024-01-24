#include "ShuckOS.h"

/*
 * Run ShuckOS both on Serial and Bluetooth(BluetoothSerial)
*/

//    With Bluetooth: 84.2% = 1103509 bytes / 1310720 bytes = 1077,65 KiB = 1,052 MiB
// Without Bluetooth: 23.9% =  312657 bytes / 1310720 bytes =  305,33 KiB = 0,298 MiB
// --------------------------------------------------------------------
//        Difference: 60.3%    790852 bytes                 =  772,32 KiB = 0,754 MiB
// Whyyy?

#include "BluetoothSerial.h"
BluetoothSerial SerialBT;

void BT_console_task(void* parameter) {
    SerialBT.begin("ShuckOS");
    
    ShuckKeyboardSimple shuck_keyboard_simple(&SerialBT);
    ShuckDisplaySimple  shuck_display_simple(&SerialBT);
    
    ShuckConsole shuck_BT_console(
        shuck_keyboard_simple,
        shuck_display_simple,
        SerialBT
    );
    
    const char BT_password[] = "shuckBT";
    while (true) {
        if (SerialBT.available()) {
            String user_pass = SerialBT.readStringUntil('\n');
            if (user_pass.equals(BT_password)) {
                ShuckCMD::Run::cmd("shuck", shuck_BT_console);
            }
            else {
                SerialBT.print("Please provide password!\n");
            }
        }
        delay(1);
    }
    
    vTaskDelete(NULL);
}


ShuckKeyboardTerminal* shuck_keyboard_terminal;
ShuckDisplayTerminal*  shuck_display_terminal;
ShuckConsole* shuck_terminal_console;

void setup() {
    Serial.begin(115200);
    
    shuck_keyboard_terminal = new ShuckKeyboardTerminal(&Serial);
    shuck_display_terminal  = new ShuckDisplayTerminal(&Serial);
    
    shuck_terminal_console =
    new ShuckConsole(
        *shuck_keyboard_terminal,
        *shuck_display_terminal,
        Serial
    );

    xTaskCreate(
        BT_console_task, // Task function.
        "BT console",    // String with name of task.
        1024*8,          // Stack size in bytes. Same as loop = 8k
        NULL,            // Parameter passed as input of the task
        1,               // Priority of the task.
        NULL             // Task handle.
    );
}

void loop() {
    // Run the Shuck Shell Interpreter
    ShuckCMD::Run::cmd("shuck -prompt \"\\e\\[1;32m#\\e\\[0m \"", *shuck_terminal_console);
    delay(1000);
}
