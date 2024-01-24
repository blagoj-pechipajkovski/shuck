#ifndef SHUCK_CMD_H
#define SHUCK_CMD_H

#include "shuck_prompt.h"
//#include "shuck_io.h"
#include "shuck_args.h"

#include "ext_expr.h"

float expr_eval(const char* expr_str) {
    //*
    struct expr_func user_funcs[] = { {NULL, NULL, NULL, 0} };
    
    
    struct expr_var_list vars = {0};
    
    struct expr_var *var_pi = expr_var(&vars, "pi", 2);
    var_pi->value = 3.14159265358979323846;
    
    struct expr_var *var_e = expr_var(&vars, "e", 1);
    var_e->value = 2.71828182845904523536;
    
    
    struct expr *e = expr_create(expr_str, strlen(expr_str), &vars, user_funcs);
    
    float ret_val = NAN;
    if (e != NULL) {
        ret_val = expr_eval(e);
    }
    
    expr_destroy(e, &vars);
    
    return ret_val;
    //*/
    //return 1;
}

#define SHUCK_CMD_NOT_FOUND (-2)
#define SHUCK_CMD_INVALID_CMD (-1)
#define SHUCK_CMD_SUCCESS (0)
#define SHUCK_CMD_GENERAL_ERR (1)
#define SHUCK_CMD_USAGE_ERR (2)
#define SHUCK_CMD_USER_CANCEL (3)
#define SHUCK_CMD_TIMEOUT (4)
#define SHUCK_CMD_SUBCMD_ERR (5)
#define SHUCK_CMD_FILE_ERR (6)
#define SHUCK_CMD_MATH_ERR (7)

class ShuckCMD {
  private:
    const char* _name;
    const char* _description;
    int (*_fn)(ShuckArgs& args, ShuckConsole& console);

  public:
    const char* name() {
        return _name;
    }

    const char* description() {
        return _description;
    }
    
    static const char* cmd_ret_to_str(int r) {
        switch (r) {
            case SHUCK_CMD_NOT_FOUND:
                return "SHUCK_CMD_NOT_FOUND";
            case SHUCK_CMD_INVALID_CMD:
                return "SHUCK_CMD_INVALID_CMD";
            case SHUCK_CMD_SUCCESS:
                return "SHUCK_CMD_SUCCESS";
            case SHUCK_CMD_GENERAL_ERR:
                return "SHUCK_CMD_GENERAL_ERR";
            case SHUCK_CMD_USAGE_ERR:
                return "SHUCK_CMD_USAGE_ERR";
            case SHUCK_CMD_USER_CANCEL:
                return "SHUCK_CMD_USER_CANCEL";
            case SHUCK_CMD_TIMEOUT:
                return "SHUCK_CMD_TIMEOUT";
            case SHUCK_CMD_SUBCMD_ERR:
                return "SHUCK_CMD_SUBCMD_ERR";
            case SHUCK_CMD_FILE_ERR:
                return "SHUCK_CMD_FILE_ERR";
            case SHUCK_CMD_MATH_ERR:
                return "SHUCK_CMD_MATH_ERR";
        }
        return "UNKNOWN SHUCK_CMD ERROR";
    }
    
    class List {
      private:
        static const int MAX_COMMANDS = 100;
        static ShuckCMD* array[List::MAX_COMMANDS];
        static int array_size;
        
      public:
        static void add(ShuckCMD* new_cmd) {
            if (List::array_size < MAX_COMMANDS) {
                List::array[array_size] = new_cmd;
                List::array_size++;
            }
        }
        
        static ShuckCMD* find(const char* wanted_name) {
            for (int i = 0 ; i < List::array_size ; i++) {
                ShuckCMD* command = array[i];
                if (strcmp(command->_name, wanted_name)==0)
                    return command;
            }
            return nullptr;
        }
        
        static int len() {
            return List::array_size;
        }
        
        static ShuckCMD* at(int index) {
            if (index >= 0 && index < List::array_size)
                return List::array[index];
            else
                return nullptr;
        }
    };
    
    ShuckCMD(const char* name, const char* description, int (*fn)(ShuckArgs& args, ShuckConsole& console)) {
        _name = name;
        _description = description;
        _fn = fn;
        
        ShuckCMD::List::add(this);
    }
    
    ShuckCMD(const char* name, int (*fn)(ShuckArgs& args, ShuckConsole& console)) {
        _name = name;
        _description = "";
        _fn = fn;
        
        ShuckCMD::List::add(this);
        // for some reason this doesnt work - ShuckCMD(name, "", fn);
    }



    
    class Run {
      private:
        
