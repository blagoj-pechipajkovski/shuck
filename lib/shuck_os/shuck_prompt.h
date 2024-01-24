#ifndef SHUCK_PROMPT_H
#define SHUCK_PROMPT_H

// Suport for:

// 1 | LEFT                 | move cursor 1 left
// 1 | RIGHT                | move cursor 1 right

// 1 | HOME                 | move cursor far left
// 1 | END                  | move cursor far right

// 1 | CTRL + LEFT          | cursor to beginning of previous word
// 1 | CTRL + RIGHT         | cursor to end of next word

// 1 | BACKSPACE            | delete left of cursor
// 1 | DEL                  | delete right of cursor

// 0 | ALT  + BACKSPACE     | delete to beginning of previous word  *! Ctrl+BACKSPACE is same as only BACKSPACE
// 0 | CTRL + DEL           | delete to end of next word

// Feature: command history
// 1 | UP                   | prev command
// 1 | DOWN                 | next command

// Feature: autocompletion
// ? | TAB                  | autocomplete



// NOT FEATURES OF 'ShuckPrompt' (anymore), now of console and interpreter
// 1 | CTRL + l             | clear screen
// 1 | CTRL + c             | kbd interrupt


class ShuckPrompt {
  protected:
    
    class HistoryClass {
      private:
        static const int HISTORY_MAX = 50;
        char* _history_ptrs[HISTORY_MAX];
        int _history_ptrs_count = 0;
        int _history_ptrs_latest = -1;
        
        bool _no_duplicate_history;
        
      public:
        HistoryClass(bool no_duplicate_history=false)
        : _no_duplicate_history(no_duplicate_history)
        { }
        
        void add(const char* line) {
            
            if (this->_no_duplicate_history) {
                if (this->len() > 0) {
                    if (strcmp(line, this->get(-1)) == 0) {
                        return;
                    }
                }
            }
        
            this->_history_ptrs_latest = (this->_history_ptrs_latest + 1) % this->HISTORY_MAX;
            
                   this->_history_ptrs[this->_history_ptrs_latest] = new char[strlen(line)+1];
            strcpy(this->_history_ptrs[this->_history_ptrs_latest], line);
                
            if (this->_history_ptrs_count != this->HISTORY_MAX)
                this->_history_ptrs_count++;
        }
    
        int max() {
            return this->_history_ptrs_count-1;
        }
        
        int min() {
            return -(this->_history_ptrs_count);
        }
        
        const char* get(int i) {
            if (i < min() || i > max()) {
                return nullptr;
            }
            else {
                // magija // https://stackoverflow.com/a/6765091
                // smenato (latest+i+1) bese samo i // isto taka magija
                int pos = (
                    (
                        (
                            this->_history_ptrs_latest+i+1
                        ) % this->_history_ptrs_count
                    ) + this->_history_ptrs_count
                ) % this->_history_ptrs_count;
                
                return this->_history_ptrs[pos];
            }
        }
        
        int len() {
            return this->_history_ptrs_count;
        }
        
    };
    
    bool _history_enabled = true;
  public:
  
    
    HistoryClass history;
    
    
    static const char END_CHAR = ShuckOS::line_terminator;
  protected:

    static const int _MAX_BUFF_LEN = 256;
    char _buff[_MAX_BUFF_LEN];
    int _buff_len = 0;

    bool _available = false;

  public:
    bool available() {
        return _available;
    }
    
    const char* c_str() {
        return _buff;
    }

    bool is(const char* to_cmp) {
        if (!available())
            return false;
        return strcmp(_buff, to_cmp)==0;
    }

  protected:
    void _end_char_rcv() {
        _buff[_buff_len] = '\0';
        if (_buff_len > 0 && _history_enabled)
            history.add(_buff);
        _available = true;
    }


  public:
    void history_enabled(bool he) {
        _history_enabled = he; //TODO// maybe possibly seperate class
    }

    ShuckPrompt(bool no_duplicate_history = true)
    : history(no_duplicate_history)
    { }
    
    
  protected:
    char _reset_reason[64] = {0};
  public:
    virtual void reset() {
        _buff_len = 0;
        _available = false;
    }
    void reset(const char* reset_reason) {
        strcpy(_reset_reason, reset_reason);
        reset();
    }


    virtual void push(char in) {
        
        // Handle end char
        if (in == END_CHAR) {
            _end_char_rcv();
            return;
        }
        
        // Handle addition of char
        _buff[_buff_len] = in;
        _buff_len++;
        
        if (_buff_len == _MAX_BUFF_LEN)
            reset("\nConsole line reached max length");
    }
    
    virtual void loop(ShuckConsole& console) {
        
        console.loop();
        
        if (console.keyboard_available()) {
            if (console.keyboard.key_available()) {
                
                ShuckKeyboard::KeyInput key;
                console.keyboard.key_peek(key);
                
                if (key.is_pure_chr()) {
                    // Key is for us - consume it
                    console.keyboard.key_read(key);
                    
                    console.display.print(key.chr);
                    this->push(key.chr);
                    return;
                }
            }
        }
        else {
            if (console.in.available())
                this->push(console.in.read());
        }
        
    }
};
















