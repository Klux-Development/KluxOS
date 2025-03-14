#include <shell/shell.h>
#include <drivers/terminal.h>
#include <drivers/keyboard.h>
#include <kernel/fs.h>
#include <kernel/types.h>
#include <kernel/process.h>

static shell_context_t shell_ctx;

typedef unsigned int size_t;

static void str_copy(char* dest, const char* src, size_t max_len);
static int str_compare(const char* s1, const char* s2);
static int str_length(const char* s);
static void str_clear(char* s, size_t len);

static const shell_command_t commands[] = {
    {
        .name = "help",
        .description = "Show help information about commands",
        .handler = cmd_help,
        .usage = "help [command]"
    },
    {
        .name = "clear",
        .description = "Clear the screen",
        .handler = cmd_clear,
        .usage = "clear"
    },
    {
        .name = "echo",
        .description = "Print text to the screen",
        .handler = cmd_echo,
        .usage = "echo [text...]"
    },
    {
        .name = "exit",
        .description = "Exit the shell",
        .handler = cmd_exit,
        .usage = "exit"
    },
    {
        .name = "version",
        .description = "Show system version information",
        .handler = cmd_version,
        .usage = "version"
    },
    {
        .name = "ls",
        .description = "List directory contents",
        .handler = cmd_ls,
        .usage = "ls [directory]"
    },
    {
        .name = "cd",
        .description = "Change directory",
        .handler = cmd_cd,
        .usage = "cd [directory]"
    },
    {
        .name = "cat",
        .description = "Show file contents",
        .handler = cmd_cat,
        .usage = "cat file"
    },
    {
        .name = "ps",
        .description = "List running processes",
        .handler = cmd_ps,
        .usage = "ps"
    },
    {
        .name = "uptime",
        .description = "Show system uptime",
        .handler = cmd_uptime,
        .usage = "uptime"
    },
    {
        .name = "meminfo",
        .description = "Show memory information",
        .handler = cmd_meminfo,
        .usage = "meminfo"
    },
    {
        .name = NULL,
        .description = NULL,
        .handler = NULL,
        .usage = NULL
    }
};

void shell_init(void) {
    str_copy(shell_ctx.current_dir, "/", sizeof(shell_ctx.current_dir));
    
    shell_ctx.flags = SHELL_FLAG_ECHO | SHELL_FLAG_COLOR;
    
    str_copy(shell_ctx.username, "root", sizeof(shell_ctx.username));
    str_copy(shell_ctx.hostname, "kluxos", sizeof(shell_ctx.hostname));
    
    str_clear(shell_ctx.last_command, sizeof(shell_ctx.last_command));
    
    shell_ctx.exit_code = 0;
    
    keyboard_connect_terminal();
}

void shell_run(void) {
    char command_line[SHELL_MAX_COMMAND_LENGTH];
    shell_status_t status;
    
    shell_print_banner();
    
    while (1) {
        shell_print_prompt();
        
        terminal_read_line(command_line, SHELL_MAX_COMMAND_LENGTH);
        
        if (command_line[0] == '\0') {
            continue;
        }
        
        str_copy(shell_ctx.last_command, command_line, SHELL_MAX_COMMAND_LENGTH);
        
        status = shell_execute_command(command_line);
        
        if (status == SHELL_EXIT) {
            break;
        }
        
        shell_ctx.exit_code = (int)status;
    }
}

int shell_parse_args(char* command_line, char** argv, int max_args) {
    int argc = 0;
    int i = 0;
    bool in_quote = false;
    bool in_arg = false;
    char quote_char = 0;
    
    while (command_line[i] != '\0' && argc < max_args) {
        char c = command_line[i];
        
        if ((c == '"' || c == '\'') && !in_quote) {
            in_quote = true;
            quote_char = c;
            
            if (!in_arg) {
                argv[argc++] = &command_line[i+1];
                in_arg = true;
            }
        }
        else if (c == quote_char && in_quote) {
            in_quote = false;
            quote_char = 0;
            command_line[i] = '\0';
        }
        else if ((c == ' ' || c == '\t') && !in_quote) {
            if (in_arg) {
                command_line[i] = '\0';
                in_arg = false;
            }
        }
        else {
            if (!in_arg) {
                argv[argc++] = &command_line[i];
                in_arg = true;
            }
        }
        
        i++;
    }
    
    argv[argc] = NULL;
    
    return argc;
}

