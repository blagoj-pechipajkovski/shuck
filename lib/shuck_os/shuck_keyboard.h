#ifndef SHUCK_KEYBOARD
#define SHUCK_KEYBOARD

#define ESC '\x1b'

// Char end key        a,b,...,1,2,...,!,@,...
// Non char end key    Delete,Backspace,Arrows,...

// Modifiers: Ctrl, Alt, Shift

// Shift + Char end key : IMPOSIBLE
// Shift + 'a' => 'A'

#define MOD_KEYS_NONE 0
#define MOD_KEY_CTRL  0b001
#define MOD_KEY_ALT   0b010
#define MOD_KEY_SHIFT 0b100

class ShuckKeyboard : public Stream {
  public:
    
    enum class NonChrKeys {
        ESCAPE,
        
        DELETE,
        BACKSPACE,
        
        ARROW_UP,
        ARROW_DOWN,
        ARROW_LEFT,
        ARROW_RIGHT,
        
        HOME,
        END,
    };
    
    struct KeyInput {
        
        uint8_t modifiers;
        
        bool is_chr;
        union {
            char chr;
            ShuckKeyboard::NonChrKeys non_chr;
        };
        
        bool ctrl() {
            return (this->modifiers &  MOD_KEY_CTRL) != 0;
        }
        bool alt() {
            return (this->modifiers &   MOD_KEY_ALT) != 0;
        }
        bool shift() {
            return (this->modifiers & MOD_KEY_SHIFT) != 0;
        }
        
        void set_modifiers(uint8_t mods) {
            this->modifiers = mods;
        }
        
        void set(uint8_t mods, char ch) {
            this->set_modifiers(mods);
            
            this->is_chr = true;
            this->chr = ch;
        }
        
        void set(uint8_t mods, ShuckKeyboard::NonChrKeys nch) {
            this->set_modifiers(mods);
            
            this->is_chr = false;
            this->non_chr = nch;
        }
        
        bool has_modifiers() {
            return this->modifiers != MOD_KEYS_NONE;
        }
        
        bool is_pure_chr() {
            return !this->has_modifiers() && this->is_chr;
        }
        
        bool is(uint8_t mods, char ch) {
            if (!this->is_chr)
                return false;
            return this->modifiers == mods && (this->chr == ch);
        }
        
        bool is(uint8_t mods, ShuckKeyboard::NonChrKeys nch) {
            if (this->is_chr)
                return false;
            return this->modifiers == mods && (this->non_chr == nch);
        }
    };
    
    virtual bool key_available() = 0;
    
    virtual void key_peek(KeyInput& key_input) = 0;
    
    virtual void key_read(KeyInput& key_input) = 0;
    
    
    virtual void reset() = 0;
    
    // in case active listening is required
    virtual void loop() { }
    
};




class ShuckKeyboardSimple : public ShuckKeyboard {
  private:
    Stream* _stream;
    
    bool _key_available = false;
    ShuckKeyboard::KeyInput _last_key_input;
    
    bool _available = false;
    char _last_char;
    
  public:
    ShuckKeyboardSimple(Stream* stream) {
        this->_stream = stream;
    }

    bool key_available() override {
        return this->_key_available;
    }
    
    void key_peek(ShuckKeyboard::KeyInput& key_input) override {
        memcpy(&key_input, &this->_last_key_input, sizeof(this->_last_key_input));
    }
    
    void key_read(ShuckKeyboard::KeyInput& key_input) override {
        this->key_peek(key_input);
        this->_key_available = false;
    }

    int available() override {
        return this->_available;
    }
    
    int peek() {
        return this->_last_char;
    }
    
    int read() override {
        this->_available = false;
        return this->peek();
    }
    
    virtual void reset() {
        this->_available = false;
        this->_key_available = false;
    }
    
    size_t write(uint8_t) { return 0; }
    
    
    void loop() override {
        
        if (this->_stream->available()) {
            // Handle one char
            char in_char = this->_stream->read();
            
            if (ShuckOS::is_print(in_char)) {
                _last_key_input.set(MOD_KEYS_NONE, in_char);
                this->_key_available = true;
                
                this->_available = true;
                this->_last_char = this->_last_key_input.chr;
            }
        }
        // loop
    }
};


class ShuckKeyboardTerminal : public ShuckKeyboard {
  private:
    Stream* _stream;
    
