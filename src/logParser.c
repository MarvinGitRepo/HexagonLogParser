/*
    Log File Parsing for Hexagon
    Written by Marvin Lee
*/


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "logParser.h"


/*************************** Key value functions ***************************/

// Function to print key-value pairs
void printKeyValuePairs(keyPair *pairs, int size) {
    for (int i = 0; i < size; i++) {
        printf("%s %d\n", pairs[i].key, pairs[i].value);
    }
    printf("\n");
}

// Finds index based on key
int findKey(keyPair *pairs, int size, const char *key) {
    for (int i = 0; i < size; i++) {
        if (strcmp(pairs[i].key, key) == 0) {
            return i; // return index on key match
        }
    }
    return -1;  // -1 means key not found
}

// Inserts or updates key value pairs and resizes array if necessary
void InsertUpdateKey(keyPair **pairs, int *size, int *capacity, const char *key) {
    int index = findKey(*pairs, *size, key);
    if (index != -1) {
        // Found key so increment value
        (*pairs)[index].value++;
    } else {
        // Double capacity if full
        if (*size == *capacity) {
            //printf("resizing\n");
            *capacity *= 2;
            *pairs = realloc(*pairs, *capacity * sizeof(keyPair));
            if (!*pairs) {
                printf("Failed to reallocate table size, exiting\n");
                exit(EXIT_FAILURE);
            }
        }
        // Insert key
        strcpy((*pairs)[*size].key, key);
        (*pairs)[*size].value = 1; // first insertion means value of 1
        (*size)++;
    }
}

//Comparison function for sorting to move the most frequent elements to the top
int compareByValue(const void *a, const void *b) {
    keyPair *pairA = (keyPair *)a;
    keyPair *pairB = (keyPair *)b;
    return pairB->value - pairA->value;
}
/************************** Log utility functions ***********************/

int parseLine(char line[], logEntry *entry) {
    char timeString[MAX_DATE_LENGTH];
    int day, hour, minute, second = 0;
    int year = 2024-1900; // years since 1900 for tm
    int month = 7; // missing from log entry but document mentions August
    char returnSizeString[MAX_STRING_LENGTH];

    if (sscanf(line, "%s [%[^]]] \"%s %s %[^\"]\" %d %s", entry->hostName, timeString, 
        entry->requestMethod, entry->requestURL, entry->requestProtocol,
        &entry->returnCode, returnSizeString) != NUM_ENTRY_TYPES) {
        return EXIT_FAILURE;
    }
    
    //post process capture
    //example log shows "-" for some return sizes which I will assume is 0
    if (!strncmp(returnSizeString,"-", MAX_STRING_LENGTH)) {
        entry->returnSize = 0;
    } else {
        entry->returnSize = atoi(returnSizeString);
    }

    //process time
    if (sscanf(timeString, "%d:%d:%d:%d", &day, &hour, &minute, &second) != NUM_TIME_ENTRIES) {
        printf("invalid time format: %s\n", timeString);
        return EXIT_FAILURE;
    }
    entry->time.tm_year = year;
    entry->time.tm_mon = month;
    entry->time.tm_mday = day;
    entry->time.tm_hour = hour;
    entry->time.tm_min = minute;
    entry->time.tm_sec = second;
    entry->time.tm_isdst = -1;
    entry->timezoneHour = -4; // storing EDT information
    entry->timezoneMin = 0;

    return EXIT_SUCCESS;
}


