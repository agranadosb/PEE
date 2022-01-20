#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

#include "cfg.h"
#include "error.h"

/******************************************/
void InitializeAcumIO(TGram G, int **Deno) {
  int i;
  Rule1 *reg1;
  Rule2 *gre1;
  
  if ((*Deno) != NULL)
    free(*Deno);
  if (((*Deno) = (int *) calloc(G.NoTer,sizeof(int))) == NULL)
    Error(1,"");
  
  for (i=0; i<G.NoTer; i++) {
    for (reg1 = G.a[i]; reg1 != NULL; reg1 = reg1->next)
      reg1->frec = MINLOG;
    for (gre1 = G.b[i]; gre1 != NULL; gre1 = gre1->next) 
      gre1->frec = MINLOG;
    (*Deno)[i] = MINLOG;
  }
}

/******************************************/
void InitializeAcumVS(TGram G, int **Deno) {
  int i;
  Rule1 *reg1;
  Rule2 *gre1;
  
  if ((*Deno) != NULL)
    free(*Deno);
  if (((*Deno) = (int *) calloc(G.NoTer,sizeof(int))) == NULL)
    Error(1,"");
  
  for (i=0; i<G.NoTer; i++) {
    for (reg1 = G.a[i]; reg1 != NULL; reg1 = reg1->next)
      reg1->frec = 0;
    for (gre1 = G.b[i]; gre1 != NULL; gre1 = gre1->next) 
      gre1->frec = 0;
    (*Deno)[i] = 0;
  }
}

/*******************************************************/
void Accumulate(Trellis e, Trellis f, TGram G, int *Deno,
	      int *index, int lon, TTabLog T, TTree tree) {
  
  int Prob, Acum, i, j, k, l, n;
  Rule1 *reg1;
  Rule2 *gre1;
  
  Prob = e[0][lon - 1][0];
  
  for(k=0; k<G.NoTer; k++) {
    reg1 = G.a[k];
    while (reg1 != NULL) {
      Acum = MINLOG;
      for (n=0; n<tree.lastNode; n=n+tree.l[n]+1) /* ACÍ */
	for (i=1; i<tree.l[n]; i++)
	  for (j=0; j<tree.l[n] - i; j++)
	    for (l=n+1+j; l<n+1+j+i; l++)
	      Acum = 
     LogAdd(Acum,
	    LogProduct(reg1->pro,
		       LogProduct(e[tree.l[n+1+j]][tree.r[l]][reg1->con1],
				  e[tree.l[l+1]][tree.r[n+1+j+i]][reg1->con2],
				  f[tree.l[n+1+j]][tree.r[n+1+j+i]][k]),
		       0),
	    T);
      reg1->frec = LogAdd(reg1->frec,LogProduct(Acum, -Prob, 0), T);
      reg1 = reg1->next;
    }
    gre1 = G.b[k];
    while (gre1 != NULL) {
      Acum = MINLOG;
      for (j=0; j<lon; j++)
        if (index[j] == gre1->con)
          Acum = LogAdd(Acum, LogProduct(e[j][j][k], f[j][j][k], 0), T);
      gre1->frec = LogAdd(gre1->frec,LogProduct(Acum, -Prob, 0), T);
      gre1 = gre1->next;
    }

    Acum = MINLOG;

    for (n=0; n<tree.lastNode; n=n+tree.l[n]+1) /* ACÍ */
      for (i=0; i<tree.l[n]-1; i++)
	for (j=0; j<tree.l[n] - i; j++)
	  Acum = LogAdd(Acum, 
			LogProduct(f[tree.l[n+1+j]][tree.r[n+1+j+i]][k], 
				   e[tree.l[n+1+j]][tree.r[n+1+j+i]][k],0), T);
    Acum = LogAdd(Acum,LogProduct(f[0][lon-1][k],e[0][lon-1][k],0), T);
    Deno[k] = LogAdd(Deno[k], LogProduct(Acum, -Prob, 0), T);
  }
}