    bool _key_available = false;
    ShuckKeyboard::KeyInput _last_key_input;
    
    bool _available = false;
    char _last_char;
    
  public:
    ShuckKeyboardTerminal(Stream* stream) {
        this->_stream = stream;
    }

    bool key_available() override {
        return this->_key_available;
    }
    
    void key_peek(ShuckKeyboard::KeyInput& key_input) override {
        memcpy(&key_input, &this->_last_key_input, sizeof(this->_last_key_input));
    }
    
    void key_read(ShuckKeyboard::KeyInput& key_input) override {
        this->key_peek(key_input);
        this->_key_available = false;
    }

    int available() override {
        return this->_available;
    }
    
    int peek() {
        return this->_last_char;
    }
    
    int read() override {
        this->_available = false;
        return this->peek();
    }
    
    virtual void reset() {
        this->_available = false;
        this->_key_available = false;
    }
    
    size_t write(uint8_t) { return 0; }
    
    // seq_1 is NOT a null terminated string, it is a regular char array as received
    // seq_2 is a string in the form of "^[A"
    // Both need to begin with '^' or ESC character
    bool esc_sequence_eq(const char* seq_1, int seq_1_len, const char* seq_2) {
        if (seq_1[0] != ESC && seq_1[0] != '^')
            return false;
        if (seq_2[0] != ESC && seq_2[0] != '^')
            return false;
        
        if (seq_1_len != strlen(seq_2))
            return false;
        
        return memcmp(seq_1 + 1, seq_2 + 1, seq_1_len - 1) == 0;
    }
    