class ShuckInteractivePrompt : public ShuckPrompt {
    
  private:
    int history_location = 0;
    int _buff_pos = 0;

    char _tmp_buff[_MAX_BUFF_LEN];
    int _tmp_buff_len = 0;

    
    void load_from_apropriate_history() {
        if (history_location == 0) {
            // restore buffer
            memcpy(_buff, _tmp_buff, _tmp_buff_len);
            _buff_len = _tmp_buff_len;
            _buff_pos = _tmp_buff_len;
        }
        else {
            strcpy(_buff, history.get(history_location));
            _buff_len = strlen(_buff);
            _buff_pos = _buff_len;
        }
    }

    bool _push_control(ShuckKeyboard::KeyInput& key) {
        if (key.is(MOD_KEYS_NONE, ShuckKeyboard::NonChrKeys::ARROW_UP)) {
            if (history.len() == 0) // no history
                return true;
                
            history_location--;
            if (history_location < history.min())
                history_location = history.min();
                
            if (history_location == -1) {
                // save active bufer if starting to go through history
                memcpy(_tmp_buff, _buff, _buff_len);
                _tmp_buff_len = _buff_len;
            }
            
            load_from_apropriate_history();
            return true;
        }
        // --- KEYBOARD: \/
        if (key.is(MOD_KEYS_NONE, ShuckKeyboard::NonChrKeys::ARROW_DOWN)) {
            if (history_location == 0)// maybe unnecesary
                return true;          // maybe unnecesary
            history_location++;
            if (history_location > 0)
                history_location = 0;
            
            load_from_apropriate_history();
            return true;
        }//////////////////
        // --- KEYBOARD: <
        if (key.is(MOD_KEYS_NONE, ShuckKeyboard::NonChrKeys::ARROW_LEFT)) {
            _buff_pos--;
            if (_buff_pos < 0)
                _buff_pos = 0;
            return true;
        }
        // --- KEYBOARD: >
        if (key.is(MOD_KEYS_NONE, ShuckKeyboard::NonChrKeys::ARROW_RIGHT)) {
            _buff_pos++;
            if (_buff_pos > _buff_len)
                _buff_pos = _buff_len;
            return true;
        }
        // --- KEYBOARD: Ctrl + <
        if (key.is(MOD_KEY_CTRL, ShuckKeyboard::NonChrKeys::ARROW_LEFT)) {
            while (true) {
                _buff_pos--;

                if (_buff_pos < 0) {
                    _buff_pos = 0;
                    break;
                }
                
                if (_buff[_buff_pos] != ' ' && _buff[_buff_pos-1] == ' ')
                    break;
            }
            return true;
        }
        // --- KEYBOARD: Ctrl + >
        if (key.is(MOD_KEY_CTRL, ShuckKeyboard::NonChrKeys::ARROW_RIGHT)) {
            while (true) {
                _buff_pos++;

                if (_buff_pos > _buff_len) {
                    _buff_pos = _buff_len;
                    break;
                }
                
                if (_buff[_buff_pos] == ' ' && _buff[_buff_pos-1] != ' ')
                    break;
            }
            return true;
        }
        // --- KEYBOARD: Home
        if (key.is(MOD_KEYS_NONE, ShuckKeyboard::NonChrKeys::HOME)) {
            _buff_pos = 0;
            return true;
        }
        // --- KEYBOARD: End
        if (key.is(MOD_KEYS_NONE, ShuckKeyboard::NonChrKeys::END)) {
            _buff_pos = _buff_len;
            return true;
        }
        // --- KEYBOARD: Backspace
        if (key.is(MOD_KEYS_NONE, ShuckKeyboard::NonChrKeys::BACKSPACE)) {
            if (_buff_len > 0 && _buff_pos > 0) {
                if (_buff_len == _buff_pos) {
                    _buff_len--;
                    _buff_pos--;
                }
                else {
                    int     tmp_len = _buff_len - _buff_pos;
                    char tmp_buff[tmp_len];

                    memcpy(tmp_buff, _buff+_buff_pos, tmp_len);

                    _buff_len--;
                    _buff_pos--;

                    memcpy(_buff+_buff_pos, tmp_buff, tmp_len);
                }
            }
            return true;
        }
        // --- KEYBOARD: Delete
        if (key.is(MOD_KEYS_NONE, ShuckKeyboard::NonChrKeys::DELETE)) {
            if (_buff_len > 0 && _buff_pos != _buff_len) {
                int buffy_possy = _buff_pos + 1;

                int tmp_len = _buff_len - buffy_possy;
                char tmp_buff[tmp_len];

                memcpy(tmp_buff, _buff+buffy_possy, tmp_len);

                _buff_len--;

                memcpy(_buff+_buff_pos, tmp_buff, tmp_len);
            }
            return true;
        }

        return false;
    }