int logParse(const char logName[], result *info, time_t timeStart,
        time_t timeEnd, bool timeConvert) {
    FILE *logFilePtr;
    char logLine[MAX_LOG_LINE_LENGTH];
    logEntry entry = {0};
    int validEntries = 0;
    int invalidEntries = 0;
    int status;
    logFilePtr = fopen(logName, "r");
    time_t logTime;
    if (logFilePtr == NULL) {
        printf("error opening file: %s errno: %s \n", logName, strerror(errno));
        exit(EXIT_FAILURE);
    }

    while (fgets(logLine, sizeof(logLine), logFilePtr)) {
        //printf("line %d: %s", i, logLine);
        status = parseLine(logLine, &entry);
        if (status == EXIT_SUCCESS) {
            //printf("entry: %s %s %s %s %d %d\n", entry.hostName, 
            //    entry.requestMethod, entry.requestURL, entry.requestProtocol, entry.returnCode, entry.returnSize);
            //printf("entry time: %d %d %d %d\n", entry.time.tm_mday, entry.time.tm_hour, entry.time.tm_min, entry.time.tm_sec);
            logTime = mktime(&entry.time);
            if (logTime == -1) {
                printf("failed to convert time \n");
                continue;
            }
            if ((logTime >= timeStart) && (logTime <=timeEnd) && (timeConvert == true)) {
                InsertUpdateKey(&info->numHostAccess,
                    &info->numHostAccessSize,
                    &info->numHostAccessCapacity, entry.hostName);

                //retrieve URL for gets
                if (!strncmp("GET", entry.requestMethod, MAX_STRING_LENGTH) &&
                    (entry.returnCode == HTTP_CODE_OK)) {
                    InsertUpdateKey(&info->numGet,
                    &info->numGetSize,
                    &info->numGetCapacity, entry.requestURL);
                validEntries++;
                }
            } else if (timeConvert == false) {
                InsertUpdateKey(&info->numHostAccess,
                    &info->numHostAccessSize,
                    &info->numHostAccessCapacity, entry.hostName);

            //retrieve URL for gets
                if (!strncmp("GET", entry.requestMethod, MAX_STRING_LENGTH) &&
                    (entry.returnCode == HTTP_CODE_OK)) {
                    InsertUpdateKey(&info->numGet,
                    &info->numGetSize,
                    &info->numGetCapacity, entry.requestURL);                
                }
                validEntries++;
            }
        } else {
            invalidEntries++;
        }
    }
    //printf("successfully processed %d lines\n", validEntries);
    //printf("failed to process %d lines\n", invalidEntries);
    //printf("total lines processed %d\n", invalidEntries + validEntries);
    fclose(logFilePtr);
    
    return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
    int status;
    result info;
    struct tm tmStart = {0};
    struct tm tmEnd = {0};
    time_t timeStart = {0};
    time_t timeEnd = {0};
    const char *inputLog;
    bool timeConvert = false;
    //Bonus feature to allow a time span for conversion
    if (argc == 4) {
        if ((sscanf(argv[2], "%d:%d:%d:%d", &tmStart.tm_mday, &tmStart.tm_hour,
            &tmStart.tm_min, &tmStart.tm_sec) != NUM_TIME_ENTRIES) ||
            (sscanf(argv[3], "%d:%d:%d:%d", &tmEnd.tm_mday, &tmEnd.tm_hour,
            &tmEnd.tm_min, &tmEnd.tm_sec) != NUM_TIME_ENTRIES)) {
            printf("expected string format is DD:HH:MM:SS \n");
            return EXIT_FAILURE;
        } else {
            tmStart.tm_mon = 7; //necessary to fill struct
            tmStart.tm_year = 2024-1900;
            tmStart.tm_isdst = -1;
            tmEnd.tm_mon = 7;
            tmEnd.tm_year = 2024-1900;
            tmEnd.tm_isdst = -1;
            timeStart = mktime(&tmStart);
            timeEnd = mktime(&tmEnd);
            if (timeStart == -1 || timeEnd == -1) {
                printf("Time input failed to convert\n");
                return EXIT_FAILURE;
            }
            if (timeStart > timeEnd) {
                printf("Start time cannot be greater than End time\n");
                return EXIT_FAILURE;
            }
            //printf("start time: %s\n", ctime(&timeStart));
            //printf("end time: %s\n", ctime(&timeEnd));          
            timeConvert = true;
            inputLog = argv[1];
        } 
    } else if (argc == 2) {
            inputLog = argv[1];
    } else {
        printf("Expected 1 or 3 additional arguements: (1)Logfile and/or "
            "(2)start + (3)end time for log filtering\n");
            return EXIT_FAILURE;
    }

    //initialize values and tables
    info.numHostAccessCapacity = 2; // Dynamic table initial size
    info.numHostAccessSize = 0; // 0 entries
    info.numHostAccess = malloc(info.numHostAccessCapacity*sizeof(keyPair));
    info.numGetCapacity = 2; // Dynamic table initial size
    info.numGetSize= 0; // 0 entries
    info.numGet = malloc(info.numGetCapacity*sizeof(keyPair));

    if (!info.numGet || !info.numHostAccess) {
        printf("malloc failed");
        return EXIT_FAILURE;
    }

    status = logParse(inputLog, &info, timeStart, timeEnd, timeConvert);

    qsort(info.numHostAccess, info.numHostAccessSize, sizeof(keyPair), compareByValue);
    qsort(info.numGet, info.numGetSize, sizeof(keyPair), compareByValue);

    printf("Generating report:\n");
    printf("Number of host accesses:\n");
    printKeyValuePairs(info.numHostAccess, info.numHostAccessSize);
    printf("\nNumber of GET requests with a return code of OK\n");
    printKeyValuePairs(info.numGet, info.numGetSize);    

    //free memory
    free(info.numHostAccess);
    free(info.numGet);
    return status;
}