#include "ht.h"

#define true  0
#define false 1
#define MAXCAD 4096 /* Default string size */
#define TATREL 1024 /* Default trellis size */
#define comment '#'
#define arrow "-->"
#define delim " "

/* Delete initial and final white spaces. */
extern int DeleteWhtSpaces(char *str);