        static bool cmd_line_is_balanced(const char* cmd_line) {
            
            int c_quotes  = 0;
            int c_vars    = 0;
            int c_cmds    = 0;
            int c_maths   = 0;
            
            int i = 0;
            int cmd_line_len = strlen(cmd_line);
            while (i < cmd_line_len) {
                if (cmd_line[i] == '\\') {
                    i += 2;
                    continue;
                }
                
                if (cmd_line[i] == '"')
                    c_quotes += 1;
                
                if (cmd_line[i] == '{')
                    c_vars += 1;
                if (cmd_line[i] == '}')
                    c_vars -= 1;

                if (cmd_line[i] == '[')
                    c_cmds += 1;
                if (cmd_line[i] == ']')
                    c_cmds -= 1;

                if (cmd_line[i] == '(')
                    c_maths += 1;
                if (cmd_line[i] == ')')
                    c_maths -= 1;
                    
                i += 1;
            }
            
            return (c_quotes % 2 == 0) and (c_vars == 0) and (c_cmds == 0) and (c_maths == 0);
        }
        
        // inserted between arguments
        static char argument_separator;
        
        // Split arguments by spaces
        // Replaces any number of spaces with ASCII group separator
        // Does not do anything to spaces that are inside: quoted strings, variables, subcommands, and math expressions
        // Only works with balanced enclosers
        static void mark_arguments(String& cmd_line) {
            
            int inside_quotes = -1;
            int inside_var    = -1;
            int inside_cmd    = -1;
            int inside_math   = -1;
            
            int proccesing_escaped = 0;
            
            int len = cmd_line.length();
            int i = 0;
            while (i < len) {
                
                if (cmd_line.charAt(i) == '\\') {
                    i += 2;
                    continue;
                }
                
                if (cmd_line.charAt(i) == '"') {
                    if      (inside_quotes == -1)
                        inside_quotes = 0;
                    else if (inside_quotes == 0)
                        inside_quotes = -1;
                }
                
                if (cmd_line.charAt(i) == '{')
                    inside_var += 1;
                if (cmd_line.charAt(i) == '}')
                    inside_var -= 1;

                if (cmd_line.charAt(i) == '[')
                    inside_cmd += 1;
                if (cmd_line.charAt(i) == ']')
                    inside_cmd -= 1;

                if (cmd_line.charAt(i) == '(')
                    inside_math += 1;
                if (cmd_line.charAt(i) == ')')
                    inside_math -= 1;
                
                
                bool is_inside = max(inside_quotes, max(inside_var, max(inside_cmd, inside_math))) != -1;
                
                if   (cmd_line.charAt(i) == ' ' && !is_inside) {
                    cmd_line.setCharAt(i, argument_separator);
                }
                
                i += 1;
            }
            
            // -- Remove multiple arg seps in a row
            
            String arg_sep_double;
            arg_sep_double.reserve(2);
            arg_sep_double.concat(ShuckCMD::Run::argument_separator);
            arg_sep_double.concat(ShuckCMD::Run::argument_separator);
            // => arg_sep_double == "\x1D\x1D"
            
            String arg_sep_single;
            arg_sep_single.reserve(1);
            arg_sep_single.concat(ShuckCMD::Run::argument_separator);
            // => arg_sep_single == "\x1D"
            
            while (cmd_line.indexOf(arg_sep_double) != -1) {
                cmd_line.replace(arg_sep_double, arg_sep_single);
            }
            
            
            // Remove leading arg sep
            if (cmd_line.startsWith(arg_sep_single))
                cmd_line.remove(0, 1);
            
            // Remove trailing arg sep
            if (cmd_line.endsWith(arg_sep_single))
                cmd_line.remove(cmd_line.length(), 1);
        }
        
        // works with '"', '(', '[', '{'
        static void index_of_first_unescaped_opener(const String& cmd_line, int& index, char& opener) {
            int index_of_q;
            int index_of_b;
            int index_of_s;
            int index_of_c;
            
            int start_index = 0;
            
            while (start_index < cmd_line.length()) {
            
                index_of_q = cmd_line.indexOf('"', start_index);
                index_of_b = cmd_line.indexOf('(', start_index);
                index_of_s = cmd_line.indexOf('[', start_index);
                index_of_c = cmd_line.indexOf('{', start_index);
                
                index_of_q = index_of_q != -1 ? index_of_q : cmd_line.length();
                index_of_b = index_of_b != -1 ? index_of_b : cmd_line.length();
                index_of_s = index_of_s != -1 ? index_of_s : cmd_line.length();
                index_of_c = index_of_c != -1 ? index_of_c : cmd_line.length();
                
                int  first_index = min(index_of_q, min(index_of_b, min(index_of_s, index_of_c)));
                char first_index_char = cmd_line.charAt(first_index);
                
                if (cmd_line.charAt(first_index-1) == '\\') {
                    start_index = first_index+1;
                    continue;
                }
                else {
                    opener = cmd_line.charAt(first_index);
                    index = first_index;
                    return;
                }
                
            }
            
            opener = '\0';
            index = -1;
        }
        
