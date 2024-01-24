
ShuckCMD pin_cmd("pin", "Pin manipulation", [](ShuckArgs& args, ShuckConsole& console) -> int {

    if (args.arg_is(1, "mode") || args.arg_is(1, "m")) {
        const char* usage[6] = {
            "pin mode [GPIO_pin] [mode]",
            "mode:",
            "  output/o"
            "  input/i"
            "  input_pullup/ip"
            "  input_pulldown/id"
        };
        
        if (!args.has(2)) {
            console.print_event(args.arg(0), "Error", "Please specify GPIO pin");
            console.print_usage(usage[0], usage[1], usage[2], usage[3], usage[4], usage[5]);
            return SHUCK_CMD_USAGE_ERR;
        }

        if (args.arg_is(2, "help")) {
            console.print_usage(usage[0], usage[1], usage[2], usage[3], usage[4], usage[5]);
            return SHUCK_CMD_SUCCESS;
        }

        if (!args.arg_is_int(2)) {
            console.print_event(args.arg(0), "Error", "Please specify GPIO pin as integer");
            console.print_usage(usage[0], usage[1], usage[2], usage[3], usage[4], usage[5]);
            return SHUCK_CMD_USAGE_ERR;
        }

        if (args.arg_is(3, "output") || args.arg_is(3, "o")) {
            pinMode(atoi(args.arg(2)), OUTPUT);
        }
        else if (args.arg_is(3, "input") || args.arg_is(3, "i")) {
            pinMode(atoi(args.arg(2)), INPUT);
        }
        else if (args.arg_is(3, "input_pullup") || args.arg_is(3, "ip")) {
            pinMode(atoi(args.arg(2)), INPUT_PULLUP);
        }
        else if (args.arg_is(3, "input_pulldown") || args.arg_is(3, "id")) {
            pinMode(atoi(args.arg(2)), INPUT_PULLDOWN);
        }
        else {
            console.print_event(args.arg(0), "Error", "Please specify valid mode");
            console.print_usage(usage[0], usage[1], usage[2], usage[3], usage[4], usage[5]);
            return SHUCK_CMD_USAGE_ERR;
        }
        return SHUCK_CMD_SUCCESS;
    }

    if (args.arg_is(1, "read") || args.arg_is(1, "r")) {
        const char* usage = "pin read [pin]";

        if (!args.has(2)) {
            console.print_event(args.arg(0), "Error", "Please specify GPIO pin");
            console.print_usage(usage);
            return SHUCK_CMD_USAGE_ERR;
        }

        if (args.arg_is(2, "help")) {
            console.print_event(args.arg(0), "Error", "Please specify GPIO pin");
            console.print_usage(usage);
            return SHUCK_CMD_SUCCESS;
        }

        if (!args.arg_is_int(2)) {
            console.print_event(args.arg(0), "Error", "Please specify GPIO pin as integer");
            console.print_usage(usage);
            return SHUCK_CMD_USAGE_ERR;
        }

        console.out.print(digitalRead(atoi(args.arg(2)))==HIGH ? "1" : "0");
        console.out.print("\n");
        return SHUCK_CMD_SUCCESS;
    }

    if (args.arg_is(1, "write") || args.arg_is(1, "w")) {
        const char* usage[4] = {
            "pin write [GPIO_pin] [state]",
            "state:",
            "  high/h/1",
            "  low/l/0"
        };
        
        if (!args.has(2)) {
            console.print_event(args.arg(0), "Error", "Please specify GPIO pin");
            console.print_usage(usage[0], usage[1], usage[2], usage[3]);
            return SHUCK_CMD_USAGE_ERR;
        }

        if (args.arg_is(2, "help")) {
            console.print_usage(usage[0], usage[1], usage[2], usage[3]);
            return SHUCK_CMD_SUCCESS;
        }

        if (!args.arg_is_int(2)) {
            console.print_event(args.arg(0), "Error", "Please specify GPIO pin as integer");
            console.print_usage(usage[0], usage[1], usage[2], usage[3]);
            return SHUCK_CMD_USAGE_ERR;
        }
        
        if (args.arg_is(3, "1") || args.arg_is(3, "high") || args.arg_is(3, "h")) {
            digitalWrite(atoi(args.arg(2)), HIGH);
        }
        else if (args.arg_is(3, "0") || args.arg_is(3, "low") || args.arg_is(3, "l")) {
            digitalWrite(atoi(args.arg(2)), LOW);
        }
        else {
            console.print_event(args.arg(0), "Error", "Please specify valid state");
            console.print_usage(usage[0], usage[1], usage[2], usage[3]);
            return SHUCK_CMD_USAGE_ERR;
        }
        return SHUCK_CMD_SUCCESS;
    }

    /*if (args.arg_is(1, "touchstream") || args.arg_is(1, "ts")) {
        const char* main_err_msg =
            "Uasge:\n"
            "pin touchstream [pin]\n"
            "where:\n"
            "[pin] is the GPIO number that supports touch input\n"
            "GPIOs that supports touch input are:\n"
            "  4, 0, 2, 15, 13, 12, 14, 17, 32\n"
        ;

        if (!args.has(2)) {
            show_cmd_err_msg(stream, main_err_msg, "Specify GPIO pin");
            return;
        }

        if (args.arg_is(2, "help")) {
            show_cmd_err_msg(stream, main_err_msg);
            return;
        }

        if (
            args.arg_is(2, "4") ||
            args.arg_is(2, "0") ||
            args.arg_is(2, "2") ||
            args.arg_is(2, "15") ||
            args.arg_is(2, "13") ||
            args.arg_is(2, "12") ||
            args.arg_is(2, "14") ||
            args.arg_is(2, "17") ||
            args.arg_is(2, "32")
        
        ) {
            int pin = atoi(args.arg(2));
            int bar_width = 50;
            int max_touch_val = 200;
            while (!stream.available()) {
                int touch = touchRead(pin);
                int num_colored = map(touch, 0, max_touch_val, 0, bar_width);
                stream.print("|");
                for (int i=0; i<bar_width; i++)
                    if (i < num_colored)
                        stream.print("#");
                    else
                        stream.print(" ");
                stream.print("| ");
                stream.println(touch);
                delay(20);
            }
            stream.read();
        }
        else {
            show_cmd_err_msg(stream, main_err_msg, "Specify GPIO pin as integer");
            return;
        }
        return;
    }*/

    if (!args.arg_is(1, "help"))
        console.print_event(args.arg(0), "Error", "Please specify pin operation");
    
    console.print_usage(
        "pin mode/m",
        "pin read/r",
        "pin write/w",
        "pin touchstream/ts"
    );

    if (!args.arg_is(1, "help"))
        return SHUCK_CMD_USAGE_ERR;
    else
        return SHUCK_CMD_SUCCESS;
});