shell_status_t shell_execute_command(const char* command_line) {
    char cmd_copy[SHELL_MAX_COMMAND_LENGTH];
    char* argv[64];
    int argc;
    
    str_copy(cmd_copy, command_line, SHELL_MAX_COMMAND_LENGTH);
    
    argc = shell_parse_args(cmd_copy, argv, 64);
    
    if (argc > 0) {
        for (int i = 0; commands[i].name != NULL; i++) {
            if (str_compare(argv[0], commands[i].name) == 0) {
                return commands[i].handler(argc, argv);
            }
        }
        
        terminal_writestring("Command not found: ");
        terminal_writestring(argv[0]);
        terminal_writestring("\n");
        return SHELL_ERROR_COMMAND_NOT_FOUND;
    }
    
    return SHELL_OK;
}

void shell_print_prompt(void) {
    terminal_set_fg_color(VGA_COLOR_GREEN);
    terminal_writestring(shell_ctx.username);
    
    terminal_set_fg_color(VGA_COLOR_WHITE);
    terminal_writestring("@");
    
    terminal_set_fg_color(VGA_COLOR_LIGHT_BLUE);
    terminal_writestring(shell_ctx.hostname);
    
    terminal_set_fg_color(VGA_COLOR_WHITE);
    terminal_writestring(":");
    
    terminal_set_fg_color(VGA_COLOR_LIGHT_BROWN);
    terminal_writestring(shell_ctx.current_dir);
    
    terminal_set_fg_color(VGA_COLOR_WHITE);
    terminal_writestring("$ ");
    
    terminal_reset_color();
}

void shell_clear_screen(void) {
    terminal_clear();
}

void shell_print_banner(void) {
    terminal_clear();
    terminal_set_fg_color(VGA_COLOR_LIGHT_RED);
    terminal_writestring("\n");
    terminal_writestring("  _  __ _               ___  ____  \n");
    terminal_writestring(" | |/ /| | _   _ __  __/ _ \\/ ___| \n");
    terminal_writestring(" | ' / | || | | |\\ \\/ / | | \\___ \\ \n");
    terminal_writestring(" | . \\ | || |_| | >  <| |_| |___) |\n");
    terminal_writestring(" |_|\\_\\|_| \\__,_|/_/\\_\\\\___/|____/ \n");
    terminal_writestring("\n");
    
    terminal_set_fg_color(VGA_COLOR_LIGHT_GREY);
    terminal_writestring("KluxOS Terminal - Version ");
    terminal_writestring(SHELL_VERSION);
    terminal_writestring("\n");
    terminal_writestring("Type 'help' for assistance.\n");
    terminal_writestring("\n");
    terminal_reset_color();
}

void shell_help(void) {
    terminal_writestring("Available commands:\n\n");
    
    for (int i = 0; commands[i].name != NULL; i++) {
        terminal_set_fg_color(VGA_COLOR_LIGHT_GREEN);
        terminal_writestring("  ");
        terminal_writestring(commands[i].name);
        terminal_set_fg_color(VGA_COLOR_LIGHT_GREY);
        
        int name_len = str_length(commands[i].name);
        int spaces = 12 - name_len;
        for (int j = 0; j < spaces; j++) {
            terminal_writestring(" ");
        }
        
        terminal_writestring(commands[i].description);
        terminal_writestring("\n");
    }
    
    terminal_reset_color();
    terminal_writestring("\n");
}

//================ Komut İşleyicileri ================//

shell_status_t cmd_help(int argc, char** argv) {
    if (argc == 1) {
        shell_help();
    } else {
        for (int i = 0; commands[i].name != NULL; i++) {
            if (str_compare(argv[1], commands[i].name) == 0) {
                terminal_set_fg_color(VGA_COLOR_LIGHT_GREEN);
                terminal_writestring("Command: ");
                terminal_writestring(commands[i].name);
                terminal_writestring("\n");
                
                terminal_set_fg_color(VGA_COLOR_WHITE);
                terminal_writestring("Description: ");
                terminal_set_fg_color(VGA_COLOR_LIGHT_GREY);
                terminal_writestring(commands[i].description);
                terminal_writestring("\n");
                
                terminal_set_fg_color(VGA_COLOR_WHITE);
                terminal_writestring("Usage: ");
                terminal_set_fg_color(VGA_COLOR_LIGHT_GREY);
                terminal_writestring(commands[i].usage);
                terminal_writestring("\n");
                
                terminal_reset_color();
                return SHELL_OK;
            }
        }
        
        terminal_writestring("Command not found: ");
        terminal_writestring(argv[1]);
        terminal_writestring("\n");
        return SHELL_ERROR_COMMAND_NOT_FOUND;
    }
    
    return SHELL_OK;
}

