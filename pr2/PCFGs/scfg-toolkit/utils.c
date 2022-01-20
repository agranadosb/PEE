#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <math.h>
#include "utils.h"
#include "error.h"

/**************************************************************/
int DeleteWhtSpaces(char *str) {
  int i = 0;
  char *aux;

  if (*str != comment) {
    aux=str;
    while ((*aux == ' ') && (*aux != '\0')) aux++;
    while (*aux != '\0') {
      *str = *aux;
      str++;
      aux++;
      i++;
    }
    *str = '\0';
    str--;
    while ((*str == ' ') || (*str == '\n')) {
      *str = '\0';
      str--;
      i--;
    }
  }
  return(i);
}
