namespace ShuckOS {
    const char line_terminator = '\n';
    
    bool is_print(char c) {
        return isprint(c) || c == ShuckOS::line_terminator;
    }
    
    bool is_truthy(const char* val) {
        if (val == nullptr) return false; // nullptr
        if (*val == '\0')   return false; // empty string
        
        if (strcmp(val, "true")==0) return true; // "true"
        if (atoi(val) != 0)         return true; // non zero
        
        return false; // everything else
    }
}

// Include core components
#include "shuck_args.h"
#include "shuck_keyboard.h"
#include "shuck_display.h"
#include "shuck_console.h"
#include "shuck_cmd.h"

// Include commands
#include "cmds.h"