shell_status_t cmd_clear(int argc, char** argv) {
    shell_clear_screen();
    return SHELL_OK;
}

shell_status_t cmd_echo(int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        terminal_writestring(argv[i]);
        if (i < argc - 1) {
            terminal_writestring(" ");
        }
    }
    terminal_writestring("\n");
    return SHELL_OK;
}

shell_status_t cmd_exit(int argc, char** argv) {
    terminal_writestring("Shell'den cikiliyor...\n");
    return SHELL_EXIT;
}

shell_status_t cmd_version(int argc, char** argv) {
    terminal_writestring("KluxOS Terminal - Version ");
    terminal_writestring(SHELL_VERSION);
    terminal_writestring("\n");
    return SHELL_OK;
}

shell_status_t cmd_ls(int argc, char** argv) {
    const char* path = (argc > 1) ? argv[1] : shell_ctx.current_dir;
    fs_node_t* node;
    
    if (str_compare(path, "/") == 0) {
        node = fs_root;
    } else {
        node = fs_finddir(fs_root, (char*)path);
    }
    
    if (!node) {
        terminal_writestring("Dizin bulunamadi: ");
        terminal_writestring(path);
        terminal_writestring("\n");
        return SHELL_ERROR_INVALID_ARGUMENTS;
    }
    
    if (node->type != FS_DIRECTORY) {
        terminal_writestring("Dizin degil: ");
        terminal_writestring(path);
        terminal_writestring("\n");
        return SHELL_ERROR_INVALID_ARGUMENTS;
    }
    
    fs_node_t* child = node->children;
    int count = 0;
    
    while (child) {
        if (child->type == FS_DIRECTORY) {
            terminal_set_fg_color(VGA_COLOR_LIGHT_BLUE);
        } else if (child->type == FS_FILE) {
            terminal_set_fg_color(VGA_COLOR_LIGHT_GREY);
        } else {
            terminal_set_fg_color(VGA_COLOR_LIGHT_RED);
        }
        
        terminal_writestring(child->name);
        
        if (child->type == FS_DIRECTORY) {
            terminal_writestring("/");
        }
        
        count++;
        if (count % 5 == 0) {
            terminal_writestring("\n");
        } else {
            terminal_writestring("\t");
        }
        
        child = child->next;
    }
    
    if (count % 5 != 0) {
        terminal_writestring("\n");
    }
    
    terminal_reset_color();
    terminal_writestring("\nToplam: ");
    terminal_print_int(count);
    terminal_writestring(" oge\n");
    
    return SHELL_OK;
}

shell_status_t cmd_cd(int argc, char** argv) {
    if (argc < 2) {
        str_copy(shell_ctx.current_dir, "/", sizeof(shell_ctx.current_dir));
        return SHELL_OK;
    }
    
    const char* path = argv[1];
    fs_node_t* node;
    
    if (str_compare(path, "/") == 0) {
        str_copy(shell_ctx.current_dir, "/", sizeof(shell_ctx.current_dir));
        return SHELL_OK;
    }
    
    if (str_compare(path, "..") == 0) {
        char* last_slash = NULL;
        int len = str_length(shell_ctx.current_dir);
        
        for (int i = len - 1; i >= 0; i--) {
            if (shell_ctx.current_dir[i] == '/') {
                last_slash = &shell_ctx.current_dir[i];
                break;
            }
        }
        
        if (last_slash) {
            if (last_slash == shell_ctx.current_dir) {
                str_copy(shell_ctx.current_dir, "/", sizeof(shell_ctx.current_dir));
            } else {
                *last_slash = '\0';
            }
        }
        
        return SHELL_OK;
    }
    
    node = fs_finddir(fs_root, (char*)path);
    
    if (!node) {
        terminal_writestring("Dizin bulunamadi: ");
        terminal_writestring(path);
        terminal_writestring("\n");
        return SHELL_ERROR_INVALID_ARGUMENTS;
    }
    
    if (node->type != FS_DIRECTORY) {
        terminal_writestring("Dizin degil: ");
        terminal_writestring(path);
        terminal_writestring("\n");
        return SHELL_ERROR_INVALID_ARGUMENTS;
    }
    
    str_copy(shell_ctx.current_dir, path, sizeof(shell_ctx.current_dir));
    
    return SHELL_OK;
}

