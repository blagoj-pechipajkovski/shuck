
ShuckCMD echo_cmd("echo", "Echo string", [](ShuckArgs& args, ShuckConsole& console) -> int {
    int n = 1;
    while (args.arg(n+1) != nullptr) {
        console.out.print(args.arg(n));
        console.out.print(' ');
        n++;
    }
    console.out.print(args.arg(n));
    console.out.print('\n');

    return SHUCK_CMD_SUCCESS;
});

ShuckCMD sleep_cmd("sleep", "Causes blocking delay", [](ShuckArgs& args, ShuckConsole& console) -> int {
    const char* usage[2] = {
        "sleep [duration_seconds]\n",
        "sleep [duration_milliseconds] ms\n"
    };
    ;
    if (args.has(1)) {
        if (args.arg_is_int(1)) {
            unsigned long start_millis = millis();
            unsigned long delay_duration = atoi(args.arg(1));
            if (!args.arg_is(2, "ms"))
                delay_duration *= 1000;
            while (millis() < start_millis+delay_duration) {
                if (console.user.available()) {
                    char first = console.in.read();
                    while (console.user.available()) {
                        console.user.read();
                        delay(1);
                    }
                    if (first == ESC)
                        break;
                }
                delay(1);
            }
        }
        else {
            console.print_event(args.arg(0), "Error", "Please specify duration as integer");
            console.print_usage(
                usage[0],
                usage[1]
            );
            return SHUCK_CMD_USAGE_ERR;
        }
    }
    else {
        console.print_event(args.arg(0), "Error", "Sleep time not specified");
        console.print_usage(
            usage[0],
            usage[1]
        );
        return SHUCK_CMD_USAGE_ERR;
    }
    
    return SHUCK_CMD_SUCCESS;
});