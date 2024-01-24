
/*char prompt[16] = "\e[1;32m#\e[0m ";
ShuckCMD chprompt("chprompt", [](ShuckArgs& args, ShuckPrompt& cl) {
    if (args.has(1))
        strcpy(Shuck::global_prompt, args.arg(1));
    else
        //strcpy(prompt, "# ");
        strcpy(prompt, "\e[1;32m#\e[0m ");
});*/

ShuckCMD shuck_cmd("shuck", "Run the Shuck interpreter", [](ShuckArgs& args, ShuckConsole& console) -> int {
    // Print iNfO
    console.out.print("\n");
    console.out.print("ShuckOS\n");
    console.print_event(args.arg(0), "Version: 0.0.0");
    
    ShuckNamedArgs named_args = ShuckNamedArgs(&args);
    
    
    const char* prompt_str = "# ";
    if (named_args.name_has_val("prompt"))
        prompt_str = named_args.name_val("prompt");
    
    ShuckPrompt* shuck_prompt;
    if (named_args.name_has_val("prompt") || named_args.has_name("interactive")) {
        shuck_prompt = new ShuckInteractivePrompt(prompt_str);
        console.print_event(args.arg(0), "Interactive mode");
    }
    else {
        shuck_prompt = new ShuckPrompt();
        console.print_event(args.arg(0), "Non interactive mode");
    }
    
    while (true) {
        
        shuck_prompt->loop(console);
        
        if (console.status() == ShuckConsole::Status::CANCEL) {
            shuck_prompt->reset();
            console.display.print('\n');
        }
        
        if (console.status() == ShuckConsole::Status::CLS) {
            shuck_prompt->reset();
        }
        
        if (console.keyboard.key_available()) { // prompt has not taken the key
            ShuckKeyboard::KeyInput key;
            console.keyboard.key_read(key); // final destination for keys
            
            // do sth if necessary
        }
        
        if (shuck_prompt->available()) {
            
            if (shuck_prompt->is("exit"))
                break;
            int cmd_ret = ShuckCMD::Run::cmd(shuck_prompt->c_str(), console);
            if (cmd_ret != SHUCK_CMD_SUCCESS) {
                console.print_event(args.arg(0), ShuckCMD::cmd_ret_to_str(cmd_ret));
            }
            shuck_prompt->reset();
        }
        
        delay(1);
    }
    
    /*ShuckPromptAsStream as_str = ShuckPromptAsStream(*shuck_prompt);
    while(as_str.available()) {
        console.out.write(as_str.read());
    }*/
    
    console.print_event(args.arg(0), "Exit");

    return SHUCK_CMD_SUCCESS;
});

ShuckCMD comment_cmd("#", "Comment command", [](ShuckArgs& args, ShuckConsole& console) -> int {
    return SHUCK_CMD_SUCCESS;
});

