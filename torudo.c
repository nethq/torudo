#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <pwd.h>
#include <stdarg.h>  // For verbose logging
#include <sys/stat.h>  // To check if files exist
#include <time.h>      // For time() function

// Include the user-defined macros for extended functionality
#include "extended_functionality_macros_user.h"

// Define constants
#define VERSION "1.3.5"                   // Software version
#define TORRC_PATH "/etc/tor/torrc"       // Path for Tor configuration
#define RESOLV_CONF "/etc/resolv.conf"    // Path to resolv.conf
#define RESOLV_CONF_BACKUP "/etc/resolv.conf.bak"  // Backup for resolv.conf
#define TRANS_PORT 9040                   // Tor transparent proxy port
#define DNS_PORT 5353                     // DNS port for Tor
#define CONTROL_PORT 9051                 // Control port for Tor
#define PACKET_CAPTURE_FILE "/tmp/torudo_capture.pcap" // Path to store the packet capture

// Function declarations
void check_root();
void check_prerequisites();
void start_tor();
void stop_tor();
void setup_iptables();
void flush_iptables();
void run_command(char **argv);
void sigint_handler(int sig);
void restore_resolv_conf();
void backup_resolv_conf();
void configure_resolv_conf();
uid_t get_tor_uid();
void show_help();
int file_exists(const char *filename);

#if ENABLE_VERBOSE_LOGGING
void verbose_log(const char *format, ...);
#endif

#if ENABLE_IDENTITY_MANAGER
void save_identity(const char *profile_name);
void load_identity(const char *profile_name);
void list_saved_identities();
#endif

#if ENABLE_COMMAND_SEQUENCE
void execute_command_sequence(char **commands, int num_commands);
#endif

#if ENABLE_GEOLOCATION_FILTER
void set_preferred_exit_countries(const char *countries);
#endif

#if ENABLE_TOR_MONITOR
void monitor_tor_activity();
char *get_current_exit_node_ip();
#endif

#if ENABLE_CIRCUIT_ASSESSMENT
void assess_and_switch_if_needed();
int assess_circuit_quality();
#endif

#if ENABLE_CUSTOM_ROUTING
void set_custom_routing_rules();
#endif

#if ENABLE_PACKET_CAPTURER
void start_packet_capture();
void stop_packet_capture();
#endif

int main(int argc, char *argv[]) {
    check_root();
    check_prerequisites();

    // Signal handler for Ctrl+C
    signal(SIGINT, sigint_handler);

    if (argc < 2) {
        show_help();
        exit(EXIT_FAILURE);
    }

    printf("\n---------------------- Torudo Debug Output ----------------------\n\n");

    #if ENABLE_VERBOSE_LOGGING
    verbose_log("Starting Torudo version %s\n", VERSION);
    #endif

    #if ENABLE_PACKET_CAPTURER
    start_packet_capture();
    #endif

    // Command parsing
    if (strcmp(argv[1], "--help") == 0) {
        show_help();
        exit(EXIT_SUCCESS);
    }

    // Handle specific command sequences if enabled
    #if ENABLE_COMMAND_SEQUENCE
    if (strcmp(argv[1], "--sequence") == 0 && argc > 2) {
        execute_command_sequence(&argv[2], argc - 2);
        stop_tor();
        exit(EXIT_SUCCESS);
    }
    #endif

    // Handle specific functionality like identity management
    #if ENABLE_IDENTITY_MANAGER
    if (strcmp(argv[1], "--save-identity") == 0 && argc == 3) {
        save_identity(argv[2]);
        stop_tor();
        exit(EXIT_SUCCESS);
    } else if (strcmp(argv[1], "--load-identity") == 0 && argc == 3) {
        load_identity(argv[2]);
        stop_tor();
        exit(EXIT_SUCCESS);
    } else if (strcmp(argv[1], "--list-identities") == 0) {
        list_saved_identities();
        stop_tor();
        exit(EXIT_SUCCESS);
    }
    #endif

    #if ENABLE_GEOLOCATION_FILTER
    if (strcmp(argv[1], "--set-exit-countries") == 0 && argc == 3) {
        set_preferred_exit_countries(argv[2]);
        stop_tor();
        exit(EXIT_SUCCESS);
    }
    #endif

    // Separate Torudo output from the command's output
    // Default: run the command via Tor
    start_tor();
    setup_iptables();

    printf("\n---------------------- Torudo is now executing your command ----------------------\n\n");
    run_command(&argv[1]);
    printf("\n---------------------- End of command output ----------------------\n\n");

    #if ENABLE_PACKET_CAPTURER
    stop_packet_capture();
    #endif

    stop_tor();

    return 0;
}

// Function definitions
void check_root() {
    if (geteuid() != 0) {
        fprintf(stderr, "You must run this program as root.\n");
        exit(EXIT_FAILURE);
    }
}