shell_status_t cmd_cat(int argc, char** argv) {
    if (argc < 2) {
        terminal_writestring("Kullanim: cat <dosya>\n");
        return SHELL_ERROR_INVALID_ARGUMENTS;
    }
    
    const char* filename = argv[1];
    fs_node_t* node = fs_finddir(fs_root, (char*)filename);
    
    if (!node) {
        terminal_writestring("Dosya bulunamadi: ");
        terminal_writestring(filename);
        terminal_writestring("\n");
        return SHELL_ERROR_INVALID_ARGUMENTS;
    }
    
    if (node->type != FS_FILE) {
        terminal_writestring("Dosya degil: ");
        terminal_writestring(filename);
        terminal_writestring("\n");
        return SHELL_ERROR_INVALID_ARGUMENTS;
    }
    
    fs_open(node);
    
    uint8_t buffer[1024];
    uint32_t bytes_read = fs_read(node, 0, node->length > 1024 ? 1024 : node->length, buffer);
    
    for (uint32_t i = 0; i < bytes_read; i++) {
        terminal_putchar(buffer[i]);
    }
    
    if (bytes_read > 0 && buffer[bytes_read - 1] != '\n') {
        terminal_writestring("\n");
    }
    
    fs_close(node);
    
    return SHELL_OK;
}

shell_status_t cmd_ps(int argc, char** argv) {
    extern process_t* process_list;
    process_t* current = process_list;
    int count = 0;
    
    terminal_writestring("  PID  |  DURUM  |  AD\n");
    terminal_writestring("-------|---------|----------------\n");
    
    while (current != NULL) {
        char buffer[64];
        const char* state_str;
        
        switch(current->state) {
            case PROCESS_READY: state_str = "HAZIR"; break;
            case PROCESS_RUNNING: state_str = "CALISMA"; break;
            case PROCESS_BLOCKED: state_str = "BLOKE"; break;
            case PROCESS_TERMINATED: state_str = "SONLANDI"; break;
            default: state_str = "BILINMIYOR"; break;
        }
        
        terminal_printf("  %3d  | %8s | %s\n", 
                       current->pid, 
                       state_str, 
                       current->name);
        
        current = current->next;
        count++;
    }
    
    if (count == 0) {
        terminal_writestring("Aktif islem bulunamadi.\n");
    }
    
    return SHELL_OK;
}

// uptime komutu
shell_status_t cmd_uptime(int argc, char** argv) {
    extern uint32_t get_tick_count(void);
    uint32_t ticks = get_tick_count();
    
    uint32_t seconds = ticks / 100;
    uint32_t minutes = seconds / 60;
    uint32_t hours = minutes / 60;
    uint32_t days = hours / 24;
    
    minutes %= 60;
    hours %= 24;
    
    terminal_writestring("System uptime: ");
    terminal_printf("%u days, %u hours, %u minutes\n", days, hours, minutes);
    
    return SHELL_OK;
}

shell_status_t cmd_meminfo(int argc, char** argv) {
    extern void memory_info(void);
    
    memory_info();
    
    return SHELL_OK;
}

//================ Yardımcı Fonksiyonlar ================//

static void str_copy(char* dest, const char* src, size_t max_len) {
    size_t i;
    
    for (i = 0; i < max_len - 1 && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    
    dest[i] = '\0';
}

static int str_compare(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    
    return *s1 - *s2;
}

static int str_length(const char* s) {
    int len = 0;
    
    while (s[len] != '\0') {
        len++;
    }
    
    return len;
}

static void str_clear(char* s, size_t len) {
    for (size_t i = 0; i < len; i++) {
        s[i] = '\0';
    }
}