/*
ShuckCMD if_cmd("if", "if command", [](ShuckArgs& args, ShuckConsole& console) -> int {
    
    //TODO// single line if
    
    ShuckInteractivePrompt interactive_prompt("> ", ALLOW_DUPLICATE_HISTORY);
    int if_level = 0;
    while (true) {
        interactive_prompt.loop(console);
        
        if (console.status() == ShuckConsole::Status::CANCEL) {
            interactive_prompt.reset();
            console.display.print('\n');
        }
        
        if (console.keyboard.key_available())
            console.keyboard.reset();
        
        if (interactive_prompt.available()) {
            
            ShuckArgs interactive_prompt_args;
            interactive_prompt_args.generate_from(interactive_prompt.c_str(), ' ');
            
            if (interactive_prompt_args.arg_is(0, "if"))
                if_level++;
            if (interactive_prompt_args.arg_is(0, "fi"))
                if_level--;
            if (if_level < 0)
                break;
            interactive_prompt.reset();
        }
        
        delay(1);
    }

    if (interactive_prompt.history.len() <= 1)
        return SHUCK_CMD_SUCCESS;

    bool execute = false;
    if (args.arg_is(1, "true") || args.arg_is(1, "1"))
        execute = true;
    
    
    // used for line reading capabilities
    ShuckPrompt exec_prompt;
    exec_prompt.history_enabled(false);
    
    // Make the body a stream without the last/closing fi
    ShuckPromptAsStream if_body_stream(interactive_prompt, interactive_prompt.history.max()-1);
    
    ShuckConsole exec_console(if_body_stream, console.out, console.user);
    
    while (if_body_stream.available()) {
        
        exec_prompt.loop(exec_console);
        
        if (exec_console.keyboard.key_available())
            exec_console.keyboard.reset();
        
        if (exec_prompt.available()) {
            
            ShuckArgs exec_prompt_args;
            exec_prompt_args.generate_from(exec_prompt.c_str(), ' ');
            
            if (exec_prompt_args.arg_is(0, "else")) {
                if (execute)
                    break;
                else
                    execute = !execute;
            }
            else {
                if (execute) {
                    int cmd_ret = ShuckCMD::Run::cmd(exec_prompt.c_str(), exec_console);
                    if (cmd_ret != SHUCK_CMD_SUCCESS) {
                        console.print_event(args.arg(0), ShuckCMD::cmd_ret_to_str(cmd_ret));
                        return SHUCK_CMD_SUBCMD_ERR;
                    }
                }
            }
            exec_prompt.reset();
        }
    }
    
    return SHUCK_CMD_SUCCESS;
    
});

ShuckCMD for_cmd("for", "for command", [](ShuckArgs& args, ShuckConsole& console) -> int {
    
    int start = 0;
    int end = -1;
    int step = 1;
    
    if      (args.match("for " ARG_ANY " up to " ARG_INT) == -1) {
        end = atoi(args.arg(4));
    }
    else if (args.match("for " ARG_ANY " in " ARG_INT " to " ARG_INT) == -1) {
        start = atoi(args.arg(3));
        end = atoi(args.arg(5));
    }
    else if (args.match("for " ARG_ANY " in " ARG_INT " to " ARG_INT " step " ARG_INT) == -1) {
        start = atoi(args.arg(3));
        end = atoi(args.arg(5));
        step = atoi(args.arg(7));
    }
    else {
        console.print_event(args.arg(0), "Error", "invalid syntax");
        console.print_usage(
            "for [var_name] up to [end index - exclusive]",
            "for [var_name] in [start index - inclusive] to [last index - exclusive]",
            "for [var_name] in [start index - inclusive] to [last index - exclusive] step [step value]"
        );
        return SHUCK_CMD_USAGE_ERR;
    }
    
    
    
    
    ShuckInteractivePrompt interactive_prompt("> ", ALLOW_DUPLICATE_HISTORY);
    int for_level = 0;
    while (true) {
        
        interactive_prompt.loop(console);
        
        if (console.status() == ShuckConsole::Status::CANCEL) {
            interactive_prompt.reset();
            console.display.print('\n');
        }
        
        if (console.keyboard.key_available())
            console.keyboard.reset();
        
        if (interactive_prompt.available()) {
            ShuckArgs interactive_prompt_args;
            interactive_prompt_args.generate_from(interactive_prompt.c_str(), ' ');
            if (interactive_prompt_args.arg_is(0, "for"))
                for_level++;
            if (interactive_prompt_args.arg_is(0, "rof"))
                for_level--;
            if (for_level < 0)
                break;
            interactive_prompt.reset();
        }
        
        delay(1);
    }
    
    if (interactive_prompt.history.len() == 0)
        return SHUCK_CMD_SUCCESS;
    
    
    for (int i=start; i<end; i+=step) {
        
        char loop_var_val[10];
        itoa(i, loop_var_val, 10);
        setenv(args.arg(1), loop_var_val, 1);
        
        
        
        // used for line reading capabilities
        ShuckPrompt exec_prompt;
        exec_prompt.history_enabled(false);
        
        // Make the body a stream without the last/closing rof
        ShuckPromptAsStream for_body_stream(interactive_prompt, interactive_prompt.history.max()-1);
        
        ShuckConsole exec_console(for_body_stream, console.out, console.user);
        
        while (for_body_stream.available()) {        
            
            exec_prompt.loop(exec_console);
            
            if (exec_console.keyboard.key_available())
                exec_console.keyboard.reset();
        
            if (exec_prompt.available()) {
            
                //ShuckArgs exec_prompt_args;
                //exec_prompt_args.generate_from(exec_line, ' ');
                //AKO// ima break ovde, vo exec_prompt_args, treba da e
                
                int cmd_ret = ShuckCMD::Run::cmd(exec_prompt.c_str(), exec_console);
                if (cmd_ret != SHUCK_CMD_SUCCESS) {
                    console.print_event(args.arg(0), ShuckCMD::cmd_ret_to_str(cmd_ret));
                    return SHUCK_CMD_SUBCMD_ERR;
                }
            }
        }
    }
    unsetenv(args.arg(1));
    
    
    return SHUCK_CMD_SUCCESS;
});
*/



