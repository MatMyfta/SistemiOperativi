#include "report.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXDIM 128

typedef struct {
	char *character;
	int count;
} unitnos_report_record;

unitnos_report_record rec[MAXDIM];

char* get_character (int i) {
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

void unitnos_report_record_init() {
	int i;
	for (i = 0; i < MAXDIM; i++) {
		rec[i].character = get_character(i);
		rec[i].count = 0;
	}
}

void unitnos_report_record_print(unitnos_report_record r, int total_char) {
		float percentage = 0;
		if (total_char != 0)
			percentage = (float) ((float) (r.count*100.0)/total_char);
		printf("|%9s |%10d|%9.3lf%|\n", r.character, r.count, percentage);
		printf("+----------+----------+----------+\n");
}

void unitnos_report_print(int total_char) {
	// print header
	printf("+----------+----------+----------+\n");
	printf("|%10s|%10s|%10s|\n", "char", "frequency", "percentage");
	printf("+----------+----------+----------+\n");
	// print body
	int i;
	for (i = 0; i < MAXDIM; i++) {
		unitnos_report_record_print(rec[i],total_char);
	}
}

int unitnos_report_total_count() {
	int counter = 0;
	int i;
	for (i = 0; i < MAXDIM; i++)
		counter += rec[i].count;
	return counter;
}

int main() {
	printf("REPORT process has been started\nReported statistics:\n\n");

	unitnos_report_record_init();

	rec['a'].count = 12;
	rec['b'].count = 2;
	rec[' '].count = 100;

	int total_char = unitnos_report_total_count();

	unitnos_report_print(total_char);

	return;
}