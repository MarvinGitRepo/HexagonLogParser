Marvin's webserver log parser

Info:
Developed in Ubuntu 24.04, gcc compiler needed

Steps to compile:
use "make" in top directory:
Example: 
    make
    gcc src/logParser.c -o output/logParser

Use "make clean" to remove output

How to use after compile:
src/LogFile is the location of the input for my testing, other files can be used by passing the path to the program

How to use in normal case:

pass in path to file
./output/logParser ./src/LogFile 
    Generating report:
    Number of host accesses:
    wpbfl2-45.gate.net 6
    ix-mia5-17.ix.netcom.com 3
    140.112.68.165 1


    Number of GET requests with a return code of OK
    /OWOW/ 3
    /logos/us-flag.gif 1
    /information.html 1
    /icons/people.gif 1
    /Access/ 1
    /waisicons/unknown.gif 1


Bonus use case with a timing window to capture results (use DD/HH/MM/SS as time input):
./output/logParser ./src/LogFile 29:23:55:30 29:23:57:08
    Generating report:
    Number of host accesses:
    wpbfl2-45.gate.net 5
    ix-mia5-17.ix.netcom.com 2
    140.112.68.165 1


    Number of GET requests with a return code of OK
    /OWOW/ 2
    /logos/us-flag.gif 1
    /information.html 1
    /icons/people.gif 1
    /Access/ 1
    /waisicons/unknown.gif 1


Additional testing for invalid use case:

Incorrect number of arguements:
    ./output/logParser 
    Expected 1 or 3 additional arguements: (1)Logfile and/or (2)start + (3)end time for log filtering

Having the Start time be greater than End time:
    ./output/logParser ./src/LogFile 29:23:57:08  29:23:55:30 
    Start time cannot be greater than End time

Incorrect or missing logfile:
    ./output/logParser ./src/123
    error opening file: ./src/123 errno: No such file or directory

Was not able to integreate CUnit for unit test case generation, but that could be a next step to ensure quality for something like the parser