#ifndef SHUCK_ARGS_H
#define SHUCK_ARGS_H

class ShuckArgs {
  friend class ShuckNamedArgs;
  private:
    char* args_buff = nullptr;
    char* _args_p[25]; //TODO// make dynamic
    int _args_n = 0;
  public:
  
    int generate_from(const char* marked_arguments, char arg_marker) {
        
        int args_buff_len = strlen(marked_arguments)+1;
        
        this->args_buff = new char[args_buff_len];
        
        strcpy(this->args_buff, marked_arguments);
        
        this->_args_p[this->_args_n] = this->args_buff;
        this->_args_n++;
        
        int i = 0;
        while (i < args_buff_len-1) {
            if (this->args_buff[i] == arg_marker) {
                this->args_buff[i] = '\0';
                
                this->_args_p[this->_args_n] = &this->args_buff[i+1];
                this->_args_n++;
            }
            i += 1;
        }
        
        return this->_args_n;
    }
    
    void escape_generated() {
        for (int i=0; i<this->_args_n; i++) {
            
            char* arg_in_pos = this->_args_p[i];
            char* arg_out_pos = this->_args_p[i];
            
            while (*arg_out_pos != '\0') {
                
                char add_char;
                
                // Escape:
                // \\ -> \
                // \" -> "
                // \( -> (
                // \) -> )
                // \[ -> [
                // \] -> ]
                // \{ -> {
                // \} -> }
                
                // \e -> 0x1B // escape
                // \n -> 0x0A
                // \r -> 0x0D
                
                // \[other char] -> '' (nothing)
                //TODO// \xNN -> 0xNN
                
                bool dont_add_char = false;
                if (*arg_in_pos == '\\') {
                    arg_in_pos++;
                    switch (*arg_in_pos) {
                        case '\\':
                        case '"':
                        case '(':
                        case ')':
                        case '[':
                        case ']':
                        case '{':
                        case '}':
                            add_char = *arg_in_pos;
                            break;
                        case 'e':
                            add_char = 0x1B;
                            break;
                        case 'n':
                            add_char = 0x0A;
                            break;
                        case 'r':
                            add_char = 0x0D;
                            break;
                        case 'x':
                            add_char = 0x0D;
                            break;
                        default:
                            dont_add_char = true;
                            arg_out_pos--;
                    }
                }
                else {
                    add_char = *arg_in_pos;
                }
                
                if (!dont_add_char)
                    *arg_out_pos = add_char;
                
                arg_in_pos++;
                arg_out_pos++;
            }
            
            *arg_out_pos = '\0';
        }
    }
    
    ~ShuckArgs() {
        delete[] args_buff;
    }

    // returns number of args
    int argc() {
        return this->_args_n;
    }

    int valid() {
        return this->argc() > 0;
    }

    bool has(int i) {
        return i < this->argc();
    }
    
    #define ARG_ANY "\x11"
    #define ARG_INT "\x12"
    // matches("state \x11 when \x11 OFF")
    // matches("state " ARG_ANY " when " ARG_ANY " OFF")
    // matches("pin write \x12 HIGH")
    // matches("pin write " ARG_INT " HIGH")
    int match(const char* user_pattern) {
        
        char pattern[strlen(user_pattern)+1];
        strcpy(pattern, user_pattern);
        
        int i = 0;
        char* token = strtok(pattern, " ");
        while (token != NULL) {
            if (strcmp(token, ARG_ANY) == 0) {
                if (!has(i))
                    return i;
            }
            else if (strcmp(token, ARG_INT) == 0) {
                if (!arg_is_int(i))
                    return i;
            }
            else {
                if (!arg_is(i, token))
                    return i;
            }
            i++;
            token = strtok(NULL, " ");
        }
        return -1;
    }

    // get argument by index
    const char* arg(int i) {
        if (i >= this->argc())
            return nullptr;

        return this->_args_p[i];
    }