        static void index_of_matching_unescaped_closer(const String& cmd_line, int opener_index, int& index) {
            
            char opener = cmd_line.charAt(opener_index);
            
            char closer;
            switch (opener) {
                case '"':
                    closer = '"';
                    break;
                case '(':
                    closer = ')';
                    break;
                case '[':
                    closer = ']';
                    break;
                case '{':
                    closer = '}';
                    break;
                default:
                    return;
                    break;
            }
            
            
            int len = cmd_line.length();
            int i = opener_index+1;
            
            int level = 1;
            
            while (i < len) {
            
                if (cmd_line.charAt(i) == '\\') {
                    i += 2;
                    continue;
                }
                
                if (cmd_line.charAt(i) == closer) // this needs to be first and with elseif to properly parse '"'
                    level -= 1;
                else if (cmd_line.charAt(i) == opener)
                    level += 1;
                
                if (level == 0) {
                    index = i;
                    return;
                }
                
                i +=1;
            }
            
            index = -1;
            return;
        }
        
        struct EncloserAndType {
            char type = '\0';
            int start = -1;
            int stop = -1;
        };
        static void find_first_top_level_encloser_and_type(const String& line, EncloserAndType& output) {
            
            int first_opener_index;
            char first_opener_type;
            
            ShuckCMD::Run::index_of_first_unescaped_opener(line, first_opener_index, first_opener_type);
            
            int closer_index;
            
            ShuckCMD::Run::index_of_matching_unescaped_closer(line, first_opener_index, closer_index);
            
            
            output.type = first_opener_type;
            output.start = first_opener_index;
            output.stop = closer_index;
            
            //TODO// return errors at apropriate places
        }
        
        static bool needs_replacing(const String& cmd_line) {
        
            ShuckCMD::Run::EncloserAndType encloser;
            ShuckCMD::Run::find_first_top_level_encloser_and_type(cmd_line, encloser);
            
            return (encloser.type != '\0');
        }
        
        static int do_suitable_replacement(String& line, ShuckConsole& console);
        static int run_quotes(String& line, ShuckConsole& console);
        static int run_var(String& line, ShuckConsole& console);
      public:
        static int math(String& line, ShuckConsole& console);
        static int cmd(String& line, ShuckConsole& console);
        static int cmd(const char* line, ShuckConsole& console) {
            String l = line;
            return ShuckCMD::Run::cmd(l, console);
        }
    };
};

ShuckCMD* ShuckCMD::List::array[ShuckCMD::List::MAX_COMMANDS];
int       ShuckCMD::List::array_size=0;


// 0x1D is group separator char
char ShuckCMD::Run::argument_separator = '\x1D';
//char ShuckCMD::Run::argument_separator = '#';



int ShuckCMD::Run::do_suitable_replacement(String& line, ShuckConsole& console) {
    
    if (needs_replacing(line)) {
        ShuckCMD::Run::EncloserAndType encloser;
        ShuckCMD::Run::find_first_top_level_encloser_and_type(line.c_str(), encloser);
        
        String operating_string = line.substring(encloser.start+1 , encloser.stop);
        
        int ret = SHUCK_CMD_SUCCESS;
        if (encloser.type == '"')
            ret = ShuckCMD::Run::run_quotes(operating_string, console);
        if (encloser.type == '{')
            ret = ShuckCMD::Run::run_var(operating_string, console);
        if (encloser.type == '[') {
            ShuckPrompt cmd_out;
            ShuckPromptAsStream cmd_out_stream = ShuckPromptAsStream(cmd_out);
            ShuckConsole cmd_console(nullptr, cmd_out_stream, console.user);
            
            int cmd_ret = ShuckCMD::Run::cmd(operating_string, cmd_console);
            
            if (cmd_ret != SHUCK_CMD_SUCCESS) {
                console.print_event("<ShuckCMD>", ShuckCMD::cmd_ret_to_str(cmd_ret));
                return SHUCK_CMD_SUBCMD_ERR;
            }
            if (cmd_out.history.len() != 0)
                operating_string = String(cmd_out.history.get(0));
            else
                operating_string = "";
        }
        if (encloser.type == '(')
            ret = ShuckCMD::Run::math(operating_string, console);
        
        if (ret != SHUCK_CMD_SUCCESS)
            return ret;
            
        line = line.substring(0, encloser.start) + operating_string + line.substring(encloser.stop+1);
    }
    return SHUCK_CMD_SUCCESS;
}

