#ifndef UNITNOS_REPORT_H_
#define UNITNOS_REPORT_H_

#ifdef __cplusplus
extern "C" {
#endif

#define MAXDIM 128

#define unitnos_fifo "/tmp/unitnos_fifo"
#define ack_end "ACK_END"
#define MAX_SIZE_MESSAGE 100

#define UNITNOS_REPORT_COMMAND_GLOBAL_PRINT "global_print"
#define UNITNOS_REPORT_COMMAND_PATH_PRINT "path_print"

void unitnos_report_full_print(int *unitnos_report_records, int total_char);
void unitnos_report_partial_print(int *unitnos_report_records, int total_char);
void unitnos_report_menu();
void unitnos_report_record_fill(int* unitnos_report_records);
void unitnos_report_record_init(int *unitnos_report_records);
int unitnos_report_total_count(int *unitnos_report_records);

#ifdef __cplusplus
}
#endif

#endif /* ifndef UNITNOS_REPORT_H_ */