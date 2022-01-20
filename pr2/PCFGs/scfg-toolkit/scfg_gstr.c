#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <getopt.h>

#include "cfg.h"
#include "error.h"

/*****************************************************************************/
void Expand(int noTer, char *separator, TGram G, 
	      char **VNT, char **VT, TTabLog T) {
  int ValAl, WAct;
  Rule1 *reg;
  Rule2 *gre;

  ValAl = (int) (log((double) ((float)rand()/RAND_MAX)) / T.logBase);
  WAct = MINLOG;

  reg = G.a[noTer];
  while ((reg != NULL) && (WAct != 0)) {
    WAct = LogAdd(WAct, reg->pro, T);
    if (WAct >= ValAl) {
      if (strcmp(separator,"   ") != 0)
	fprintf(stdout,"%c",separator[0]);
      Expand(reg->con1, separator, G, VNT, VT, T);
      if (strcmp(separator,"   ") != 0)
	fprintf(stdout,"%c",separator[1]);
      Expand(reg->con2, separator, G, VNT, VT, T);
      if (strcmp(separator,"   ") != 0)
	fprintf(stdout,"%c",separator[2]);
      WAct = 0;
    }
    reg = reg->next;
  }

  if (WAct != 0) {
    gre = G.b[noTer];
    while ((gre != NULL) && (WAct != 0)) {
      WAct = LogAdd(WAct, gre->pro, T);
      if (WAct > ValAl) {
        if (strcmp(separator,"   ") != 0)
          fprintf(stdout,"%c",separator[0]);
        if (strcmp(separator,"   ") == 0)
          fprintf(stdout," ");
        fprintf(stdout,"%s",VT[gre->con]);
        if (strcmp(separator,"   ") != 0)
          fprintf(stdout,"%c",separator[2]);
        WAct = 0;
      }
      gre = gre->next;
    }
  }

  /* PARA EVITAR ERRORES DE PRECISION Y QUE NO SE SELECCIONE NINGUNA REGLA */
  /* INCLUIMOS LO SIGUIENTE                                                */

  if (WAct != 0) {
    Warning(5,VNT[noTer]);
    fprintf(stderr,"%e\n",exp(WAct * T.logBase));
    Expand(noTer, separator, G, VNT, VT, T);
  }
}

/******************************************************************************/
Tnode * ExpandAll(int noTer, int lon, char *separator,
  TGram G, char **VNT, char **VT) {

  Tnode *l, *laux, *list1, *list2, *rl1, *rl2;
  Rule1 *reg;
  Rule2 *gre;
  char sepa0[2], sepa1[2], sepa2[2];

  sepa0[0] = separator[0];
  sepa1[0] = separator[1];
  sepa2[0] = separator[2];
  sepa0[1] = sepa1[1] = sepa2[1] = '\0';

  l = NULL;
  
  gre = G.b[noTer];
  while (gre != NULL) {
    if ((laux = (Tnode *) malloc(sizeof(Tnode))) == NULL)
      Error(1,"");

    if (((laux->str) = (char *) malloc((4+strlen(VT[gre->con])) *
                           sizeof(char))) == NULL)
      Error(1,"");
    strcpy(laux->str,"");
    if (strcmp(sepa0," ") != 0)
      strcat(laux->str, sepa0);
    else
      strcat(laux->str," ");
    strcat(laux->str,VT[gre->con]);
    if (strcmp(sepa2," ") != 0) strcat(laux->str,sepa2);
    laux->next = l;
    l = laux;
    gre = gre->next;
  }

  if (lon > 0) {
    reg = G.a[noTer];
    while (reg != NULL) {
      list1 = ExpandAll(reg->con1, lon-1, separator, G, VNT, VT);
      list2 = ExpandAll(reg->con2, lon-1, separator, G, VNT, VT);
      rl1 = list1;
      while (rl1 != NULL) {
        rl2 = list2;
        while (rl2 != NULL) {
          if ((laux = (Tnode *) malloc(sizeof(Tnode))) == NULL)
            Error(1,"");

          if (((laux->str) = (char *) malloc((6+strlen(rl1->str)+
                  strlen(rl2->str)) * sizeof(char))) == NULL)
            Error(1,"");
          strcpy(laux->str,"");
          if (strcmp(sepa0," ") != 0)
            strcat(laux->str,sepa0);
          strcat(laux->str,rl1->str);
	  if (strcmp(sepa1," ") != 0) strcat(laux->str,sepa1);
          strcat(laux->str,rl2->str);
          if (strcmp(sepa2," ") != 0) strcat(laux->str,sepa2);
          laux->next = l;
          l = laux;
          rl2 = rl2->next;
        }
        rl1 = rl1->next;
      }

      reg = reg->next;

      rl1 = list1;
      while (rl1 != NULL) {
	rl2 = rl1;
	rl1 = rl1->next;
	free(rl2->str);
	free(rl2);
      }
      rl1 = list2;
      while (rl1 != NULL) {
	rl2 = rl1;
	rl1 = rl1->next;
	free(rl2->str);
	free(rl2);
      }
    }
  }
  return(l);
}

/******************************************************************************/

void Syntax(char *command) {
  fprintf(stdout,"\nUsage: %s -g gr [-s seed]",command);
  fprintf(stdout," [-c num] [-p xyz] [-l n]\n\n");
  fprintf(stdout,"-g gr:\t\tinput grammar.\n");
  fprintf(stdout,"-s seed:\tseed for the generator (1 by default).\n");
  fprintf(stdout,"-c num:\t\tnumber of samples to be generated (1 by default).\n");
  fprintf(stdout,"-p xyz:\t\tparentized samples.\n");
  fprintf(stdout,"\tx\topen character\n");
  fprintf(stdout,"\ty\tseparator character\n");
  fprintf(stdout,"\tz\tclose character\n");
  fprintf(stdout,"-l n:\t\tall strings until length n. Use this option with caution: exponential space complexity.\n\n");
  exit(0);
}

/******************************************************************************/

int main(int argc,char *argv[]) {   
  extern char *optarg;
  extern int opterr, optind;
  TGram G;
  char **VNT, **VT, gram1[MAXCAD], separator[4];
  int seed, option, i, lon, nsamples;
  Tnode *list;
  TTabLog T;

  seed = 1;
  nsamples = 1;
  lon = 0;
  strcpy(gram1,"");
  strcpy(separator,"   ");

  if (argc == 1) Syntax(argv[0]);
  while ((option=getopt(argc,argv,"hg:m:s:c:p:l:")) != EOF ) {
    switch(option) {
      case 'g': strcpy(gram1,optarg);break;
      case 's': seed = atoi(optarg);break;
      case 'l': lon = atoi(optarg);break;
      case 'c': nsamples = atoi(optarg);break;
      case 'p': strcpy(separator,optarg);break;
      case 'h': Syntax(argv[0]);
      default: Syntax(argv[0]);
    }
  }

  CreateTabLog(1.0001, &T);
  G.Base = 1.0001;
  ReadGram(&G, gram1);
  srand(seed);

  WriteHashTab(G.SNT, &VNT);
  WriteHashTab(G.ST, &VT);
  
  if (lon == 0) {
    for (i=0; i<nsamples; i++) {
      Expand(0, separator, G, VNT, VT, T);
      fprintf(stdout,"\n");
    }
  }
  else {
    list = ExpandAll(0, lon-1, separator, G, VNT, VT);
    while (list != NULL) {
      fprintf(stdout,"%s\n", list->str);
      list = list->next;
    }
  }
  return(0);
}
