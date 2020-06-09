#include "report.h"

#include "../logger.h"
#include "../protocol.h"
#include "../utils.h"

struct unitnos_report {
  unitnos_process *process;
  FILE *fin;
};

static char* get_character (int i) {
	char *ret = malloc(8);

	switch(i) {
		case 0: return "NUL";
		case 1: return "SOH";
		case 2: return "STX";
		case 3: return "ETX";
		case 4: return "EOT";
		case 5: return "ENQ";
		case 6: return "ACK";
		case 7: return "BEL";
		case 8: return "BS";
		case 9: return "TAB";
		case 10: return "LF";
		case 11: return "VT";
		case 12: return "FF";
		case 13: return "CR";
		case 14: return "SO";
		case 15: return "SI";
		case 16: return "DLE";
		case 17: return "DC1";
		case 18: return "DC2";
		case 19: return "DC3";
		case 20: return "DC4";
		case 21: return "NAK";
		case 22: return "SYN";
		case 23: return "ETB";
		case 24: return "CAN";
		case 25: return "EM";
		case 26: return "SUB";
		case 27: return "ESC";
		case 28: return "FS";
		case 29: return "GS";
		case 30: return "RS";
		case 31: return "US";
		case 127: return "DEL";
		default: ret[0] = (char) i; break;
	}
	return ret;
}

int unitnos_report_total_count(int *unitnos_report_records) {
	int counter = 0;
	int i;
	for (i = 0; i < MAXDIM; i++)
		counter += unitnos_report_records[i];
	return counter;
}

void unitnos_report_record_init(int *unitnos_report_records) {
	int i;
	for (i = 0; i < MAXDIM; i++) {
		unitnos_report_records[i] = 0;
	}
}

static void unitnos_report_record_print(int *unitnos_report_records, int position, int total_char) {
	float percentage = 0;
	if (total_char != 0)
		percentage = (float) ((float) (unitnos_report_records[position]*100.0)/total_char);
	printf("|%9s |%10d|%9.3lf%|\n", get_character(position), unitnos_report_records[position], percentage);
	printf("+----------+----------+----------+\n");
}

void unitnos_report_record_fill(int* unitnos_report_records) {
	unitnos_report_records['a'] = 12;
	unitnos_report_records['b'] = 2;
	unitnos_report_records[' '] = 100;
}


void unitnos_report_menu() {
	printf("*** SIMPLE MENU **************\n");
	printf("* 0: print_menu              *\n");
	printf("* 1: full_print              *\n");
	printf("* 2: partial_print           *\n");
	printf("* 3: send_request            *\n");
	printf("* 4: terminate               *\n");
	printf("******************************\n\n");
}

void unitnos_report_full_print(int *unitnos_report_records, int total_char) {
	// print header
	printf("+----------+----------+----------+\n");
	printf("|%10s|%10s|%10s|\n", "char", "frequency", "percentage");
	printf("+----------+----------+----------+\n");
	// print body
	int i;
	for (i = 0; i < MAXDIM; i++) {
		unitnos_report_record_print(unitnos_report_records,i,total_char);
	}
}

void unitnos_report_partial_print(int *unitnos_report_records, int total_char) {
	// print header
	printf("+----------+----------+----------+\n");
	printf("|%10s|%10s|%10s|\n", "char", "frequency", "percentage");
	printf("+----------+----------+----------+\n");
	// print body
	int i;
	for (i = 0; i < MAXDIM; i++) {
		if (unitnos_report_records[i] != 0)
			unitnos_report_record_print(unitnos_report_records, i,total_char);
	}
}