ShuckCMD set_cmd("set", "Set environment variable",[](ShuckArgs& args, ShuckConsole& console) -> int {
    if (args.argc() != 3) {
        console.print_event(args.arg(0), "Error", "must specify [var_name] and [value]");
        console.print_usage(
            "set [var_name] [value]"
        );
        return SHUCK_CMD_USAGE_ERR;
    }

    setenv(args.arg(1), args.arg(2), 1);
    return SHUCK_CMD_SUCCESS;
});

ShuckCMD read_cmd("read", "Read user input to environment variable", [](ShuckArgs& args, ShuckConsole& console) -> int {
    if (!args.has(1)) {
        console.print_event(args.arg(0), "Error", "must specify [var_name]");
        console.print_usage(
            "read [var_name]",
            "read [var_name] [prompt]"
        );
        return SHUCK_CMD_USAGE_ERR;
    }

    ShuckInteractivePrompt interactive_prompt;
    if (args.has(2))
        interactive_prompt.set_prompt(args.arg(2));
    
    while (true) {
        
        interactive_prompt.loop(console);
        
        if (console.status() == ShuckConsole::Status::CANCEL) {
            console.display.print('\n');
            return SHUCK_CMD_USER_CANCEL;
        }
        
        if (console.keyboard.key_available()) { // prompt has not taken the key
            ShuckKeyboard::KeyInput key;
            console.keyboard.key_read(key); // final destination for keys
            
            // do sth if necessary
        }
        
        if (interactive_prompt.available()) {
            setenv(args.arg(1), interactive_prompt.c_str(), 1);
            return SHUCK_CMD_SUCCESS;
        }
        
        delay(1);
    }
    
    return SHUCK_CMD_SUCCESS;
});

ShuckCMD help_cmd("help", "List available commands", [](ShuckArgs& args, ShuckConsole& console) -> int {
    if (args.has(1)) {
        ShuckCMD* command = ShuckCMD::List::find(args.arg(1));
        
        if (command != nullptr) {
            console.out.print(command->name());
            console.out.print(" - ");
            if (strlen(command->description()) > 0)
                console.out.print(command->description());
            else
                console.out.print("[NO DESCRIPTION AVAILABLE]");
            console.out.print('\n');
            return SHUCK_CMD_SUCCESS;
        }
        
        console.print_event(args.arg(0), "Error", "Command does not exist:", args.arg(1));
        return SHUCK_CMD_GENERAL_ERR;
    }
    else {
        console.out.print("Available commands:\n");
        
        for (int i=0; i<ShuckCMD::List::len(); i++) {
            
            ShuckCMD* command = ShuckCMD::List::at(i);
            
            console.out.printf("  %-12s", command->name());
            if (strlen(command->description()) > 0) {
                console.out.print(" - ");
                console.out.print(command->description());
            }
            console.out.print('\n');
        }
        
        console.out.print("For description of specific comand try:\n"
                      "  help [command]\n"
                      "For comand's help try:\n"
                      "  [command] help\n");
        return SHUCK_CMD_SUCCESS;
    }
});