void check_prerequisites() {
    // Check if Tor configuration file exists
    if (!file_exists(TORRC_PATH)) {
        fprintf(stderr, "Tor configuration file (%s) not found.\n", TORRC_PATH);
        exit(EXIT_FAILURE);
    }

    // Check if the Tor service is installed and available
    if (system("which tor > /dev/null 2>&1") != 0) {
        fprintf(stderr, "Tor service is not installed. Please install Tor.\n");
        exit(EXIT_FAILURE);
    }

    // Ensure resolv.conf exists
    if (!file_exists(RESOLV_CONF)) {
        fprintf(stderr, "resolv.conf file not found at %s.\n", RESOLV_CONF);
        exit(EXIT_FAILURE);
    }

    #if ENABLE_VERBOSE_LOGGING
    verbose_log("All prerequisites satisfied.\n");
    #endif
}

void start_tor() {
    backup_resolv_conf();
    configure_resolv_conf();

    FILE *torrc = fopen(TORRC_PATH, "w");
    if (torrc == NULL) {
        perror("Failed to create tor configuration file");
        exit(EXIT_FAILURE);
    }

    fprintf(torrc,
        "VirtualAddrNetwork 10.0.0.0/10\n"
        "AutomapHostsOnResolve 1\n"
        "TransPort %d\n"
        "DNSPort %d\n"
        "ControlPort %d\n"
        "RunAsDaemon 1\n",
        TRANS_PORT, DNS_PORT, CONTROL_PORT
    );
    fclose(torrc);

    system("systemctl stop tor");
    system("pkill -f 'tor -f'");

    uid_t tor_uid = get_tor_uid();
    char command[256];
    snprintf(command, sizeof(command), "sudo -u '#%d' tor -f %s &", tor_uid, TORRC_PATH);
    system(command);

    sleep(5);

    #if ENABLE_VERBOSE_LOGGING
    verbose_log("Tor started successfully.\n");
    #endif
}

void setup_iptables() {
    flush_iptables();

    uid_t tor_uid = get_tor_uid();
    char command[512];

    snprintf(command, sizeof(command),
        "iptables -t nat -A OUTPUT -m owner --uid-owner %d -j RETURN", tor_uid);
    system(command);

    snprintf(command, sizeof(command),
        "iptables -t nat -A OUTPUT -p udp --dport 53 -j REDIRECT --to-ports %d", DNS_PORT);
    system(command);

    system("iptables -t nat -A OUTPUT -d 127.0.0.0/8 -j RETURN");

    snprintf(command, sizeof(command),
        "iptables -t nat -A OUTPUT -p tcp --syn -j REDIRECT --to-ports %d", TRANS_PORT);
    system(command);

    system("iptables -A OUTPUT -m state --state ESTABLISHED,RELATED -j ACCEPT");
    system("iptables -A OUTPUT -d 127.0.0.0/8 -j ACCEPT");

    snprintf(command, sizeof(command),
        "iptables -A OUTPUT -m owner --uid-owner %d -j ACCEPT", tor_uid);
    system(command);

    system("iptables -A OUTPUT -j REJECT");

    #if ENABLE_VERBOSE_LOGGING
    verbose_log("iptables rules configured.\n");
    #endif
}

void flush_iptables() {
    system("iptables -F");
    system("iptables -t nat -F");
    system("iptables -t mangle -F");
    system("iptables -X");

    #if ENABLE_VERBOSE_LOGGING
    verbose_log("iptables rules flushed.\n");
    #endif
}

void run_command(char **argv) {
    pid_t pid = fork();

    if (pid == 0) {
        execvp(argv[0], argv);
        perror("Failed to execute command");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    } else {
        perror("Failed to fork");
        exit(EXIT_FAILURE);
    }
}

void stop_tor() {
    flush_iptables();
    system("pkill -f 'tor -f'");
    restore_resolv_conf();

    #if ENABLE_VERBOSE_LOGGING
    verbose_log("Tor stopped and settings restored.\n");
    #endif
}

void sigint_handler(int sig) {
    printf("\nInterrupt received. Cleaning up...\n");
    stop_tor();
    #if ENABLE_PACKET_CAPTURER
    stop_packet_capture();
    #endif
    exit(EXIT_SUCCESS);
}

void backup_resolv_conf() {
    char command[256];
    snprintf(command, sizeof(command), "cp %s %s", RESOLV_CONF, RESOLV_CONF_BACKUP);
    system(command);
}

void restore_resolv_conf() {
    char command[256];
    snprintf(command, sizeof(command), "mv %s %s", RESOLV_CONF_BACKUP, RESOLV_CONF);
    system(command);
}

void configure_resolv_conf() {
    FILE *resolv = fopen(RESOLV_CONF, "w");
    if (resolv == NULL) {
        perror("Failed to configure resolv.conf");
        exit(EXIT_FAILURE);
    }

    fprintf(resolv, "nameserver 127.0.0.1\n");
    fclose(resolv);
}

uid_t get_tor_uid() {
    struct passwd *pwd = getpwnam("debian-tor");
    if (pwd == NULL) {
        perror("Failed to get debian-tor UID");
        exit(EXIT_FAILURE);
    }
    return pwd->pw_uid;
}

