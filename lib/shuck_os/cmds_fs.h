#if defined(ESP_PLATFORM)
    #include "FS.h"
    #include <LittleFS.h>
    FS& FILESYSTEM = LittleFS;
#endif
#define FILE_READ       "r"
#define FILE_WRITE      "w"
#define FILE_APPEND     "a"

void begin_FS() {


    if (!LittleFS.begin()) {
        Serial.printf("Unable to LittleFS.begin()\n");
        
        if (!LittleFS.format()) {
            Serial.printf("LittleFS.format()\n");
        }

        if (!LittleFS.begin()) {
            Serial.println("Still no LittleFS.begin()");
        }
    }
}


#define MAX_FS_PATH_LEN 32

char cwd[MAX_FS_PATH_LEN] = "/";
bool full_path_from_arg(const char* arg, char* out_path) {

    // if first line / absolute path
    if (strchr(arg, '/') == arg) {
        strcpy(out_path, arg);
    }
    else {
        strcpy(out_path, cwd);
        if (out_path[strlen(out_path)-1] != '/')
            strcat(out_path, "/");
        strcat(out_path, arg);
    }


    return true;
}
/*ShuckCMD fs_cd_cmd("cd", [](ShuckArgs& args, ShuckPrompt& cl) {
    if (args.has(1)) {
        char path[MAX_FS_PATH_LEN];
        full_path_from_arg(args.arg(1), path);
        char* last_chr_in_path = &path[strlen(path)-1];
        while (last_chr_in_path != path && *last_chr_in_path == '/') {
            *last_chr_in_path = 0;
            last_chr_in_path--;
        }        
        File fod = FILESYSTEM.open(path);
        if(!fod) {
            Serial.println("FAILED - unable to open directory");
            return;
        }
        if (fod.isDirectory()) {
            strcpy(cwd, path);
        }
        else {
            stream.println("FAILED - not directory");
        }
        fod.close();
    }
    else {
        strcpy(cwd, "/");
    }
});*/
ShuckCMD fs_pwd_cmd("pwd", [](ShuckArgs& args, ShuckConsole& console) -> int {
    console.out.printf("%s\n", cwd);
    return SHUCK_CMD_SUCCESS;
});

int count_chars_in_str(char chr, const char* str, int len=-1) {
    int count = 0;

    if (len == -1)
        len = strlen(str);
    
    for (int i=0; i<len; i++) {
        if (str[i]==chr)
            count++;
    }

    return count;
}