  public:
  private:
    const char* _prompt;
  public:
    void set_prompt(const char* new_prompt) {
        _prompt = new_prompt;
    }
    #define ALLOW_DUPLICATE_HISTORY false
    ShuckInteractivePrompt(const char* new_prompt=nullptr, bool no_duplicate_history = true)
    : ShuckPrompt(no_duplicate_history) {
        set_prompt(new_prompt);
    }
    
  private:
    bool initial_draw_done = false;
  public:
    using ShuckPrompt::reset;
    void reset() override {
        _buff_pos = 0;
        history_location = 0;
        initial_draw_done = false;
        ShuckPrompt::reset();
    }


    void push(char in) override {
        
        // Handle end char
        if (in == END_CHAR) {
            _end_char_rcv();
            return;
        }
        
        // Handle addition of char
        if (_buff_pos == _buff_len) {
            // Addition at end of line
            _buff[_buff_pos] = in;
            _buff_len++;
            _buff_pos++;
        }
        else {
            // Addition in middle of line
            int tmp_len = _buff_len - _buff_pos;
            char tmp_buff[tmp_len];

            memcpy(tmp_buff, _buff+_buff_pos, tmp_len);

            _buff[_buff_pos] = in;
            _buff_len++;
            _buff_pos++;

            memcpy(_buff+_buff_pos, tmp_buff, tmp_len);
        }
        
        if (_buff_len == _MAX_BUFF_LEN)
            reset("\nConsole line reached max length");
    }

    void redraw(ShuckDisplay& display) { //TODO// only works with terminals
        if (strlen(_reset_reason) > 0) {
            display.print('\n');
            display.print(_reset_reason);
            display.print('\n');
            strcpy(_reset_reason, "");
        }
        
        // Clear line
        display.write(ESC);
        display.write('[');
        display.write('2');
        display.write('K');

        // Return cursor to beginning
        display.write('\r');

        // Print prompt if specified
        if (_prompt != nullptr) {
            display.print(_prompt);
        }

        // Write buffer
        const char* p = _buff;
        for (int i=0; i<_buff_len; i++) {
            display.write(*p);
            p++;
        }

        // Bring cursor back if not at end
        if (_buff_pos != _buff_len) {
            display.write(ESC);
            display.write('[');
            char num[8];
            sprintf(num, "%d", _buff_len-_buff_pos);
            for (int i=0; i<strlen(num); i++)
                display.write(num[i]);
            display.write('D');
        }
    }
    
    void loop(ShuckConsole& console) override {
        //TODO// console.keyboard not always available
        if (initial_draw_done == false) {
            this->redraw(console.display);
            initial_draw_done = true;
        }
        
        console.loop();
        
        if (console.status() == ShuckConsole::Status::CLS) {
            this->redraw(console.display);
        }
        
        if (console.keyboard_available()) {
            if (console.keyboard.key_available()) {
                
                ShuckKeyboard::KeyInput key;
                console.keyboard.key_peek(key);
                
                if (key.is_pure_chr()) {
                    // Key is for us - consume it
                    console.keyboard.key_read(key);
                    
                    console.display.print(key.chr);
                    this->push(key.chr);
                }
                else {
                    if (this->_push_control(key))
                        // Handled - consume key
                        console.keyboard.key_read(key);
                }
                
                if (!this->available())
                    this->redraw(console.display);
                
                console.keyboard.reset();
            }
        }
        else {
            if (console.in.available())
                this->push(console.in.read());
        }
        
    }
};






class ShuckPromptAsStream : public Stream {
    ShuckPrompt& _shucky_prompt;

    int _read_position_line = 0;
    int _read_position_char = 0;
    
    int _history_line_limit;
    
  public:
    ShuckPromptAsStream(ShuckPrompt& shucky_prompt, int history_line_limit = -1)
    : _shucky_prompt(shucky_prompt)
    {
        if (history_line_limit >= 0)
            _history_line_limit = history_line_limit;
        else
            _history_line_limit = shucky_prompt.history.max();
    }

    size_t write(uint8_t in) {
        _shucky_prompt.push(in);
        return 1;
    }
    
    
    void reset_read() {
        _read_position_line = 0;
        _read_position_char = 0;
    }
    
    
    int available() {
        if (_read_position_line < min(_shucky_prompt.history.max(), _history_line_limit)) { // everything up to last line in history
            return true;
        }
        else { // last line in history
            if (_read_position_line == min(_shucky_prompt.history.max(), _history_line_limit))
                return _read_position_char <= strlen(_shucky_prompt.history.get(_read_position_line));
            return false;
        }
    }
    
    int peek() {
        if (!available())
            return -1;
        
        int ret;

        const char* current_line = _shucky_prompt.history.get(_read_position_line);

        if (_read_position_char == strlen(current_line)) {
            ret = '\n';
        }
        else {
            ret = current_line[_read_position_char];
        }

        return ret;
    }
    int read() {
        if (!available())
            return -1;
        
        int ret = peek();
        
        _read_position_char++;
        
        if (ret == '\n') {
            _read_position_char = 0;
            _read_position_line++;
        }

        return ret;
    }
};

#endif