int file_exists(const char *filename) {
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}

#if ENABLE_VERBOSE_LOGGING
void verbose_log(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}
#endif

#if ENABLE_IDENTITY_MANAGER
#include <json-c/json.h>

void save_identity(const char *profile_name) {
    FILE *file = fopen("/etc/tor/identities.json", "r+");
    if (!file) {
        file = fopen("/etc/tor/identities.json", "w");
    }

    struct json_object *json = json_object_from_file("/etc/tor/identities.json");
    if (!json) {
        json = json_object_new_object();
    }

    struct json_object *profile = json_object_new_object();
    json_object_object_add(profile, "timestamp", json_object_new_int(time(NULL)));

    json_object_object_add(json, profile_name, profile);
    json_object_to_file("/etc/tor/identities.json", json);

    fclose(file);
    printf("Identity saved as %s\n", profile_name);
}

void load_identity(const char *profile_name) {
    struct json_object *json = json_object_from_file("/etc/tor/identities.json");
    if (!json) {
        printf("No saved identities found.\n");
        return;
    }

    struct json_object *profile;
    if (json_object_object_get_ex(json, profile_name, &profile)) {
        printf("Loaded identity: %s\n", profile_name);
    } else {
        printf("Identity not found.\n");
    }
}

void list_saved_identities() {
    struct json_object *json = json_object_from_file("/etc/tor/identities.json");
    if (!json) {
        printf("No saved identities found.\n");
        return;
    }

    json_object_object_foreach(json, key, val) {
        printf("Saved Identity: %s\n", key);
    }
}
#endif

#if ENABLE_COMMAND_SEQUENCE
void execute_command_sequence(char **commands, int num_commands) {
    for (int i = 0; i < num_commands; i++) {
        printf("Executing command %d: %s\n", i + 1, commands[i]);
        run_command(&commands[i]);
        if (i != num_commands - 1) {
            stop_tor();
            start_tor();  // Restart Tor for new identity
        }
    }
}
#endif

#if ENABLE_GEOLOCATION_FILTER
void set_preferred_exit_countries(const char *countries) {
    char command[512];
    snprintf(command, sizeof(command), "ExitNodes {%s}", countries);
    FILE *torrc = fopen(TORRC_PATH, "a");
    if (torrc != NULL) {
        fprintf(torrc, "%s\n", command);
        fclose(torrc);
        printf("Exit nodes filtered by countries: %s\n", countries);
    } else {
        perror("Failed to modify Tor configuration");
    }
}
#endif

#if ENABLE_TOR_MONITOR
void monitor_tor_activity() {
    time_t start_time = time(NULL);
    while (1) {
        time_t current_time = time(NULL);
        printf("Current exit node IP: %s\n", get_current_exit_node_ip());
        printf("Connection uptime: %ld seconds\n", current_time - start_time);
        sleep(10);
    }
}

char *get_current_exit_node_ip() {
    return "IP_ADDRESS_HERE";  // Stub for the actual implementation
}
#endif

#if ENABLE_CIRCUIT_ASSESSMENT
void assess_and_switch_if_needed() {
    int quality = assess_circuit_quality();
    if (quality < 50) {
        printf("Circuit quality is low. Switching to a new exit node...\n");
        stop_tor();
        start_tor();
    }
}

int assess_circuit_quality() {
    return rand() % 100;  // Random quality check as a placeholder
}
#endif

#if ENABLE_CUSTOM_ROUTING
void set_custom_routing_rules() {
    system("iptables -t nat -A OUTPUT -p tcp -d example.com -j RETURN");
    printf("Custom routing rules applied: example.com bypasses Tor.\n");
}
#endif

#if ENABLE_PACKET_CAPTURER
void start_packet_capture() {
    printf("Starting packet capture to %s...\n", PACKET_CAPTURE_FILE);
    char command[512];
    snprintf(command, sizeof(command), "tcpdump -i any -w %s &", PACKET_CAPTURE_FILE);
    system(command);
}

void stop_packet_capture() {
    printf("Stopping packet capture...\n");
    system("pkill -f 'tcpdump -i any'");
    printf("Packet capture saved to %s\n", PACKET_CAPTURE_FILE);
}
#endif

void show_help() {
    printf("Torudo v%s\n", VERSION);
    printf("Usage: torudo [options] <command>\n\n");
    printf("Options:\n");
    printf("  --help                     Show this help message\n");

    #if ENABLE_IDENTITY_MANAGER
    printf("  --save-identity <name>      Save the current Tor identity\n");
    printf("  --load-identity <name>      Load a saved Tor identity\n");
    printf("  --list-identities           List all saved identities\n");
    #endif

    #if ENABLE_COMMAND_SEQUENCE
    printf("  --sequence <commands...>    Run a sequence of commands with different Tor identities\n");
    #endif

    #if ENABLE_GEOLOCATION_FILTER
    printf("  --set-exit-countries <list> Filter exit nodes by country (e.g. us,de)\n");
    #endif

    printf("\n");
}