/***************************************************************/
void OuterSpan(Trellis f, Trellis e, TGram G, int lon, TTabLog T, 
	       TTree tree) {
  Rule1 *reg1;
  int i, j, k, l, *ll, *rr, n;
  
  /* First, the list of nodes of the tree is reordered in order 
     to be processed adequately. */ 
  ll = (int *) malloc(tree.lastNode * sizeof(int));
  rr = (int *) malloc(tree.lastNode * sizeof(int));
  
  for (i=0; i<tree.lastNode; i=i+tree.l[i]+1) {
    ll[tree.lastNode-tree.l[i]-i-1] = tree.l[i];
    for (j=0; j<tree.l[i]; j++) {
      ll[tree.lastNode-tree.l[i]-i+j] = tree.l[i+1+j];
      rr[tree.lastNode-tree.l[i]-i+j] = tree.r[i+1+j];
    }
  }

  for (k=0; k<G.NoTer; k++)
    f[0][lon-1][k] = MINLOG;
  f[0][lon - 1][0] = 0;

  for (n=0; n<tree.lastNode-1; n=n+ll[n]+1)
    for (i=ll[n]-2; i>=0; i--)
      for (j=0; j<ll[n] - i; j++) {
	for (k=0; k<G.NoTer; k++)
	  f[ll[n+1+j]][rr[n+1+j+i]][k] = MINLOG;
	for (k=0; k<G.NoTer; k++) {
	  reg1 = G.a[k];
	  while (reg1 != NULL) {
	    for (l=n+1; l<n+1+j; l++)
	      f[ll[n+1+j]][rr[n+1+j+i]][reg1->con2] = 
		LogAdd(f[ll[n+1+j]][rr[n+1+j+i]][reg1->con2],
		       LogProduct(f[ll[l]][rr[n+1+j+i]][k],
				  reg1->pro,
				  e[ll[l]][rr[n+1+j-1]][reg1->con1]),
		       T);
	    for (l=n+1+j+i+1; l<n+1+ll[n]; l++)
	      f[ll[n+1+j]][rr[n+1+j+i]][reg1->con1] =
		LogAdd(f[ll[n+1+j]][rr[n+1+j+i]][reg1->con1],
		       LogProduct(f[ll[n+1+j]][rr[l]][k],
				  reg1->pro,
				  e[ll[n+1+j+i+1]][rr[l]][reg1->con2]),
		       T);
	    reg1 = reg1->next;
	  }
	}
      }
  free(ll); free(rr);

}

/**************************************/
void  ReestimateIO(TGram G, int *Deno) {
  int i;
  Rule1 *reg1;
  Rule2 *gre1;
  
  for (i=0; i<G.NoTer; i++) {
    reg1 = G.a[i];
    while (reg1 != NULL) {
      reg1->pro = LogProduct(reg1->frec, -Deno[i], 0);
      reg1 = reg1->next;
    }
    gre1 = G.b[i];
    while (gre1 != NULL) {
      gre1->pro = LogProduct(gre1->frec, -Deno[i], 0);
      gre1 = gre1->next;
    }
  }
}

/**************************************/
void  ReestimateVS(TGram G, int *Deno) {
  int i;
  Rule1 *reg1;
  Rule2 *gre1;
  
  for (i=0; i<G.NoTer; i++)
    if (Deno[i] != 0) {
      reg1 = G.a[i];
      while (reg1 != NULL) {
	if (reg1->frec == 0)
	  reg1->pro = MINLOG;
        else
          reg1->pro = (int) (log(((double) reg1->frec/Deno[i]))/log(G.Base));
        reg1 = reg1->next;
      }

      gre1 = G.b[i];
      while (gre1 != NULL) {
	if (gre1->frec == 0)
	  gre1->pro = MINLOG;
        else
          gre1->pro = (int) (log(((double) gre1->frec/Deno[i]))/log(G.Base));
        gre1 = gre1->next;
      }
    }
    else {
      reg1 = G.a[i];
      while (reg1 != NULL) {
        reg1->pro = MINLOG;
        reg1 = reg1->next;
      }
      gre1 = G.b[i];
      while (gre1 != NULL) {
        gre1->pro = MINLOG;
        gre1 = gre1->next;
      }
    }
}


/*********************************************************************/
void Analyse (int prod, int Li, int Ld, TGram G, int *index, Trellis Q,
	      Trellis F1, Trellis F2, Trellis S, int *Deno) {

  Rule2 *gre;
  Rule1 *reg;
  int l, pi, pd;

  if ((Ld - Li) == 0)
    if ((gre = Search_b(prod, index[Li], G.b)) != NULL) {
      gre->frec = gre->frec + 1;
      Deno[prod] = Deno[prod] + 1;
    }
    else
      Warning(2, "");
  else {
    l = S[Li][Ld][prod];
    pi = F1[Li][Ld][prod];
    pd = F2[Li][Ld][prod];
    if ((reg = Search_a(prod, pi, pd, G.a)) != NULL) {
      reg->frec = reg->frec + 1;
      Deno[prod] = Deno[prod] + 1;
      Analyse(pi, Li, l, G, index, Q, F1, F2, S, Deno);
      Analyse(pd, l+1, Ld, G, index, Q, F1, F2, S, Deno);
    }
    else
      Warning(2, "");
  }
}

