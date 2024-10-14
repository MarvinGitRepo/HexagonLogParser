/*
    Log File Parsing header for Hexagon
    Written by Marvin Lee
*/

#ifndef LOGPARSER_H
#define LOGPARSER_H

#define MAX_LOG_LINE_LENGTH 256
#define MAX_KEY_LENGTH  80
#define MAX_DATE_LENGTH     16
#define MAX_STRING_LENGTH   80
#define NUM_ENTRY_TYPES     7
#define NUM_TIME_ENTRIES    4
#define HTTP_CODE_OK        200

//static const char inputLog[] = "./src/LogFile";

typedef struct keyPair_t {
    char key[MAX_KEY_LENGTH];
    int value;
} keyPair;

typedef struct logEntry_t {
	char hostName[MAX_KEY_LENGTH];
	struct tm time;
    int timezoneHour;
    int timezoneMin;
    char request[MAX_STRING_LENGTH];
    char requestMethod[MAX_STRING_LENGTH];
    char requestURL[MAX_STRING_LENGTH];
    char requestProtocol[MAX_STRING_LENGTH];
    int returnCode;
    int returnSize;
} logEntry;

typedef struct results_t {
    keyPair *numHostAccess;
    int      numHostAccessCapacity;
    int      numHostAccessSize;
    keyPair *numGet;
    int      numGetCapacity;
    int      numGetSize;    
} result;

#endif /* MYHEADER_H */