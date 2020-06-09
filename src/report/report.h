#include <assert.h>
#include <fcntl.h>
#include <limits.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h>

#define MAXDIM 128

#define LOG_TAG "analyzer"

#define unitnos_fifo "/tmp/unitnos_fifo"
#define ack_end "ACK_END"
#define MAX_SIZE_MESSAGE 100

void unitnos_report_full_print(int *unitnos_report_records, int total_char);
void unitnos_report_partial_print(int *unitnos_report_records, int total_char);
void unitnos_report_menu();
void unitnos_report_record_fill(int* unitnos_report_records);
void unitnos_report_record_init(int *unitnos_report_records);
int unitnos_report_total_count(int *unitnos_report_records);