ShuckCMD fs_truch_cmd("truch", "TRuncate/toUCH(Create) file", [](ShuckArgs& args, ShuckConsole& console) -> int {
    const char usage[] = "truch [file]";
    if (!args.has(1)) {
        console.print_event(args.arg(0), "Error", "You have to specify one file");
        console.print_usage(usage);
        return SHUCK_CMD_USAGE_ERR;
    }
    
    char file_path[MAX_FS_PATH_LEN];
    if (!full_path_from_arg(args.arg(1), file_path)) {
        console.print_event(args.arg(0), "Error", "File path error");
        console.print_usage(usage);
        return SHUCK_CMD_FILE_ERR;
    }

    console.print_event(args.arg(0), "File", "Truncating/Creating file:", (const char*)file_path);

    File file = FILESYSTEM.open(file_path, FILE_WRITE);
    if(!file) {
        console.print_event(args.arg(0), "Error", "unable to create/truncate file");
        return SHUCK_CMD_FILE_ERR;
    }
    file.close();
    console.print_event(args.arg(0), "Success", "file", (const char*)file_path, "exists and contains 0B");
    return SHUCK_CMD_SUCCESS;
});
ShuckCMD fs_ls_cmd("ls", "List files", [](ShuckArgs& args, ShuckConsole& console) -> int {


    #if defined(ESP_PLATFORM) || defined(ARDUINO_ARCH_ESP8266)
    File root = FILESYSTEM.open("/");
    #else
    File root = FILESYSTEM.open("/", FILE_READ);
    #endif

    if(!root) {
        console.print_event(args.arg(0), "Error", "unable to open directory '/'");
        return SHUCK_CMD_FILE_ERR;
    }
    if(!root.isDirectory()){ 
        console.print_event(args.arg(0), "Error", "'/' not a directory");
        return SHUCK_CMD_FILE_ERR;
    }

    File file = root.openNextFile();
    while(file) {
        if(file.isDirectory()) {
            console.out.print(file.name());
            console.out.println("/");
        }
        else {
            #if defined(ARDUINO_ARCH_RP2040)
            console.out.print(file.name());
            #else
            console.out.print(file.path());
            #endif
            console.out.print(", ");
            console.out.println(file.size());
        }
        file = root.openNextFile();
    }
    return SHUCK_CMD_SUCCESS;
});
ShuckCMD fs_rm_cmd("rm", "Remove file", [](ShuckArgs& args, ShuckConsole& console) -> int {
    const char usage[] = "rm [FILE]";
    if (!args.has(1)) {
        console.print_event(args.arg(0), "Error", "You have to specify one file");
        console.print_usage(usage);
        return SHUCK_CMD_USAGE_ERR;
    }

    char file_path[MAX_FS_PATH_LEN];
    if (!full_path_from_arg(args.arg(1), file_path)) {
        console.print_event(args.arg(0), "Error", "File path error");
        console.print_usage(usage);
        return SHUCK_CMD_FILE_ERR;
    }

    console.print_event(args.arg(0), "File", "Deleting file:", (const char*)file_path);

    /*if (!FILESYSTEM.exists(file_path)) {
        console.out.println("ERROR - file does not exist");
        return;
    }*/

    if (FILESYSTEM.remove(file_path)) {
        console.print_event(args.arg(0), "Success");
        return SHUCK_CMD_SUCCESS;
    }
    else {
        console.print_event(args.arg(0), "Error", "something went wrong");
        return SHUCK_CMD_FILE_ERR;
    }
});
ShuckCMD fs_echof_cmd("echof", "Echo to file", [](ShuckArgs& args, ShuckConsole& console) -> int {

    const char usage[] = "echof [FILE] [STRING]";

    if (!args.has(1)) {
        console.print_event(args.arg(0), "Error", "you have to specify at least one argument");
        console.print_usage(usage);
        return SHUCK_CMD_USAGE_ERR;
    }

    char file_path[MAX_FS_PATH_LEN];
    if (!full_path_from_arg(args.arg(1), file_path)) {
        console.print_event(args.arg(0), "Error", "File path error");
        console.print_usage(usage);
        return SHUCK_CMD_FILE_ERR;
    }

    console.print_event(args.arg(0), "File", " Appending to file:", (const char*)file_path);

    File file = FILESYSTEM.open(file_path, FILE_APPEND);
    if (!file) {
        console.print_event(args.arg(0), "Error", "Unable to open file for reading");
        return SHUCK_CMD_FILE_ERR;
    }
    if (file.isDirectory()){
        console.print_event(args.arg(0), "Error", "Cannot append to directory");
        return SHUCK_CMD_FILE_ERR;
    }

    char message[64];
    strcpy(message, "");
    if (args.has(2))
        strcpy(message, args.arg(2));
    strcat(message, "\n");

    if (!file.print(message)) {
        console.print_event(args.arg(0), "Error", "something went wrong");
        file.close();
        return SHUCK_CMD_FILE_ERR;
    }

    console.print_event(args.arg(0), "Success");
    file.close();
    return SHUCK_CMD_SUCCESS;
});
ShuckCMD fs_type_cmd("type", "Read file contents", [](ShuckArgs& args, ShuckConsole& console) -> int {

    const char usage[] = "type [FILE]";
    if (!args.has(1)) {
        console.print_event(args.arg(0), "Error", "you have to specify at least one file");
        console.print_usage(usage);
        return SHUCK_CMD_USAGE_ERR;
    }

    char file_path[MAX_FS_PATH_LEN];
    if (!full_path_from_arg(args.arg(1), file_path)) {
        console.print_event(args.arg(0), "Error", "File path error");
        console.print_usage(usage);
        return SHUCK_CMD_FILE_ERR;
    }

    /*Serial.print("Reading file: ");
    Serial.println(file_path);*/

    File file = FILESYSTEM.open(file_path, FILE_READ);
    if (!file) {
        console.print_event(args.arg(0), "Error", "Unable to open file for reading");
        return SHUCK_CMD_FILE_ERR;
    }
    if (file.isDirectory()) {
        console.print_event(args.arg(0), "Error", "Cannot read directory");
        return SHUCK_CMD_FILE_ERR;
    }

    while(file.available()) {
        console.out.write(file.read());
    }
    file.close();
    return SHUCK_CMD_SUCCESS;
});