    void loop() override {
        char buff[10];
        int buff_len = 0;
        
        if (this->_stream->available()) {
            // Handle one char
            buff[0] = this->_stream->read();
            buff_len = 1;
            
            // If escape sequence ther might be more chars
            if (buff[0] == ESC) {
                
                // time based escape sequence handling
                unsigned long rcv_time = millis();

                while (true) {
                    // Wait 5ms for additional chars
                    while (!this->_stream->available() && millis() < rcv_time+5)
                        delay(1);
                    
                    if (!this->_stream->available())
                        break;
                    else
                        buff[buff_len++] = this->_stream->read();
                }
            }
            
            // Reading done
            
            // Assume special key
            if (buff[0] != ESC) {
                if (buff[0] >= ' ') { // Definitely not special
                    _last_key_input.set(MOD_KEYS_NONE, buff[0]);
                }
                else { // Maybe control
                    switch (buff[0]) {
                        // Not control
                        case '\t':
                        case '\n':
                        case '\r':
                            _last_key_input.set(MOD_KEYS_NONE, buff[0]);
                            break;
                        // Control
                        case 0x08: // '\b'
                            _last_key_input.set(MOD_KEYS_NONE, ShuckKeyboard::NonChrKeys::BACKSPACE);
                            break;
                        case 0x01:
                            _last_key_input.set(MOD_KEY_CTRL, 'a');
                            break;
                        case 0x02:
                            _last_key_input.set(MOD_KEY_CTRL, 'b');
                            break;
                        case 0x03:
                            _last_key_input.set(MOD_KEY_CTRL, 'c');
                            break;
                        case 0x05:
                            _last_key_input.set(MOD_KEY_CTRL, 'e');
                            break;
                        case 0x06:
                            _last_key_input.set(MOD_KEY_CTRL, 'f');
                            break;
                        case 0x07:
                            _last_key_input.set(MOD_KEY_CTRL, 'g');
                            break;
                        case 0x0b:
                            _last_key_input.set(MOD_KEY_CTRL, 'k');
                            break;
                        case 0x0c:
                            _last_key_input.set(MOD_KEY_CTRL, 'l');
                            break;
                        case 0x0e:
                            _last_key_input.set(MOD_KEY_CTRL, 'n');
                            break;
                        case 0x0f:
                            _last_key_input.set(MOD_KEY_CTRL, 'o');
                            break;
                        case 0x10:
                            _last_key_input.set(MOD_KEY_CTRL, 'p');
                            break;
                        case 0x14:
                            _last_key_input.set(MOD_KEY_CTRL, 't');
                            break;
                        case 0x19:
                            _last_key_input.set(MOD_KEY_CTRL, 'y');
                            break;
                        default:
                            return;
                    }
                }
            }
            else { // ANSI escape
                // ESC key
                if      (ShuckKeyboardTerminal::esc_sequence_eq(buff, buff_len, "^"))
                    _last_key_input.set(MOD_KEYS_NONE, ShuckKeyboard::NonChrKeys::ESCAPE);
                // Delete
                else if (ShuckKeyboardTerminal::esc_sequence_eq(buff, buff_len, "^[3~"))
                    _last_key_input.set(MOD_KEYS_NONE, ShuckKeyboard::NonChrKeys::DELETE);
                // Ctrl + Delete
                else if (ShuckKeyboardTerminal::esc_sequence_eq(buff, buff_len, "^[3;5~"))
                    _last_key_input.set(MOD_KEY_CTRL, ShuckKeyboard::NonChrKeys::DELETE);
                // Ctrl + Delete
                else if (ShuckKeyboardTerminal::esc_sequence_eq(buff, buff_len, "^[3;5~"))
                    _last_key_input.set(MOD_KEY_ALT, ShuckKeyboard::NonChrKeys::DELETE);
                // Arrows
                else if (ShuckKeyboardTerminal::esc_sequence_eq(buff, buff_len, "^[A"))
                    _last_key_input.set(MOD_KEYS_NONE, ShuckKeyboard::NonChrKeys::ARROW_UP);
                else if (ShuckKeyboardTerminal::esc_sequence_eq(buff, buff_len, "^[B"))
                    _last_key_input.set(MOD_KEYS_NONE, ShuckKeyboard::NonChrKeys::ARROW_DOWN);
                else if (ShuckKeyboardTerminal::esc_sequence_eq(buff, buff_len, "^[C"))
                    _last_key_input.set(MOD_KEYS_NONE, ShuckKeyboard::NonChrKeys::ARROW_RIGHT);
                else if (ShuckKeyboardTerminal::esc_sequence_eq(buff, buff_len, "^[D"))
                    _last_key_input.set(MOD_KEYS_NONE, ShuckKeyboard::NonChrKeys::ARROW_LEFT);
                // Ctrl + Arrows
                else if (ShuckKeyboardTerminal::esc_sequence_eq(buff, buff_len, "^[1;5A"))
                    _last_key_input.set(MOD_KEY_CTRL, ShuckKeyboard::NonChrKeys::ARROW_UP);
                else if (ShuckKeyboardTerminal::esc_sequence_eq(buff, buff_len, "^[1;5B"))
                    _last_key_input.set(MOD_KEY_CTRL, ShuckKeyboard::NonChrKeys::ARROW_DOWN);
                else if (ShuckKeyboardTerminal::esc_sequence_eq(buff, buff_len, "^[1;5C"))
                    _last_key_input.set(MOD_KEY_CTRL, ShuckKeyboard::NonChrKeys::ARROW_RIGHT);
                else if (ShuckKeyboardTerminal::esc_sequence_eq(buff, buff_len, "^[1;5D"))
                    _last_key_input.set(MOD_KEY_CTRL, ShuckKeyboard::NonChrKeys::ARROW_LEFT);
                // Home
                else if (ShuckKeyboardTerminal::esc_sequence_eq(buff, buff_len, "^[H"))
                    _last_key_input.set(MOD_KEYS_NONE, ShuckKeyboard::NonChrKeys::HOME);
                // End
                else if (ShuckKeyboardTerminal::esc_sequence_eq(buff, buff_len, "^[F"))
                    _last_key_input.set(MOD_KEYS_NONE, ShuckKeyboard::NonChrKeys::END);
                // Ctrl + Home
                else if (ShuckKeyboardTerminal::esc_sequence_eq(buff, buff_len, "^[1;5H"))
                    _last_key_input.set(MOD_KEY_CTRL, ShuckKeyboard::NonChrKeys::HOME);
                // Ctrl + End
                else if (ShuckKeyboardTerminal::esc_sequence_eq(buff, buff_len, "^[1;5F"))
                    _last_key_input.set(MOD_KEY_CTRL, ShuckKeyboard::NonChrKeys::END);
                else {
                    if (buff_len == 2 && buff[1] >= ' ' && buff[1] <= '~') // Alt + chr key
                        _last_key_input.set(MOD_KEY_ALT, buff[1]);
                    else
                        return;
                }
            }
            this->_key_available = true;
            if (this->_last_key_input.is_pure_chr()) {
                this->_available = true;
                this->_last_char = this->_last_key_input.chr;
            }
        }
        // loop
    }
};

#endif