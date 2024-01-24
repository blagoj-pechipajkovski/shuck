#ifndef SHUCK_CONSOLE
#define SHUCK_CONSOLE

#include "shuck_keyboard.h"
#include "shuck_display.h"


class PrintDummy : public Print {
    size_t write(uint8_t) { return 0; }
} print_dummy;

class StreamDummy : public Stream {
    // Print
    size_t write(uint8_t) { return 0; }
    
    // Stream
    int available() { return 0; }
    int read() { return -1; }
    int peek() { return -1; }
} stream_dummy;

class ShuckKeyboardDummy : public ShuckKeyboard {
    // Print
    size_t write(uint8_t) { return 0; }
    
    // Stream
    int available() { return 0; }
    int read() { return -1; }
    int peek() { return -1; }
    
    // ShuckKeyboard
    bool key_available() { return false; }
    void key_peek(KeyInput& key_input) {}
    void key_read(KeyInput& key_input) {}
    
    virtual void reset() {}
} shuck_keyboard_dummy;

class ShuckDisplayDummy : public ShuckDisplay {
    // Print
    size_t write(uint8_t) { return 0; }
    
    // ShuckDisplay
    void cursor_home() {}
    void cursor_place(int line, int column) {}
    void cursor_move(int h, int v) {}
    void cursor_get_line_column(int& line, int& column) { line = -1; column = -1; }
    
    void clear() {}
    
    void color_set(uint8_t r, uint8_t g, uint8_t b) {}
    void color_reset() {}
} shuck_display_dummy;



class ShuckConsole {
  public:
    static bool is_safe_char(char c) {
        return isprint(c) || c == ShuckOS::line_terminator;
    }
    
    enum class Status {
        NORMAL,
        CANCEL,
        CLS
    };
    
    void print_event(const char* from, const char* event) {
        user.printf("%s : %s", from, event);
        user.print("\n");
    }
    template <typename... DescriptionStrs>
    void print_event(const char* from, const char* event, DescriptionStrs... description_strs) {
        user.printf("%s : %s", from, event);
        user.print(" - ");
        for(const auto str : {description_strs...}) {
            if (str == nullptr)
                continue;
            user.printf("%s ", str);
        }
        user.print("\n");
    }

    template <typename... Strs>
    void print_usage(Strs... strs) {
        user.printf("Usage:\n");
        int i = 0;
        for(const auto str : {strs...}) {
            if (str == nullptr)
                continue;
            user.printf("  %s\n", str);
            i++;
        }
    }
    
  private:
    ShuckConsole::Status _status;
  public:
    void status_reset() {
        this->_status = ShuckConsole::Status::NORMAL;
    }
    
    ShuckConsole::Status status() {
        ShuckConsole::Status ret_status = this->_status;
        this->status_reset();
        return ret_status;
    }
    
    void loop() {
        
        this->keyboard.loop();
        
        if (this->keyboard.key_available()) {
            
            ShuckKeyboard::KeyInput key;
            this->keyboard.key_peek(key);
            
            
            if (key.is(MOD_KEY_CTRL, 'c') || key.is(MOD_KEYS_NONE, ShuckKeyboard::NonChrKeys::ESCAPE)) {
                this->_status = ShuckConsole::Status::CANCEL;
                this->keyboard.key_read(key); // consume
            }
            else if (key.is(MOD_KEY_CTRL, 'l')) {
                this->display.clear();
                this->display.cursor_home();
                
                this->keyboard.key_read(key);
                this->_status = ShuckConsole::Status::CLS;
            }
        }
    }
    
  public:
    Stream& user = stream_dummy;
    
    Stream&              in =         stream_dummy;
    ShuckKeyboard& keyboard = shuck_keyboard_dummy; // inherits from Stream
    
    Print&               out =         print_dummy;
    Stream&       out_stream =        stream_dummy; // inherits from Print
    ShuckDisplay&    display = shuck_display_dummy; // inherits from Print
    
    
    // IN as ShuckKeyboard check
    bool keyboard_available() {
        return (&keyboard != &shuck_keyboard_dummy);
    }
    
    ////////////////////
    
    // OUT as Stream check
    bool out_stream_available() {
        return (&out_stream != &stream_dummy);
    }
    
    // OUT as ShuckDisplay check
    bool display_available() {
        return (&display != &shuck_display_dummy);
    }


    /////////////////////////////////////////////////// 1
    ShuckConsole(
        void* in_ptr,
        void* out_ptr,
        void* user_ptr
    )
    { }
    /////////////////////////
    ShuckConsole(
        void* in_ptr,
        void* out_ptr,
        Stream& new_user
    )
    :
        user(new_user)
    { }

    /////////////////////////////////////////////////// 2
    ShuckConsole(
        void* in_ptr,
        Print& new_out,
        void* user_ptr
    )
    :
        out(new_out)
    { }
    /////////////////////////
    ShuckConsole(
        void* in_ptr,
        Print& new_out,
        Stream& new_user
    )
    :
        out(new_out),
        user(new_user)
    { }

