#include "report.h"
#include "../bool.h"

#define LOG_TAG "report"
#include "../logger.h"

#include "../process.h"
#include "../protocol.h"

#include <assert.h>
#include <fcntl.h>
#include <limits.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h>

/*******************************************************************************
 * Private functions declarations
 *******************************************************************************/
static int independent_report_main(int argc, char **argv);
static int child_report_main(int in_pipe, int output_pipe);

/*
 * Call command_ask_global for each type
 * Call command_ask_path for each type
 */
static void command_ask_all();
static void command_ask_all_path();
static void command_ask_all_global();

/*
 * Not used but could be useful
 * argv[0]: not used
 * argv[1]: global or path
 * argv[2]: type of statistic
 */
static int send_request_command(int argc, const char *argv[]);

/*
 * Ask for global statistic of one type
 */
static void command_ask_global(const char *type);

/*
 * Ask for each path statistic of one type
 */
static void command_ask_path(const char *type);
static int send_ack(char* value);
static bool is_ack_end(const char* str);
static int send_message (char *message_to_send);
static char* read_message();
static char* concat(const char *s1, const char *s2);

int main(int argc, char *argv[]) {
  if (unitnos_process_is_process(argc, argv)) {
    return child_report_main(atoi(argv[1]), atoi(argv[2]));
  } else {
    return independent_report_main(argc, argv);
  }
}

static int independent_report_main(int argc, char** argv) {
    log_debug("Report running in standalone mode");
    //mkfifo(unitnos_fifo, 0666);

    if (argc != 2) {
        log_error("Usage: report <global || path || both>");
        return -1;
    }

    if (strcmp(argv[1], "both")==0) {
        //command_ask_all();
    } else if (strcmp(argv[1], "path")==0) {
        //command_ask_all_path();
    } else if (strcmp(argv[1], "global")==0) {
        //command_ask_all_global();
    } else {
        log_error("Unrecognized command");
    }

    
    return 0; 
}

static int child_report_main(int in_pipe, int output_pipe) {
    log_debug("Report running in child mode");

    FILE *fin = fdopen(in_pipe, "r");

    char *message = NULL;
    size_t message_size = 0;

    while (1) {
        if (getline(&message, &message_size, fin) >= 0) {
            struct unitnos_protocol_command command = unitnos_protocol_parse(message);
            log_verbose("Received command: %s", command.command);

            if (!strcmp(command.command, "both_print")) {
                //command_ask_all_global();
            }

            if (!strcmp(command.command, "path_print")) {
                //command_ask_all_path();
            }

        } else if (feof(fin)) {
            log_debug("Input pipe closed. Terminate");
            break;
        }
    }
  return 0;
}


static int send_request_command(int argc, const char *argv[]) {
    if (argc != 3) {
        printf("%s\n", "Error, usage: send_request <scope> <type>");
    } else if ((strcmp(argv[1], "global")!=0) && (strcmp(argv[1], "path")!=0)) {
        printf("%s | You inserted: [%s]\n", "scope: [global] - [path]", argv[1]);
    } else if ((atoi(argv[2])<1) || (atoi(argv[2])>4)) {
        printf("%s | You inserted: [%s]\n", "type: [1-4]", argv[2]);
    } else {
        if (strcmp(argv[1], "global")==0)
            command_ask_global(argv[2]);
        else
            command_ask_path(argv[2]);
    }
    return 0;
}

static void command_ask_all() {
    command_ask_all_path();
    command_ask_all_global();
}

static void command_ask_all_path() {
    command_ask_path("1");
    command_ask_path("2");
    command_ask_path("3");
    command_ask_path("4");
}

static void command_ask_all_global() {
    command_ask_global("1");
    command_ask_global("2");
    command_ask_global("3");
    command_ask_global("4");
}

static void command_ask_global(const char *type) {
    int fd, readed, size=80;

    /* messaggio viene codificato come comando per analyzer*/
    char *tmp="global:";
    char *tmp1 = concat(tmp, type);
    char * message_to_send = concat(tmp1, "\n");
    message_to_send[strlen(message_to_send)]='\0';

    send_message(message_to_send);
    char *response = read_message(size);
    if (response!=NULL) // ELABORA RISPOSTA
        printf("%s\n", response);
}

static void command_ask_path(const char *type) {
    /* messaggio viene codificato come comando per analyzer*/
    char *tmp="path:";
    char *tmp1 = concat(tmp, type);
    char * message_to_send = concat(tmp1, "\n");
    message_to_send[strlen(message_to_send)]='\0';

    send_message(message_to_send);

    // Mi metto in ascolto finche non ho elaborato ciascun path
    bool received_last_path=false;
    int i=0;
    while (!received_last_path) {
        char* received_message_path = read_message(MAX_SIZE_MESSAGE);
        char* received_message_stat;
        if (received_message_path!=NULL) {
            if (!is_ack_end(received_message_path)) {
                send_ack("1");
                received_message_stat = read_message(MAX_SIZE_MESSAGE);
                if (received_message_stat!=NULL) {
                    if (!is_ack_end(received_message_stat)) { // ELABORA STATISTICHE
                        printf("path #%d received\n", i+1);
                        i++;
                        printf("%s - %s\n", received_message_path, received_message_stat);
                        send_ack("1");
                    } else {
                        received_last_path = true;
                    }
                } else {
                    send_ack("0");
                }
            } else {
                received_last_path = true;
            }
        } else {
            send_ack("0");
        }
    }
}

static int send_message(char *message_to_send) {
    int fd, ret_value;
    fd = open(unitnos_fifo, O_WRONLY);
    if (fd==-1) {
        log_error("Failed opening named pipe");
        ret_value = 1;
    } else {
        if (write(fd, message_to_send, strlen(message_to_send)+1)==-1) {
            log_error("Failed writing on named pipe");
            ret_value = 1;
        } else {
            ret_value = 0;
        }
        close(fd);
    }
    return ret_value;
}

static char* read_message(int size) {
    char* ret_value;
    int fd;
    ssize_t readed;
    char *message_to_read = malloc(size +1);
    fd = open(unitnos_fifo, O_RDONLY);
    if (fd==-1) {
        log_error("Failed opening named pipe");
        free(message_to_read);
        ret_value=NULL;
    } else {
        readed=read(fd, message_to_read, size +1);
        if (readed==-1) {
            log_error("Failed writing on named pipe");
            free(message_to_read);
            ret_value=NULL;
        } else {
            message_to_read[readed]='\0';
            ret_value = message_to_read;
        }
        close(fd);
    }
    return ret_value;  
}

/*
 * value = 1 -> go on, value = 0 -> stop
 */
static int send_ack(char* value) {
    int ret_value;
    char *tmp="ack:";
    char *tmp1=concat(tmp,value);
    char *message_to_send = concat(tmp1, "\n");
    message_to_send[(strlen(message_to_send))]='\0';
    ret_value = send_message(message_to_send);
    return ret_value;
}

static bool is_ack_end(const char *str) {
    bool ret_value = false;
    if (strcmp(str, ack_end) == 0)
        ret_value = true;
    return ret_value;
}

char* concat(const char *s1, const char *s2)
{
    char *result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
    strcpy(result, s1);
    strcat(result, s2); 
    return result;
}