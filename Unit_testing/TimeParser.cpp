#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "TimeParser.h"

// time format: HHMMSS (6 characters)
int time_parse(char *time) {

	int seconds = TIME_LEN_ERROR;
	// Check that string is not null
	if (time == NULL) { return TIME_LEN_ERROR; }

	// Lenght check
	if (strlen(time) != 6) { return TIME_LEN_ERROR; }

	// Type check
	for (int i = 0; i < 6; i++) {
		if(!isdigit((unsigned char)time[i])){
			return TIME_ARRAY_ERROR;
		}
	}

	// Parse values from time string
	// For example: 124033 -> 12hour 40min 33sec
    int values[3];
	values[2] = atoi(time+4); // seconds
	time[4] = 0;
	values[1] = atoi(time+2); // minutes
	time[2] = 0;
	values[0] = atoi(time); // hours
	// Now you have:
	// values[0] hour
	// values[1] minute
	// values[2] second

	// Add boundary check time values: below zero or above limit not allowed
	// limits are 59 for minutes, 23 for hours, etc
	if (values[0] < 0 || values[0] > 23 ||
		values[1] < 0 || values[1] > 59 ||
		values[2] < 0 || values[2] > 59) {
			return TIME_VALUE_ERROR;
		}

	// Calculate return value from the parsed minutes and seconds
	// Otherwise error will be returned!
	seconds = (values[0] * 3600) + (values[1] * 60) + values[2];

	if (seconds == 0) { return TIME_ZERO_ERROR; }
	return seconds;

}