/****************************************************************/
void Parsing (TGram G, int lsam, int *index, int *Deno, Trellis Q, 
	      Trellis F1, Trellis F2, Trellis S, TTree tree) {
  int l, pi, pd;
  Rule1 *reg;
  Rule2 *gre;

  ViterbiSpan(G, lsam, index, Q, F1, F2, S, tree);

  if (Q[0][lsam - 1][0] == MINLOG)
    Warning(0, "");
  else
    if (lsam == 1)
      if ((gre = Search_b(0, index[0], G.b)) != NULL) {
        Deno[0] = Deno[0] + 1;
        gre->frec = gre->frec + 1;
      }
      else
        Warning(2, "");
    else {
      l = S[0][lsam - 1][0];
      pi = F1[0][lsam - 1][0];
      pd = F2[0][lsam - 1][0];
      if ((reg = Search_a(0, pi, pd, G.a)) != NULL) {
        reg->frec = reg->frec + 1;
        Deno[0] = Deno[0] + 1;
        Analyse(pi, 0, l, G, index, Q, F1, F2, S, Deno);
        Analyse(pd, l+1, lsam-1, G, index, Q, F1, F2, S, Deno);
      }
      else
        Warning(2, "");
    }
}

/**************************/
void Syntax(char *command) {
  
  fprintf(stdout,"\nUsage: %s -g gram1 [-f gram2] -m samples [-i ite]",command);
  fprintf(stdout," [{-s|-S} smooth] [-l] [-e TaTre] [-v] [-a] [-b base]\n\n");
  fprintf(stdout,"-a:\t\tkeep null rules (no, by default).\n");
  fprintf(stdout,"-g gram1:\tinitial grammar.\n");
  fprintf(stdout,"-f gram2:\tfinal grammar (gram1, by default).\n");
  fprintf(stdout,"-m sample:\tsample file.\n");
  fprintf(stdout,"-i ite:\t\titeration number (1, by default).\n");
  fprintf(stdout,"-s|-S smooth:\tsmooth value (0, by default).\n");
  fprintf(stdout,"\t\t-s: smooth after each iteration.\n");
  fprintf(stdout,"\t\t-S: smooth after last iteration.\n");
  fprintf(stdout,"-l:\t\tshow log likelihood with the input grammar.\n");
  fprintf(stdout,"-e TaTre:\ttrellis size (%d, by default).\n",TATREL);
  fprintf(stdout,"-v:\t\tuse viterbi (inside-outside by default).\n");
  fprintf(stdout,"-a:\t\tkeep null rules (no, by default).\n");
  fprintf(stdout,"-b base:\tbase of the logarithm (1.0001, by default).\n\n");
  
  exit(0);
}

/************************************************************************/
/************************************************************************/
/************************************************************************/