    /////////////////////////////////////////////////// 3
    ShuckConsole(
        void* in_ptr,
        Stream& new_out,
        void* user_ptr
    )
    :
        out(new_out), out_stream(new_out)
    { }
    /////////////////////////
    ShuckConsole(
        void* in_ptr,
        Stream& new_out,
        Stream& new_user
    )
    :
        out(new_out), out_stream(new_out),
        user(new_user)
    { }

    /////////////////////////////////////////////////// 4
    ShuckConsole(
        void* in_ptr,
        ShuckDisplay& new_out,
        void* user_ptr
    )
    :
        out(new_out), display(new_out)
    { }
    /////////////////////////
    ShuckConsole(
        void* in_ptr,
        ShuckDisplay& new_out,
        Stream& new_user
    )
    :
        out(new_out), display(new_out),
        user(new_user)
    { }

    /////////////////////////////////////////////////// 5
    ShuckConsole(
        Stream& new_in,
        void* out_ptr,
        void* user_ptr
    )
    :
        in(new_in)
    { }
    /////////////////////////
    ShuckConsole(
        Stream& new_in,
        void* out_ptr,
        Stream& new_user
    )
    :
        in(new_in),
        user(new_user)
    { }

    /////////////////////////////////////////////////// 6
    ShuckConsole(
        Stream& new_in,
        Print& new_out,
        void* user_ptr
    )
    :
        in(new_in),
        out(new_out)
    { }
    /////////////////////////
    ShuckConsole(
        Stream& new_in,
        Print& new_out,
        Stream& new_user
    )
    :
        in(new_in),
        out(new_out),
        user(new_user)
    { }

    /////////////////////////////////////////////////// 7
    ShuckConsole(
        Stream& new_in,
        Stream& new_out,
        void* user_ptr
    )
    :
        in(new_in),
        out(new_out), out_stream(new_out)
    { }
    /////////////////////////
    ShuckConsole(
        Stream& new_in,
        Stream& new_out,
        Stream& new_user
    )
    :
        in(new_in),
        out(new_out), out_stream(new_out),
        user(new_user)
    { }

    /////////////////////////////////////////////////// 8
    ShuckConsole(
        Stream& new_in,
        ShuckDisplay& new_out,
        void* user_ptr
    )
    :
        in(new_in),
        out(new_out), display(new_out)
    { }
    /////////////////////////
    ShuckConsole(
        Stream& new_in,
        ShuckDisplay& new_out,
        Stream& new_user
    )
    :
        in(new_in),
        out(new_out), display(new_out),
        user(new_user)
    { }

    /////////////////////////////////////////////////// 9
    ShuckConsole(
        ShuckKeyboard& new_in,
        void* out_ptr,
        void* user_ptr
    )
    :
        in(new_in), keyboard(new_in)
    { }
    /////////////////////////
    ShuckConsole(
        ShuckKeyboard& new_in,
        void* out_ptr,
        Stream& new_user
    )
    :
        in(new_in), keyboard(new_in),
        user(new_user)
    { }

    /////////////////////////////////////////////////// 10
    ShuckConsole(
        ShuckKeyboard& new_in,
        Print& new_out,
        void* user_ptr
    )
    :
        in(new_in), keyboard(new_in),
        out(new_out)
    { }
    /////////////////////////
    ShuckConsole(
        ShuckKeyboard& new_in,
        Print& new_out,
        Stream& new_user
    )
    :
        in(new_in), keyboard(new_in),
        out(new_out),
        user(new_user)
    { }

    /////////////////////////////////////////////////// 11
    ShuckConsole(
        ShuckKeyboard& new_in,
        Stream& new_out,
        void* user_ptr
    )
    :
        in(new_in), keyboard(new_in),
        out(new_out), out_stream(new_out)/*, display(new_out),*/
    { }
    /////////////////////////
    ShuckConsole(
        ShuckKeyboard& new_in,
        Stream& new_out,
        Stream& new_user
    )
    :
        in(new_in), keyboard(new_in),
        out(new_out), out_stream(new_out), /*display(new_out),*/
        user(new_user)
    { }

    /////////////////////////////////////////////////// 12
    ShuckConsole(
        ShuckKeyboard& new_in,
        ShuckDisplay& new_out,
        void* user_ptr
    )
    :
        in(new_in), keyboard(new_in),
        out(new_out), /*out_stream(new_out),*/ display(new_out)
    { }
    /////////////////////////
    ShuckConsole(
        ShuckKeyboard& new_in,
        ShuckDisplay& new_out,
        Stream& new_user
    )
    :
        in(new_in), keyboard(new_in),
        out(new_out), /*out_stream(new_out),*/ display(new_out),
        user(new_user)
    { }
};

#endif