    bool arg_is(int i, const char* th) {
        if (arg(i) == nullptr)
            return false;
        return strcmp(th, arg(i)) == 0;
    }
    
    bool arg_is_int(int i) {
        if (arg(i) == nullptr)
            return false;
        int start_index = 0;
        if (arg(i)[0] == '+' || arg(i)[0] == '-')
            start_index = 1;
        for (int j=start_index; j<strlen(arg(i)); j++) {
            if (isdigit(arg(i)[j]) == 0) { // not numeric
                return false;
            }
        }
        return true;
    }
    
    bool arg_is_decimal(int i) { // (fuck exponent)
        if (arg(i) == nullptr)
            return false;
        int start_index = 0;
        bool dot_found = false;
        if (arg(i)[0] == '+' || arg(i)[0] == '-' || arg(i)[0] == '.')
            start_index = 1;
        if (arg(i)[0] == '.') {
            start_index = 1;
            dot_found = true;
        }
        for (int j=start_index; j<strlen(arg(i)); j++) {
            if (isdigit(arg(i)[j]) == 1) { // numeric
                continue;
            }
            else if (arg(i)[j] == '.' && !dot_found) {
                dot_found = true;
                continue;
            }
            return false; // if this is reached not float
        }
        return true;
    }
};

class ShuckNamedArgs {
    
  private:
    ShuckArgs* _shuck_args;
    
    // args_buff
    
    const char* _args_numbered_p[25];
    int _args_numbered_count = 0;
    
    
    const char* _args_named_names_p[25];
    const char* _args_named_vals_p[25] = { nullptr };
    int _args_named_count = 0;
    
  public:
    ShuckNamedArgs(ShuckArgs* shuck_args) {
        this->_shuck_args = shuck_args;
        
        for (int i=1; i<this->_shuck_args->argc(); i++) {
            if (this->_shuck_args->arg(i)[0] == '-') {
                const char* name = this->_shuck_args->arg(i);
                const char* val  = this->_shuck_args->arg(i+1);
                
                _args_named_names_p[_args_named_count] = (name+1);
                if (val != nullptr) {
                    if (val[0] != '-') {
                        _args_named_vals_p[_args_named_count] = (val);
                        i++;
                    }
                }
                _args_named_count++;
            }
            else {
                _args_numbered_p[_args_numbered_count] = this->_shuck_args->arg(i);
                _args_numbered_count++;
            }
        }
    }
    
    
    int numberedc() {
        return _args_numbered_count;
    }
    
    const char* numbered(int i) {
        if (i > this->numberedc())
            return nullptr;

        return this->_args_numbered_p[i];
    }
    
    
    
    int namedc() {
        return _args_named_count;
    }
    
    const char* name_by_index(int i) {
        if (i > this->namedc())
            return nullptr;

        return this->_args_named_names_p[i];
    }
    
    bool has_name(const char* n) {
        if (n == nullptr)
            return false;
            
        const char* found_name = nullptr;
        
        for (int i=0; i < this->namedc(); i++) {
            if (strcmp(this->_args_named_names_p[i], n) == 0) {
                found_name = this->_args_named_names_p[i];
                break;
            }
        }
        
        return found_name != nullptr;
    }
    
    const char* name_val(const char* n) {
        if (!this->has_name(n))
            return nullptr;
        
        
        const char* found_val = nullptr;
        
        for (int i=0; i < this->namedc(); i++) {
            if (strcmp(this->_args_named_names_p[i], n) == 0) {
                found_val = this->_args_named_vals_p[i];
                break;
            }
        }

        return found_val;
    }
    
    bool name_has_val(const char* n) {
        return this->name_val(n) != nullptr;
    }
    
    bool name_val_is(const char* n, const char* v) {
        const char* found = this->name_val(n);
        if (found != nullptr)
            return strcmp(found, v) == 0;
        
        return false;
    }
    
    //AKO// dodavas is_int DODAJ GO UBO
};

#endif