int main(int argc,char *argv[]) {  
  extern char *optarg;
  extern int opterr;
  TGram G;
  Rules1 a_null;
  Rules2 b_null;
  char gram1[MAXCAD], gram2[MAXCAD];
  int  i, option, ite, lsam, all, smooth, *Deno=NULL, *index, 
    SmoCadaIte, SacaVero, TaTrel, CCat=0, vite=0;
  char cad1[MAXCAD], fmu[MAXCAD];
  Trellis e, f, F1, F2, S;
  FILE *ficm;
  double VeroTotal;
  TTabLog T;
  float smoothF;
  TTree tree;

  smooth = MINLOG;
  SmoCadaIte = true;
  ite = 1;
  strcpy(gram1,"");
  strcpy(gram2,"");
  strcpy(fmu,"");
  all = false;
  VeroTotal = 0.0;
  SacaVero = false;
  TaTrel = TATREL;
  smoothF = 0.0;
  T.Base = 1.0001;
 
  if (argc == 1)
    Syntax(argv[0]);
  while ((option=getopt(argc,argv,"lahg:f:m:i:s:S:e:vb:")) != EOF ) {
    switch(option) {
    case 'g': strcpy(gram1,optarg);strcpy(gram2,gram1); break;
    case 'f': strcpy(gram2,optarg); break;
    case 'm': strcpy(fmu,optarg); break;
    case 'i': ite = atoi(optarg); break;
    case 'e': TaTrel = atoi(optarg); break;
    case 'v': vite = 1; break;
    case 's': smoothF = atof(optarg); SmoCadaIte = true; break;
    case 'S': smoothF = atof(optarg); SmoCadaIte = false; break;
    case 'b': T.Base = atof(optarg); break;
    case 'a': all = true; break;
    case 'l': SacaVero = true; break;
    case 'h': Syntax(argv[0]); break;
    default: Syntax(argv[0]);
    }
  }

  CreateTabLog(T.Base, &T);

  if (smooth != MINLOG)
    smooth = (int)(log((double)smoothF)/T.logBase);

  G.Base = T.Base;
  ReadGram(&G, gram1);

  MemNullRules(&a_null, &b_null, G.NoTer);
  if (smooth > MINLOG)
    Smooth(G, smooth, T);
 
  if ((index = (int*) malloc(TaTrel*sizeof(int))) == NULL) 
    Error(1,"");

  Memory(&e, G.NoTer, TaTrel);
  if (vite == 0) {
    Memory(&f, G.NoTer, TaTrel);
  }
  else {
    Memory(&F1, G.NoTer, TaTrel);
    Memory(&F2, G.NoTer, TaTrel);
    Memory(&S, G.NoTer, TaTrel);
  }

  tree.maxNodes=5*TaTrel;
  tree.l = (int *) malloc(tree.maxNodes * sizeof(int));
  tree.r = (int *) malloc(tree.maxNodes * sizeof(int));
  

  for (i=0; i<ite; i++) {
    CCat = 0;

    if ((ficm = fopen(fmu,"r")) == NULL) Error(0, fmu);
    
    VeroTotal = 0.0;
 
    if (vite == 0)
      InitializeAcumIO(G, &Deno);
    else
      InitializeAcumVS(G, &Deno);
    while ((fgets(cad1, MAXCAD, ficm) != NULL)) {
      CCat++; 
      cad1[strlen(cad1) - 1] = '\0';
      if (strlen(cad1) >= MAXCAD) fprintf(stderr,"Warning: string number %d too long.\n",CCat);

      tree.lastNode = 0;
      if ((lsam = IndexSpan(cad1, G, index, &tree, TaTrel)) != 0) {
	if (vite == 0) {
	  InnerSpan(e, G, index, lsam, T, tree);
	  /* fprintf(stdout,"%e ",exp(e[0][lsam - 1][0] * T.logBase)); 
	     fflush(stdout); */
	  OuterSpan(f, e, G, lsam, T, tree); 
	  /* VeroTotal = MINLOG;
	     for (all=0; all<G.NoTer; all++)
	     if ((rule=Search_b(all, index[0], G.b)) != NULL)
	     VeroTotal = LogAdd(VeroTotal,
	     LogProduct(f[0][0][all],rule->pro, 0),
	     T);
	     fprintf(stdout,"%e\n",exp(VeroTotal * T.logBase)); 
	     fflush(stdout);
	     exit(-1); */
	  if (e[0][lsam - 1][0] <= MINLOG) {
            fprintf(stderr,"Warning: string number %d .\n",CCat);
            fflush(stderr);
	    Warning(0, cad1);
          }
	  else {
	    Accumulate(e, f, G, Deno, index, lsam, T, tree);
	    VeroTotal += ((double) e[0][lsam - 1][0]) * T.logBase;
	  }
	}
	else {
	  Parsing(G, lsam, index, Deno, e, F1, F2, S, tree);
	  if (e[0][lsam - 1][0] <= MINLOG) {
            fprintf(stderr,"Warning: string number %d .\n",CCat);
            fflush(stderr);
	    Warning(0, cad1);
          }
	  else
	    VeroTotal += ((double) e[0][lsam - 1][0]) * T.logBase;
	}
      } 
      else {
        fprintf(stderr,"Warning: problems scanning string number %d .\n",CCat);
        fflush(stderr);
        Warning(1,cad1); 
      }
    }
    if (vite == 0)
      ReestimateIO(G, Deno);
    else 
      ReestimateVS(G, Deno);
    if ((smooth > MINLOG) && (SmoCadaIte == true))
      Smooth(G, smooth, T);
    else
      DeleteNullRules(&G, a_null, b_null);

    if (SacaVero == true)
      fprintf(stdout,"%e\n",VeroTotal);
    fflush(stdout);
    fclose(ficm);
    WriteGram(G, gram2, all, argc, argv);
  }

  if (all == true)
    InsertNullRules(&G, a_null, b_null);
  if (smooth > MINLOG)
    if (SmoCadaIte == false) 
      Smooth(G, smooth, T);
  WriteGram(G, gram2, all, argc, argv);
  
  return(0);
}