char le_file[MAX_FS_PATH_LEN];
bool le_file_opened = false;

enum class range_decoder_result {
    success_single,
    success_range,
    err_non_numeric,
    err_range_decode,
    err_input,
    err_unknown
};

bool check_str_if_integer(const char* str)  {
    if (str == nullptr)
        return false;
    for (int i=0; i<strlen(str); i++) {
        if (isdigit(str[i]) == 0) { // not numeric
            return false;
        }
    }
    return true;
};
// get range from the form of 'x,x' or 'x,' or ',x' or 'x'
// always returns zero indexed
// if input is not zero indexed set zero_indexed=false
range_decoder_result le_range_decoder(const char* str, int& lower, int& upper, bool zero_indexed=true) {
    
    if (count_chars_in_str(',', str) == 0) {
        if (check_str_if_integer(str)) {
            lower = atoi(str)-1;
            upper = atoi(str);
            return range_decoder_result::success_single;
        }
        else
            return range_decoder_result::err_non_numeric;
    }
    else if (count_chars_in_str(',', str) == 1) {
        char str1[strlen(str)+1];
        strcpy(str1, str);
        char* str2;

        char* comma = strchr(str1, ',');
        *comma = '0';
        if (!check_str_if_integer(str1)) {
            return range_decoder_result::err_non_numeric;
        }
        *comma = '\0';
        str2 = comma+1;

        // 'x,'
        if (str1[0] != '\0' && str2[0] == '\0') {
            lower = atoi(str1)-1;
            return range_decoder_result::success_range;
        }
        // ',x'
        else if (str1[0] == '\0' && str2[0] != '\0') {
            upper = atoi(str2);
            return range_decoder_result::success_range;
        }
        // 'x,x'
        else if (str1[0] != '\0' && str2[0] != '\0') {
            lower = atoi(str1)-1;
            upper = atoi(str2);
            return range_decoder_result::success_range;
        }
        else
            return range_decoder_result::err_range_decode;
    }
    else
        return range_decoder_result::err_input;

    return range_decoder_result::err_unknown;
}

