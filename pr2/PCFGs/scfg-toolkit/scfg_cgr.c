#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>

#include "cfg.h"
#include "error.h"

/**************************************************************/
void CreateGram(TGram G, TTabLog T) {
  
  int i, j, k;
  Rule1 *nr1, *aux1;
  Rule2 *nr2, *aux2;
  int Total, val;
  double Factor;

  for (i=0; i<G.NoTer; i++) {
    Total = MINLOG;
    if ((nr1  = (Rule1 *) malloc(sizeof(Rule1))) == NULL)
      Error(1, "");
    aux1 = nr1;
    for (j=0; j<G.NoTer; j++)
      for (k=0; k<G.NoTer; k++) {
	if ((nr1->next = (Rule1 *) malloc(sizeof(Rule1))) == NULL)
	  Error(1,"");
	nr1->next->con1 = j;
	nr1->next->con2 = k;
	val = (int) (log((double) ((float)rand()/RAND_MAX)) 
		     / T.logBase);
	nr1->next->pro = val;
	Total = LogAdd(Total, val, T);
	nr1 = nr1->next;
      }
    nr1->next = NULL;
    G.a[i] = aux1->next;
    free(aux1);
    aux1 = NULL;
    
    if ((nr2  = (Rule2 *) malloc(sizeof(Rule2))) == NULL)
      Error(1,"");
    aux2 = nr2;
    for (j=0; j<G.Ter; j++) {
      if ((nr2->next = (Rule2 *) malloc(sizeof(Rule2))) == NULL)
	Error(1,"");
      nr2->next->con = j;
      val = (int) (log((double) ((float)rand()/RAND_MAX)) / T.logBase);
      nr2->next->pro = val;
      Total = LogAdd(Total, val, T);
      nr2 = nr2->next;
    }
    nr2->next = NULL;
    G.b[i] = aux2->next;
    free(aux2);
    aux2 = NULL;
    
    /* NORMALIZATION */
    
    Factor = (1 - exp(Total * T.logBase)) / exp(Total * T.logBase);
    
    nr1 = G.a[i];
    while (nr1 != NULL) {
      nr1->pro = (int)(log(exp(nr1->pro * T.logBase) * (1.0+Factor)) / 
		       T.logBase);
      nr1 = nr1->next;
    }
    nr2 = G.b[i];
    while (nr2 != NULL) {
      nr2->pro = (int)(log(exp(nr2->pro * T.logBase) * (1.0+Factor)) / 
		       T.logBase);
      nr2 = nr2->next;
    } 
  }
}

/**************************************************************/
void CompleteGram(TGram G) {

  int j;
  Rule1 *nr1;
  Rule2 *nr2;
  int * VecInt;
  int found;

  if ((VecInt = (int *) malloc(G.Ter*sizeof(int))) == NULL) {
    fprintf(stderr,"Error: not enought space creating vector.\n");
    exit(-1);
  }
 
  found = 0;
  for (nr1=G.a[0]; nr1!=NULL; nr1=nr1->next)
    if ((nr1->con1 == 0) && (nr1->con2 == 0))
      found = 1;
  if (found == 0) {
    if ((nr1 = (Rule1 *) malloc(sizeof(Rule1))) == NULL)
      Error(1,"");
    nr1->con1 = 0;
    nr1->con2 = 0;
    nr1->pro = MINLOG;
    nr1->next = G.a[0];
    G.a[0] = nr1;
  }
  
  for (j=0; j<G.Ter; j++)
    VecInt[j] = MINLOG;
  for (nr2=G.b[0]; nr2!=NULL; nr2=nr2->next)
    VecInt[nr2->con] = nr2->pro;
  for (j=0; j<G.Ter; j++)
    if (VecInt[j] == MINLOG) {
      if ((nr2 = (Rule2 *) malloc(sizeof(Rule2))) == NULL)
	Error(1,"");
      nr2->con = j;
      nr2->pro = MINLOG;
      nr2->next = G.b[0];
      G.b[0] = nr2;
    }
}

/**************************************************************/
void Syntax(char *command) {

  fprintf(stdout,"\nUsage: %s -g gr1 [-f gr2]",command);
  fprintf(stdout," [-a] [-c smooth] [-s seed]\n\n");
  fprintf(stdout,"-g gr1:\t\tfile with initial grammar.\n");
  fprintf(stdout,"\t\tThe rules can be omitted.\n");
  fprintf(stdout,"-f gr2:\t\tfile with final grammar (gr1 by default).\n");
  fprintf(stdout,"-a:\t\tkeep rules with null probability (no by default).\n");
  fprintf(stdout,"-c smooth:\tcomplete a grammar with smooth value for null rules.\n");
  fprintf(stdout,"-s seed:\tseed for the generator (1 by default).\n\n");
  fprintf(stdout,"Example of the header of gr1:\n\
# Comments\n\
NonTerminals 3\n\
Nt1\n\
# Nt1 is the initial non terminal\n\
Nt2\n\
Nt3\n\
Terminals 2\n\
T1\n\
T2\n\
Rules\n\
...\n\n");
  exit(0);
}

/**************************************************************/
int main(int argc,char *argv[]){
  
  extern char *optarg;
  extern int opterr;
  TGram G;
  char gram1[MAXCAD], gram2[MAXCAD];
  int option, seed, all, val_smo;
  TTabLog T;
  
  strcpy(gram1,"");
  strcpy(gram2,"");
  seed = 1;
  all = false;
  val_smo = MINLOG;
  CreateTabLog(1.0001, &T);

  if (argc == 1) Syntax(argv[0]);
  while ((option=getopt(argc,argv,"ag:f:s:he:c:")) != EOF ) {
    switch(option) {
    case 'a': all = true;break;
    case 'c': val_smo = (int) (log(atof(optarg))/T.logBase);break;
    case 'g': strcpy(gram1,optarg);strcpy(gram2,gram1);break;
    case 'f': strcpy(gram2,optarg);break;
    case 's': seed = atoi(optarg);break;
    case 'h': Syntax(argv[0]);
    default:  Syntax(argv[0]);
    }
  }

  G.Base = 1.0001;
  ReadGram(&G, gram1);
  srand(seed);
  
  if (val_smo == MINLOG)
    CreateGram(G, T);
  else {
    CompleteGram(G);
    Smooth(G, val_smo, T);
  }
  WriteGram(G, gram2, all, argc, argv);
  return(0);
  
}

