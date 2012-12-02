int start_daemon(
        unsigned int minimal_cpu_speed, unsigned int maximal_cpu_speed, unsigned int start_cpu_speed, int cpu_speed_step,
        unsigned long hotkey, unsigned long incrementkey, unsigned long decrmentkey,
        int speed_display, int foreground,
        unsigned long delay);
        
int kill_running_daemon();

void nano_setup();
void cmd_daemon(int argc, char *argv[]);

unsigned long parse_key_sequence(char *key_sequence);

void daemonmenu();

void formatkey(char * base, unsigned long keyseq);

void start_daemon_by_settings();
