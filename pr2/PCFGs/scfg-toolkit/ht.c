#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "ht.h"

/**************************************************************/
void ErrorHashTab(int nerror, char *msg) {
  char *error[] = {
    "Not enough memory",
    "Repeated value in hash table:",
    "Internal error in hash table",
  };
  
  fprintf(stderr,"Error: %s %s\n",error[nerror],msg);
  exit(-1);
}

/**************************************************************/
/*** [Cormen,90] **********************************************/
int hashFunc (char *str, int size) {
#define A 0.61803398874989484820 /* (sqrt(5) - 1) / 2 */
  
  double val, base;
  
  /* A = (sqrt(5) -1) / 2; */
  val = 0;
  base = 1;
  while (*str != '\0') {
    val += base * (*str);
    base *= 2;
    str++;
  }
  return((int) floor(size * modf(val * A, &base)));
}

/**************************************************************/
int CreateHashTab(THashTable *T) {
  int i;
  
  if (((*T).v = (Tnode **) malloc((*T).size * sizeof(Tnode *))) == NULL)
    ErrorHashTab(0, "");
  
  for (i=0; i<(*T).size; i++)
    (*T).v[i] = NULL;

  return(0);
}

/**************************************************************/
int isInHashTab(char *str, THashTable T) {
  Tnode *head;
  int numSlot;

  numSlot = hashFunc(str, T.size); 
  head = T.v[numSlot];
  while ((head != NULL) && (strcmp(str, head->str) != 0))
    head = head->next;
  
  if (head != NULL)
    return(head->val);
  else
    return(-1);
}

/**************************************************************/
void InsertInHashTab(char *str, int ocu, THashTable T) {
  Tnode *head;
  int numSlot;

  head = NULL;
  if ((head = (Tnode *) malloc(sizeof(Tnode))) == NULL)
    ErrorHashTab(0,"");
  if (((head->str) = (char *) malloc((1+strlen(str)) * sizeof(char))) == NULL)
    ErrorHashTab(0,"");
  numSlot = hashFunc(str, T.size); 
  strcpy(head->str, str);
  head->val = ocu;
  head->next = T.v[numSlot];
  T.v[numSlot] = head;
}

/**************************************************************/
int DeleteFromHashTab(char *str, THashTable T) {
  Tnode *head1,*head2;
  int val, numSlot;

  numSlot = hashFunc(str, T.size); 

  head1 = T.v[numSlot];
  head2 = NULL; 
  while (strcmp(head1->str, str) != 0) {
    head2 = head1;
    head1 = head1->next;
  }

  if (head2 == NULL)
    T.v[numSlot] = head1->next;
  else
    head2->next = head1->next;

  val = head1->val;
  free(head1->str);
  free(head1);
  return(val);
}

/**************************************************************/
void WriteHashTab(THashTable T, char ***VectStr) {
  Tnode *head;
  int i;

  if (((*VectStr) = (char **) malloc(T.size * sizeof(char *))) == NULL)
    ErrorHashTab(0,"");

  for (i=0; i<T.size; i++)
    (*VectStr)[i] = NULL;

  for (i=0; i<T.size; i++) {
    head = T.v[i];
    while (head != NULL)
      if ((*VectStr)[head->val] != NULL)
	ErrorHashTab(2,"");
      else {
        if (((*VectStr)[head->val] = (char *) malloc((1+strlen(head->str)) * sizeof(char))) == NULL)
          ErrorHashTab(0,"");
        strcpy((*VectStr)[head->val], head->str);
	head = head->next;
      }
  }
}











