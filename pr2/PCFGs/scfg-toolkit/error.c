#include <stdio.h>
#include <stdlib.h>

/**************************************************************/
void Error(int nerror, char *msg) {
  char *error[] = {
    "File not found: ",
    "Problems in memory allocation",
    "Incorrect grammar format",
    "Incorrect number of non-terminals",
    "Not enough non-terminals: ",
    "Non-terminal not found: ",
    "Terminal not found: ",
    "Omitted mandatory parameters",
    "Information about file not available: ",
    "Empty file: ",
    "Different number of terminals. ",
    "Write the terminal in the same order",
    "Division by zero.",
    "Incorrect number of bytes.",
    "Repeated non-terminal",
    "Repeated terminal",
  };
  
  fprintf(stderr, "Error: %s %s\n", error[nerror], msg);
  exit(-1);
}

/**************************************************************/
void Warning(int nerror,char *msg) {
  char *error[] = {
    "Sample not accepted by the grammar: ",
    "Sample with null length",
    "Probably incorrect parsing",
    "Negative frequencies",
    "Sample too long",
    "Precission error expanding non-terminal.",
    "Incorrecto value.",
  };
  
  fprintf(stderr,"Warning: %s %s\n", error[nerror], msg);
}  