int ShuckCMD::Run::run_quotes(String& qoutes_content, ShuckConsole& console) {
    return SHUCK_CMD_SUCCESS;
}

int ShuckCMD::Run::run_var(String& var_innards, ShuckConsole& console) {
    if (!ShuckCMD::Run::cmd_line_is_balanced(var_innards.c_str()))
        return SHUCK_CMD_INVALID_CMD;

    while (needs_replacing(var_innards)) {
        int ret = do_suitable_replacement(var_innards, console);
        if (ret != SHUCK_CMD_SUCCESS)
            return ret;
    }
    
    var_innards.trim();
    const char* var_contents = getenv(var_innards.c_str());
    if (var_contents)
        var_innards = String(var_contents);
    else
        var_innards = "UNKNOWNVAR";
    
    return SHUCK_CMD_SUCCESS;
}

int ShuckCMD::Run::math(String& math_expr, ShuckConsole& console) {
    math_expr.trim();
    
    if (math_expr.length() == 0) {
        math_expr = "";
        return SHUCK_CMD_SUCCESS;
    }
    
    if (!ShuckCMD::Run::cmd_line_is_balanced(math_expr.c_str()))
        return SHUCK_CMD_INVALID_CMD;
    
    while (needs_replacing(math_expr)) {
        int ret = do_suitable_replacement(math_expr, console);
        if (ret != SHUCK_CMD_SUCCESS)
            return ret;
    }
    
    double result = expr_eval(math_expr.c_str());

    if (isnan(result)) {
        /*console.print_event("<ShuckCMD Math>", "Error", "Math calculation error");
        console.user.print(math_expr);
        console.user.print("\n");*/
        /*for (int i=0; i<err; i++)
            console.user.print(' ');
        console.user.print("^\n");*/
        
        return SHUCK_CMD_MATH_ERR;
    }
    
    if (abs(abs(result) - floor(abs(result))) < 0.01)
        math_expr = String(result, 0);
    else
        math_expr = String(result, 2);
    math_expr.trim();

    return SHUCK_CMD_SUCCESS;
}


int ShuckCMD::Run::cmd(String& user_linee, ShuckConsole& console) {
    String user_line = user_linee;
    user_line.trim();
    
    if (user_line.length() == 0)
        return SHUCK_CMD_SUCCESS;
    
    if (!ShuckCMD::Run::cmd_line_is_balanced(user_line.c_str()))
        return SHUCK_CMD_INVALID_CMD;
    
    ShuckCMD::Run::mark_arguments(user_line); // Remains same size or shrinks
    
    while ( needs_replacing(user_line) && ShuckCMD::Run::cmd_line_is_balanced(user_line.c_str()) ) {
        int ret = do_suitable_replacement(user_line, console);
        if (ret != SHUCK_CMD_SUCCESS)
            return ret;
    }
    // ILI ne escepovaj subcomandi X
    // ILI check za balance        <-
    if (!ShuckCMD::Run::cmd_line_is_balanced(user_line.c_str())) {
        user_line.replace(ShuckCMD::Run::argument_separator, ' ');
        console.print_event("<ShuckCMD>", "Error", "Command became unbalanced:", user_line.c_str());
        return SHUCK_CMD_INVALID_CMD;
    }
    
    ShuckArgs args;
    
    if (args.generate_from(user_line.c_str(), ShuckCMD::Run::argument_separator) == 0)
        return SHUCK_CMD_INVALID_CMD;
    
    args.escape_generated();
    
    int cmd_ret = SHUCK_CMD_NOT_FOUND;
    
    ShuckCMD* command = ShuckCMD::List::find(args.arg(0));
    
    if (command != nullptr) {
        cmd_ret = command->_fn(args, console);
    }

    if (cmd_ret == SHUCK_CMD_NOT_FOUND) {
        console.print_event("<ShuckCMD>", "Error", "Command not found:", args.arg(0));
    }
    
    return cmd_ret;
}


#endif