/*
class OutputPinController {
  private:
    int _pin;
    int _inactive_state;

    bool _auto_deactivate = false;
    unsigned long _deactivate_time;
  public:
    OutputPinController* _previous_instance = nullptr;
    static OutputPinController* last_instance;

    void begin(int pin, int inactive_state=LOW) {
        _pin = pin;
        pinMode(_pin, OUTPUT);
        _inactive_state = inactive_state;
        digitalWrite(pin, _inactive_state);

        _previous_instance = OutputPinController::last_instance;
        OutputPinController::last_instance = this;
    }

    bool active() {
        return digitalRead(_pin) != _inactive_state;
    }

    bool active(bool new_state) {
        if (new_state)
            digitalWrite(_pin, !_inactive_state);
        else
            digitalWrite(_pin, _inactive_state);
        
        return active();
    }

    void activate() {
        active(true);
    }

    void deactivate() {
        active(false);
    }

    void activate_for(unsigned long active_time) {
        _auto_deactivate = true;
        _deactivate_time = millis() + active_time;
        activate();
    }

    void _loop() {
        if (_auto_deactivate) {
            if (millis() > _deactivate_time) {
                _auto_deactivate = false;
                deactivate();
            }
        }
    }

    static void loop() {
        OutputPinController* current_pin = OutputPinController::last_instance;
        
        while (current_pin != nullptr) {
            current_pin->_loop();
            current_pin = current_pin->_previous_instance;
        }
    }
};
OutputPinController* OutputPinController::last_instance = nullptr;


OutputPinController led_ctrl;
ShuckCMD led("led", "Controll LED", [](ShuckArgs& args, ShuckPrompt& cl) {
    if (args.argc() == 1) {
        Serial.print("LED is ");
        Serial.println(led_ctrl.active() ? "ON" : "OFF");
    }

    if (args.arg_is(1, "help")) {
        Serial.println();
        Serial.println("Usage:");
        Serial.println("led [on/off]");
        Serial.println("Turns on board LED on or off");
        Serial.println("If no argument provided prints current status");
    }
    if (args.arg_is(1, "on")) {
        led_ctrl.activate();
    }
    if (args.arg_is(1, "off")) {
        led_ctrl.deactivate();
    }
});*/