ShuckCMD le_cmd("le", "File line editor", [](ShuckArgs& args, ShuckConsole& console) -> int {

    if (args.argc() == 1) {
        if (le_file_opened)
            console.user.printf("Editing %s\n", le_file);
        else
            console.user.println("No file opened");
        
        return SHUCK_CMD_SUCCESS;
    }

    if (args.arg_is(1, "open") || args.arg_is(1, "o")) {
        if (!args.has(2)) {
            console.print_event(args.arg(0), "Error", "You have to specify a file");
            return SHUCK_CMD_FILE_ERR;
        }

        if (!full_path_from_arg(args.arg(2), le_file)) {
            console.print_event(args.arg(0), "Error", "Path error");
            return SHUCK_CMD_FILE_ERR;
        }
        
        le_file_opened = true;

        return SHUCK_CMD_SUCCESS;
    }

    if (!le_file_opened) {
        console.print_event(args.arg(0), "Error", "No file opened");
        return SHUCK_CMD_FILE_ERR;
    }

    if (args.arg_is(1, "read") || args.arg_is(1, "r")) {

        int wanted_lines_start = 0;
        int wanted_lines_end   = -1;

        if (args.has(2)) {
            range_decoder_result rdr = le_range_decoder(args.arg(2), wanted_lines_start, wanted_lines_end, false);
            if (rdr != range_decoder_result::success_single || rdr != range_decoder_result::success_range) {
                console.print_event(args.arg(0), "Error", "Range decoding error");
                return SHUCK_CMD_FILE_ERR;
            }
        }

        File file = FILESYSTEM.open(le_file, FILE_READ);
        if (!file) {
            console.print_event(args.arg(0), "Error", "Unable to open file for reading");
            return SHUCK_CMD_FILE_ERR;
        }

        int file_size = file.size();
        char file_contents[file_size+1]; // will be modified
        int file_pos = 0;
        while(file.available())
            file_contents[file_pos++] = file.read();
        file.close();
        file_contents[file_pos] = '\0';
        
        int num_lines = count_chars_in_str('\n', file_contents, file_size);

        if (num_lines == 0) {
            console.print_event(args.arg(0), "Error", "File empty or not text");
            return SHUCK_CMD_FILE_ERR;
        }

        char* line_ptrs[num_lines]; // points to line beginnings

        line_ptrs[0] = file_contents;
        for (int i=1; i<num_lines; i++) {
            char* nl_ptr = strchr(line_ptrs[i-1], '\n');
            *nl_ptr = '\0';
            line_ptrs[i] = nl_ptr+1;
        }
        *strchr(line_ptrs[num_lines-1], '\n') = '\0';

        if (wanted_lines_end == -1)
            wanted_lines_end = num_lines;
        
        if (wanted_lines_start < 0) {
            console.print_event(args.arg(0), "Error", "Index under range");
            return SHUCK_CMD_GENERAL_ERR;
        }
        if (wanted_lines_end > num_lines) {
            console.print_event(args.arg(0), "Error", "Index over range");
            return SHUCK_CMD_GENERAL_ERR;
        }

        for (int i=wanted_lines_start; i<wanted_lines_end; i++)
            console.out.printf("%03d | %s\n", i+1, line_ptrs[i]);

        return SHUCK_CMD_SUCCESS;
    }

    if (args.arg_is(1, "append") || args.arg_is(1, "a")) {
        // Could be added to write/delete but more efficient this way

        File file = FILESYSTEM.open(le_file, FILE_APPEND);
        if (!file) {
            console.print_event(args.arg(0), "Error", "Unable to open file for reading");
            return SHUCK_CMD_FILE_ERR;
        }

        int arg_num = 2;
        while (args.has(arg_num)) {
            file.print(args.arg(arg_num));
            if (args.has(arg_num+1))
                file.print(' ');
            arg_num++;
        }
        file.write('\n');
        file.close();

        return SHUCK_CMD_SUCCESS;
    }

    if (args.arg_is(1, "write") || args.arg_is(1, "w") ||
        args.arg_is(1, "delete") || args.arg_is(1, "d") ||
        args.arg_is(1, "insert") || args.arg_is(1, "i"))
        {
        
        int wanted_line = -10;

        if (args.has(2)) {
            int _discard;
            range_decoder_result rdr = le_range_decoder(args.arg(2), wanted_line, _discard, false);
            if (rdr != range_decoder_result::success_single) {
                console.print_event(args.arg(0), "Error", "Invalid line");
                return SHUCK_CMD_FILE_ERR;
            }
        }

        File file = FILESYSTEM.open(le_file, FILE_READ);
        if (!file) {
            console.print_event(args.arg(0), "Error", "Unable to open file for reading");
            return SHUCK_CMD_FILE_ERR;
        }
        
        int file_size = file.size();
        char file_contents[file_size+1]; // will be modified
        int file_pos = 0;
        while(file.available())
            file_contents[file_pos++] = file.read();
        file.close();
        file_contents[file_pos] = '\0';
        
        int num_lines = count_chars_in_str('\n', file_contents, file_size);

        if (num_lines == 0) {
            console.print_event(args.arg(0), "Error", "File empty or not text");
            return SHUCK_CMD_FILE_ERR;
        }

        char* line_ptrs[num_lines]; // points to line beginnings

        line_ptrs[0] = file_contents;
        for (int i=1; i<num_lines; i++) {
            char* nl_ptr = strchr(line_ptrs[i-1], '\n');
            *nl_ptr = '\0';
            line_ptrs[i] = nl_ptr+1;
        }
        *strchr(line_ptrs[num_lines-1], '\n') = '\0';

        if (wanted_line == -10)
            wanted_line = num_lines;
        
        if (wanted_line < 0) {
            console.print_event(args.arg(0), "Error", "Index under range");
            return SHUCK_CMD_GENERAL_ERR;
        }
        if (wanted_line > num_lines) {
            console.print_event(args.arg(0), "Error", "Index over range");
            return SHUCK_CMD_GENERAL_ERR;
        }

        
        file = FILESYSTEM.open(le_file, FILE_WRITE);

        int current_line = 0;
        while (current_line<wanted_line) {
            file.print(line_ptrs[current_line]);
            file.print('\n');
            current_line++;
        }

        if (args.arg_is(1, "write") || args.arg_is(1, "w") || args.arg_is(1, "insert") || args.arg_is(1, "i")) {
            int arg_num = 3;
            while (args.has(arg_num)) {
                file.print(args.arg(arg_num));
                if (args.has(arg_num+1))
                    file.print(' ');
                arg_num++;
            }
            file.print('\n');
        }
        if (!(args.arg_is(1, "insert") || args.arg_is(1, "i")))
            current_line++;

        if (current_line < num_lines) {
            while (current_line<num_lines) {
                file.print(line_ptrs[current_line]);
                file.print('\n');
                current_line++;
            }
        }
        file.close();

        return SHUCK_CMD_SUCCESS;
    }

    return SHUCK_CMD_GENERAL_ERR;
});


