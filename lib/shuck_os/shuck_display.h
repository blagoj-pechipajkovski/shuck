#ifndef SHUCK_DISPLAY
#define SHUCK_DISPLAY


class ShuckDisplay : public Print {
  public:
    //from Print// virtual size_t write(uint8_t) = 0;
    
    virtual void cursor_home() = 0;
    virtual void cursor_place(int line, int column) = 0;
    virtual void cursor_move(int h, int v) = 0;
    virtual void cursor_get_line_column(int& line, int& column) = 0;
    
    virtual void clear() = 0;
    
    virtual void color_set(uint8_t r, uint8_t g, uint8_t b) = 0;
    virtual void color_reset() = 0;
};



class ShuckDisplaySimple : public ShuckDisplay {
  private:
    Stream* _stream;
    
  public:
    ShuckDisplaySimple(Stream* stream) {
        this->_stream = stream;
    }
  public:
    size_t write(uint8_t c) override {
        return this->_stream->write(c);
    }
    
    virtual void cursor_home() { }
    
    virtual void cursor_place(int line, int column) { }
    
    virtual void cursor_move(int h, int v) {};//= 0;
    virtual void cursor_get_line_column(int& line, int& column) {};//= 0;
    
    virtual void clear() { }
    
    virtual void color_set(uint8_t r, uint8_t g, uint8_t b) { };//= 0;
    virtual void color_reset() { }
};



class ShuckDisplayTerminal : public ShuckDisplay {
  private:
    Stream* _stream;
    
  public:
    ShuckDisplayTerminal(Stream* stream) {
        this->_stream = stream;
    }
  public:
    size_t write(uint8_t c) override {
        return this->_stream->write(c);
    }
    
    virtual void cursor_home() {
        // Move cursor to top left
        this->_stream->write(ESC);
        this->_stream->write('[');
        this->_stream->write('H');
    }
    
    virtual void cursor_place(int line, int column) {
        if (line <   0) line =   0;
        if (line > 999) line = 999;
        if (column <   0) column =   0;
        if (column > 999) column = 999;
        char   line_str[4];
        char column_str[4];
        sprintf(line_str, "%d", line);
        sprintf(column_str, "%d", column);
        
        this->_stream->write(ESC);
        this->_stream->write('[');
        this->_stream->print(line_str);
        this->_stream->write(';');
        this->_stream->print(column_str);
        this->_stream->write('H');
    }
    
    virtual void cursor_move(int h, int v) {};//= 0;
    virtual void cursor_get_line_column(int& line, int& column) {};//= 0;
    
    virtual void clear() {
        this->_stream->write(ESC);
        this->_stream->write('[');
        this->_stream->write('2');
        this->_stream->write('J');
    }
    
    virtual void color_set(uint8_t r, uint8_t g, uint8_t b) {};//= 0;
    virtual void color_reset() {
        this->_stream->write(ESC);
        this->_stream->write('[');
        this->_stream->write('0');
        this->_stream->write('m');
    }
};

#endif