
ShuckCMD args_cmd("args", "Show arguments", [](ShuckArgs& args, ShuckConsole& console) -> int {
    char num_string[5];
    strcpy(num_string, "%d");
    if (args.argc() >= 10)
        strcpy(num_string, "%2d");
    if (args.argc() >= 100)
        strcpy(num_string, "%3d");
    for (int i=0; i<args.argc(); i++) {
        console.out.printf(num_string, i);
        console.out.print(": ");
        console.out.print(args.arg(i));
        if (args.arg_is_int(i))
            console.out.print("; INTEGER");
        if (args.arg_is_decimal(i))
            console.out.print("; DECIMAL");
        console.out.print("\n");
    }
    return SHUCK_CMD_SUCCESS;
});

ShuckCMD asda("Tnamed", "Test named args", [](ShuckArgs& args, ShuckConsole& console) -> int { //TODO// merge with args
    ShuckNamedArgs named_args = ShuckNamedArgs(&args);
    
    
    for (int i=0; i<args.argc(); i++) {
        console.out.printf("%02d: %s\n", i, args.arg(i));
    }
    
    console.out.print("================\n");
    
    for (int i=0; i<named_args.numberedc(); i++) {
        console.out.printf("%02d: %s\n", i, named_args.numbered(i));
    }
    console.out.print("----------------\n");
    for (int i=0; i<named_args.namedc(); i++) {
        const char* name = named_args.name_by_index(i);
        
        if (!named_args.has_name(name))
            break;
        
        console.out.printf("%s:\n    ", name);
        if (named_args.name_has_val(name))
            console.out.printf("%s", named_args.name_val(name));
        else
            console.out.printf(". -- NO VALUE -- .");
        console.out.print("\n");
    }
    
    return SHUCK_CMD_SUCCESS;
});



ShuckCMD test_kbd_cmd("Tkbd", "Test keyboard", [](ShuckArgs& args, ShuckConsole& console) -> int {
    
    ShuckKeyboardTerminal skt = ShuckKeyboardTerminal(&console.in);
    
    while (true) {
    
        skt.loop();
        
        if (skt.key_available()) {
            ShuckKeyboard::KeyInput key;
            skt.key_read(key);
            
            console.out.print("\n");
            
            console.out.print("CTRL:  ");
            console.out.print(key.ctrl()  ? "ON" : "OFF");
            console.out.print("\n");
            
            console.out.print("ALT:   ");
            console.out.print(key.alt()   ? "ON" : "OFF");
            console.out.print("\n");
            
            console.out.print("SHIFT: ");
            console.out.print(key.shift() ? "ON" : "OFF");
            console.out.print("\n");
            
            console.out.print("CHR: ");
            console.out.print(key.is_chr ? "TRUE" : "FALSE");
            console.out.print("\n");
            
            console.out.print("KEY:  >");
            if (key.is_chr) {
                console.out.print(key.chr);
            }
            else {
                switch(key.non_chr) {
                    case ShuckKeyboard::NonChrKeys::ESCAPE:
                        console.out.print("ESCAPE");
                        break;
                    
                    case ShuckKeyboard::NonChrKeys::DELETE:
                        console.out.print("DELETE");
                        break;
                    case ShuckKeyboard::NonChrKeys::BACKSPACE:
                        console.out.print("BACKSPACE");
                        break;
                    
                    case ShuckKeyboard::NonChrKeys::ARROW_UP:
                        console.out.print("ARROW_UP");
                        break;
                    case ShuckKeyboard::NonChrKeys::ARROW_DOWN:
                        console.out.print("ARROW_DOWN");
                        break;
                    case ShuckKeyboard::NonChrKeys::ARROW_LEFT:
                        console.out.print("ARROW_LEFT");
                        break;
                    case ShuckKeyboard::NonChrKeys::ARROW_RIGHT:
                        console.out.print("ARROW_RIGHT");
                        break;
                    
                    case ShuckKeyboard::NonChrKeys::HOME:
                        console.out.print("HOME");
                        break;
                    case ShuckKeyboard::NonChrKeys::END:
                        console.out.print("END");
                        break;
                    default:
                        console.out.print("unknown");
                        break;
                }
            }
            console.out.print("<\n");
            if (key.is(MOD_KEYS_NONE, ShuckKeyboard::NonChrKeys::ESCAPE))
                break;
            if (key.is(MOD_KEYS_NONE, 'T'))
                console.out.print("test\n");
        }
        
        if (skt.available()) {
            console.out.printf("Read as from Stream: %c\n", skt.read());
            console.out.print("\n");
        }
        
    }
    
    return SHUCK_CMD_SUCCESS;
});

//ShuckPrompt prompt = ShuckPrompt();

ShuckCMD test_history_cmd("Thst", "Test history", [](ShuckArgs& args, ShuckConsole& console) -> int {
/*
    if (args.argc() == 3) {
        if (args.arg_is(1, "add")) {
            prompt.historyy.add(args.arg(2));
        }
        else {
            int index = atoi(args.arg(2));
            const char* line = prompt.historyy.get(index);
            if (line)
                console.out.printf("%d: %s\n", index, line);
            else
                console.out.printf("NOT AVAILABLE\n");
        }
    }
    else {
        return SHUCK_CMD_GENERAL_ERR;
    }*/
    
    return SHUCK_CMD_SUCCESS;
});

class CircularBuffer {
  private:
    static const int capacity = 3;
    char buff[capacity];
    int latest = -1;
    int count = 0;
  public:
    CircularBuffer() {}

    void add(char c) {
        
        latest = (latest + 1) % capacity;
        buff[latest] = c;
        if (count != capacity)
            count++;
        
    }

    char get(int index) {
        // magija // https://stackoverflow.com/a/6765091
        // smenato (latest+index+1) bese samo index // isto taka magija
        int pos = (((latest+index+1) % count) + count) % count;
        
        return buff[pos];
    }
    
    int max() {
        return count-1;
    }
    
    int min() {
        return -count;
    }
};


ShuckCMD cbuff_cmd("cbuff", "Test keyboard", [](ShuckArgs& args, ShuckConsole& console) -> int {
    CircularBuffer cb;
    
    
    // Adding values to the buffer
    for (char c = 'A'; c <= 'F'; ++c) {
        Serial.println();
        Serial.print("Adding: ");
        Serial.println(c);
        cb.add(c);
        for (int i = cb.min(); i <= cb.max(); i++) {
            Serial.print("get(");
            Serial.print(i);
            Serial.print("): ");
            Serial.println(cb.get(i));
        }
    }
    return 0;
});