ShuckCMD run_cmd("run", "Runs a script from file", [](ShuckArgs& args, ShuckConsole& console) -> int {
    const char usage[] = "run [file] (if exists)";
    if (!args.has(1)) {
        console.print_event(args.arg(0), "Error", "you have to specify [file]");
        console.print_usage(usage);
        return SHUCK_CMD_USAGE_ERR;
    }

    char file_path[MAX_FS_PATH_LEN];
    if (!full_path_from_arg(args.arg(1), file_path)) {
        console.print_event(args.arg(0), "Error", "File path error");
        console.print_usage(usage);
        return SHUCK_CMD_FILE_ERR;
    }

    if (!FILESYSTEM.exists(file_path)) {
        if (args.arg_is(2, "if") && args.arg_is(3, "exists")) {
            return SHUCK_CMD_SUCCESS;
        }
        else {
            console.print_event(args.arg(0), "Error", "File does not exist", (const char*)file_path);
            return SHUCK_CMD_FILE_ERR;
        }
    }

    File file = FILESYSTEM.open(file_path, FILE_READ);
    if(!file){
        console.print_event(args.arg(0), "Error", "unable to open file for reading");
        return SHUCK_CMD_FILE_ERR;
    }

    ShuckPrompt line;
    ShuckConsole script_console(file, console.out, console.user);
    while (true) {
        if (!file.available())
            break;
        read_line_result_t line_result = ShuckConsole::read_line(line, file, console.dummy);
        
        if (line_result == read_line_result_t::LINE_READ) {
            if (line.is("exit"))
                break;
            int cmd_ret = ShuckCMD::Run::cmd(line, script_console);
            if (cmd_ret != SHUCK_CMD_SUCCESS) {
                console.print_event(args.arg(0), ShuckCMD::cmd_ret_to_str(cmd_ret));
                return SHUCK_CMD_SUBCMD_ERR;
            }
            line.reset();
        }
        if (!file.available())
            break;
        delay(1);
    }
    
    return SHUCK_CMD_SUCCESS;
});

ShuckCMD rtof_cmd("rtof", "Raw input to file", [](ShuckArgs& args, ShuckConsole& console) -> int {

    const char usage[] = "rtof [FILE]";

    if (!args.has(1)) {
        console.print_event(args.arg(0), "Error", "you have to specify at least one argument");
        console.print_usage(usage);
        return SHUCK_CMD_USAGE_ERR;
    }

    char file_path[MAX_FS_PATH_LEN];
    if (!full_path_from_arg(args.arg(1), file_path)) {
        console.print_event(args.arg(0), "Error", "File path error");
        console.print_usage(usage);
        return SHUCK_CMD_FILE_ERR;
    }

    File file = FILESYSTEM.open(file_path, FILE_WRITE);
    if (!file) {
        console.print_event(args.arg(0), "Error", "Unable to open file for reading");
        return SHUCK_CMD_FILE_ERR;
    }
    if (file.isDirectory()){
        console.print_event(args.arg(0), "Error", "Cannot append to directory");
        return SHUCK_CMD_FILE_ERR;
    }

    while (console.user.peek() != ESC) {
        if (console.user.available()) {
            char in = console.user.read();
            file.write(in);
            console.user.write(in);
        }
    }
    file.close();

    return SHUCK_CMD_SUCCESS;
});

