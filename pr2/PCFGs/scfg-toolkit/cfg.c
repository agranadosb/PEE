#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "cfg.h"
#include "error.h"

/**************************************************************/
void ReverseList(TGram *G) {  
  Rule1 *reg1, *reg2;
  Rule2 *gre1, *gre2;
  int i;

  for (i=0; i<(*G).NoTer; i++) {
    reg1 = (*G).a[i];
    (*G).a[i] = NULL;
    while (reg1 != NULL) {
      reg2 = reg1->next;
      reg1->next = (*G).a[i];
      (*G).a[i] = reg1;
      reg1 = reg2;
    }
    gre1 = (*G).b[i];
    (*G).b[i] = NULL;
    while (gre1 != NULL) {
      gre2 = gre1->next;
      gre1->next = (*G).b[i];
      (*G).b[i] = gre1;
      gre1 = gre2;
    }
  }
}

/**************************************************************/
void ReadGram(TGram *G, char *NameFic) {
  FILE *fic;
  char str[MAXCAD], bas[15], aux1[1024], aux2[1024], aux3[1024],
    aux4[1024], aux5[1024];
  int i, found, PIZ, PD1, PD2, loncad, num_items;
  Rule1 *new_rule1;
  Rule2 *n_rule1;
  float logBase;

  logBase = log((*G).Base);
  
  if ((fic = fopen(NameFic,"r")) == NULL) Error(0, NameFic);

  (*G).NoTer = 0;
  (*G).Ter = 0;
  
  found = false;
  while ((found == false) && (fgets(str, MAXCAD, fic) != NULL)) {
    str[strlen(str) - 1] = '\0';
    if ((loncad = DeleteWhtSpaces(str)) != 0) {
      if ((strncmp(str,"No_Terminales",13) == 0) ||
	  (strncmp(str,"NonTerminals",12) == 0)) {
	sscanf(str,"%s %d",bas,&((*G).NoTer));
        /**********************************/
        /** HASH TABLE OF NON TERMINALS  **/
        /**********************************/
	(*G).SNT.size = (*G).NoTer;
        i = CreateHashTab(&((*G).SNT));
        i = 0;
        while ((fgets(str, MAXCAD, fic) != NULL) && (i < (*G).NoTer)) {
	  str[strlen(str) - 1] = '\0';
          if ((loncad = DeleteWhtSpaces(str)) != 0) {
            if (isInHashTab(str, (*G).SNT) < 0)
	      InsertInHashTab(str, i, (*G).SNT);
            else
	      Error(14, str);
	    i++;
	  }
	}
      } else
      if ((strncmp(str,"Terminales",10) == 0) ||
	  (strncmp(str,"Terminals",9) == 0)) {
	sscanf(str,"%s %d",bas,&((*G).Ter));
        /*****************************/
        /** HASH TABLE OF TERMINALS **/
        /*****************************/
	(*G).ST.size = (*G).Ter;
        i = CreateHashTab(&((*G).ST));
        i = 0;
	while ((fgets(str,MAXCAD,fic) != NULL) && (i < (*G).Ter)) {
	  str[strlen(str) - 1] = '\0';
	  if ((loncad = DeleteWhtSpaces(str)) != 0) {
            if (isInHashTab(str, (*G).ST) < 0)
	      InsertInHashTab(str, i, (*G).ST);
            else
	      Error(15, str);
	    i++;
          }
	}
      } else
      if ((strncmp(str,"Reglas",6) == 0) ||
	  (strncmp(str,"Rules",5) == 0)) found = true;
    }
  }

  if (((*G).NoTer == 0) || ((*G).Ter == 0)) { 
      fprintf(stderr,"%d %d\n",(*G).NoTer,(*G).Ter);
      Error(3,"");
  }
      
  /* MEMORY ALLOCATE */

  /***********/
  /** RULES **/
  /***********/
  if (((*G).a = (Rules1) malloc((*G).NoTer * sizeof(Rule1 *))) == NULL) 
    Error(1,"");
  for (i=0; i<(*G).NoTer; i++)
    (*G).a[i] = NULL;
  if (((*G).b = (Rules2) malloc((*G).NoTer * sizeof(Rule2 *))) == NULL) 
    Error(1,"");
  for (i=0; i<(*G).NoTer; i++)
    (*G).b[i] = NULL;


  /********************************************/ 
  /** READING RULES   *************************/
  /********************************************/ 
  while ((fgets(str,MAXCAD,fic) != NULL)) {
    str[strlen(str) - 1] = '\0';
    if ((loncad = DeleteWhtSpaces(str)) != 0) {
      if ((num_items = sscanf(str, "%s %s %s %s %s",
			      aux1, aux2, aux3, aux4, aux5)) == 5) {
	/** RULE p A --> B C **/
	PIZ = PD1 = PD2 = -1;
	if ((PIZ=isInHashTab(aux2,(*G).SNT)) < 0) Error(5,aux2);
        if ((PD1=isInHashTab(aux4,(*G).SNT)) < 0) Error(5,aux4);
        if ((PD2=isInHashTab(aux5,(*G).SNT)) < 0) Error(5,aux5);
	if ((new_rule1 = (Rule1 *) malloc(sizeof(Rule1))) == NULL)
	  Error(1,"");
	new_rule1->con1 = PD1;
	new_rule1->con2 = PD2;
	new_rule1->pro = (int) (log(atof(aux1)) / logBase);
	new_rule1->frec = 0;
	new_rule1->next = (*G).a[PIZ];
	(*G).a[PIZ] = new_rule1;
      }
      else {
	/** RULE p A --> b  **/
	PIZ = PD1 = -1;
	if ((PIZ = isInHashTab(aux2,(*G).SNT)) < 0) Error(5, aux2);
        if ((PD1=isInHashTab(aux4,(*G).ST)) < 0) Error(6,aux4);
	if ((n_rule1 = (Rule2 *) malloc(sizeof(Rule2))) == NULL)
	  Error(1,"");
	n_rule1->con = PD1;
	n_rule1->pro = (int) (log(atof(aux1)) / logBase);
	n_rule1->frec = 0;
	n_rule1->next = (*G).b[PIZ];
	(*G).b[PIZ] = n_rule1;
      }
    } 
  }
  fclose(fic);
  ReverseList(G);
}

/**************************************************************/
void CleanGram(TGram G, int *VNTACT, int *VTACT, 
		   int *NoTerAct, int *TerAct) {
  int i;
  Rule2 *nr2;

  /* CHECK IF SOME TERMINAL OR NON TERMINAL MUST DESAPPEAR */

  (*TerAct) = (*NoTerAct) = 0;

  for (i=0; i<G.NoTer; i++)
    if ((G.a[i] != NULL) || (G.b[i] != NULL)){
      (*NoTerAct)++;
      VNTACT[i] = 1;
    }
    else
      VNTACT[i] = 0;

  for (i=0; i<G.Ter; i++) VTACT[i] = 0;
  i = 0;
  while ((i < G.NoTer) && ((*TerAct) < G.Ter)) {
    nr2 = G.b[i];
    while ((nr2 != NULL) && ((*TerAct) < G.Ter)) {
      if (VTACT[nr2->con] == 0) {
	(*TerAct)++;
	VTACT[nr2->con] = 1;
      }
      nr2 = nr2->next;
    }
    i++;
  }
}

/**************************************************************/
void WriteGram(TGram G, char *NameFic, int all,
	       int argc, char *argv[]) {
  FILE *fsal;
  Rule1 *nr1;
  Rule2 *nr2;
  int *VNTACT, *VTACT, TerAct, NoTerAct, i;
  char **VNT,**VT;
  double VALMIN; /* IGNORE ALL RULES WITH PROBABILITY LESS THAN VALMIN */
  float logBase, proba;

  logBase = log(G.Base);

  VALMIN = 1.0e-08;

  if ((fsal = fopen(NameFic,"w")) == NULL) Error(0,NameFic);

  /* DELETE NON TERMINALS WITH NO RULES */
  if ((VNTACT = (int *) malloc(G.NoTer*sizeof(int))) == NULL) Error(1,"");
  if ((VTACT = (int *) malloc(G.Ter*sizeof(int))) == NULL) Error(1,"");
  CleanGram(G, VNTACT, VTACT, &NoTerAct, &TerAct);

  fprintf(fsal,"#");
  for (i=0; i<argc; i++)
    fprintf(fsal," %s",argv[i]);

  WriteHashTab(G.SNT, &VNT);
  fprintf(fsal,"\n\nNonTerminals %d\n",NoTerAct);
  for (i=0; i<G.NoTer; i++)
    if (VNTACT[i] == 1)
      fprintf(fsal,"%s\n",VNT[i]);

  WriteHashTab(G.ST, &VT);
  fprintf(fsal,"\nTerminals %d\n",TerAct);
  for (i=0; i<G.Ter; i++)
    if (VTACT[i] == 1)
      fprintf(fsal,"%s\n",VT[i]);

  fprintf(fsal,"\nRules\n");

  for (i=0; i<G.NoTer; i++) {
    nr1 = G.a[i];
    while (nr1 != NULL) {
      proba = exp(nr1->pro * logBase);
      if (all == true) 
        fprintf(fsal,"%e %s %s %s %s\n",proba,VNT[i],arrow,VNT[nr1->con1],
	VNT[nr1->con2]);
      else if ((nr1->pro != MINLOG) && (proba > VALMIN))
        fprintf(fsal,"%e %s %s %s %s\n",proba,VNT[i],arrow,VNT[nr1->con1],
	VNT[nr1->con2]);
      nr1 = nr1->next;
    }
    nr2 = G.b[i];
    while (nr2 != NULL) {
      proba = exp(nr2->pro * logBase);
      if (all == true)
        fprintf(fsal,"%e %s %s %s\n",proba,VNT[i],arrow,VT[nr2->con]);
      else if ((nr2->pro != MINLOG) && (proba > VALMIN))
        fprintf(fsal,"%e %s %s %s\n",proba,VNT[i],arrow,VT[nr2->con]);
      nr2 = nr2->next;
    }
  }
  fclose(fsal);
}

/**************************************************************/
void Memory(Trellis *e, int NoTer, int TaTrel) {
  int *eee, **ee, i, j;

  if ((eee = (int *) malloc(TaTrel * TaTrel * NoTer * sizeof(int))) == NULL)
    Error(1,"(in memory)");
  if ((ee = (int **) malloc(TaTrel * TaTrel * sizeof(int *))) == NULL)
    Error(1,"(in memory)");
  if (((*e) = (int ***) malloc(TaTrel * sizeof(int **))) == NULL)
    Error(1,"(in memory)");

  for (i=0; i<TaTrel; i++)
    (*e)[i] = &(ee[i * TaTrel]);
  for (i=0; i<TaTrel; i++)
    for (j=0; j<TaTrel; j++)
      ee[j + i * TaTrel] = &(eee[j * NoTer + i * TaTrel * NoTer]);

}

/******************************************************************/
void InnerSpan(Trellis e, TGram G, int *index, int lsam,  TTabLog T, 
	       TTree tree) {
  Rule2 *gre1;
  Rule1 *reg1;
  int n, i, j, k, l;

  for (i=0; i<lsam; i++)
    for (j=0; j<G.NoTer; j++) {
      gre1 = G.b[j];
      while ((gre1 != NULL) && (gre1->con != index[i]))
	gre1 = gre1->next;
      if (gre1 != NULL)
        e[i][i][j] = gre1->pro;
      else
	e[i][i][j] = MINLOG;
    }

  for (n=0; n<tree.lastNode; n=n+tree.l[n]+1)
    for (i=1; i<tree.l[n]; i++)
      for (j=0; j<tree.l[n] - i; j++)
	for (k=0; k<G.NoTer; k++) {
	  e[tree.l[n+1+j]][tree.r[n+1+j+i]][k] = MINLOG;
	  reg1 = G.a[k];
	  while (reg1 != NULL) {
	    for (l=n+1+j; l<n+1+j+i; l++)
	      e[tree.l[n+1+j]][tree.r[n+1+j+i]][k] = 
		LogAdd(e[tree.l[n+1+j]][tree.r[n+1+j+i]][k],
		       LogProduct(reg1->pro,
				  e[tree.l[n+1+j]][tree.r[l]][reg1->con1],
				  e[tree.l[l+1]][tree.r[n+1+j+i]][reg1->con2]),
		       T);
	    reg1 = reg1->next;				    
	  }
	}
}

/******************************************************************/
/* This routine computes a tree from a bracketed string, in which 
   each node represents a subproblem in the dynamic programing 
   schemme. The representation of the tree is as follows:

   ((x_0 x_1) ... (k_q ... x_k)) ... (x_l ... x_p)
   
                       a
                      / \
                     /   \
                    /     \
                  n_1 ... n_i
                 (0,k)   (l,p)
                  /\
                 /  \
                /    \
               m_1...m_j ...
              (0,1) (q,k)

    -------------------------------------
T.l |j|0| ... |q| ...
    --------------------------------------
T.r | |1| ... |k| ...
    -------------------------------------
*/
   
int IndexSpan(char * cade1, TGram G, int * index, TTree * T, int size) {
  
#define MaxStack 200

  int i, j, pos, par, nextNode, auxint;
  char * aux, copycade1[MAXCAD];
  int Stack[MaxStack], CStack;
  int Pairs1[MaxStack], Pairs2[MaxStack], CountPairs;

  for (i=0; i<size; i++)
    index[i] = -1;

  strcpy(copycade1," [");

  aux = (char *) strtok(cade1,delim);
  while (aux != NULL) {
    if ((strcmp(aux,"[") == 0) || (strcmp(aux,"]") == 0)) {
      strcat(copycade1," ");
      strcat(copycade1,aux);
    }
    else {
      strcat(copycade1," [ ");
      strcat(copycade1,aux);
      strcat(copycade1," ]");
    }
    aux = (char *) strtok(NULL,delim);
  }
  strcat(copycade1," ]");

  /* printf("%s\n",copycade1); */

  CStack = 0;
  pos = 0;
  CountPairs = 0;

  aux = (char *) strtok(copycade1,delim);
  while ((aux != NULL) && (pos <= size)) {
    if (strcmp(aux,"[") == 0) {
      Stack[CStack] = pos;
      CStack++;
    }
    else
      if (strcmp(aux,"]") == 0) {
	CStack--;
	par = Stack[CStack];
	if (par == pos-1) {
	  if (CountPairs == 0)  {
	    Pairs1[CountPairs] = Pairs2[CountPairs] = par;
	    CountPairs++;
	  }
	  else {
	    if ((Pairs1[CountPairs-1] != par) &&
		(Pairs2[CountPairs-1] != par)) {
	      Pairs1[CountPairs] = Pairs2[CountPairs] = par;
	      CountPairs++;
	    }
	  }
	}
	else {
	  nextNode = 1;

	  CountPairs--;
	  (*T).l[(*T).lastNode] = 0;
	  while ((CountPairs >= 0) && (Pairs1[CountPairs] >= par)) {
	    (*T).l[(*T).lastNode]++;
	    if ((*T).lastNode + nextNode == (*T).maxNodes) {
	      fprintf(stderr,"Error: tree size overflow.\n");
	      exit(-1);
	    }
	    (*T).l[(*T).lastNode + nextNode] = Pairs1[CountPairs];
	    (*T).r[(*T).lastNode + nextNode] = Pairs2[CountPairs];
	    nextNode++;
	    CountPairs--;
	  }
	  (*T).lastNode = (*T).lastNode + nextNode;

	  
	  CountPairs++;

	  if (CountPairs == 0)  {
	    Pairs1[CountPairs] = par;
	    Pairs2[CountPairs] = pos-1;
	    CountPairs++;
	  }
	  else {
	    if ((Pairs1[CountPairs-1] != par) &&
		(Pairs2[CountPairs-1] != pos-1)) {
	      Pairs1[CountPairs] = par;
	      Pairs2[CountPairs] = pos-1;
	      CountPairs++;
	    }
	  }
	}
      }
      else {
	if ((j = isInHashTab(aux,G.ST)) < 0) ErrorHashTab(6,aux);
	index[pos] = j;
	pos++;
      }
    aux = (char *) strtok(NULL,delim);
  }

  if (CStack != 0) {
    fprintf(stderr,"Error: stack not empty.\n");
    return(0);
  }
  if (pos > size) {
    Warning(4,"");
    return(0);
  }

  /* The list of sons of each node is inverted. */
  for (i=0; i<(*T).lastNode; i=i+(*T).l[i]+1)
    for (j=1; j<=(*T).l[i]/2; j++) {
      auxint = (*T).l[i+j];
      (*T).l[i+j] = (*T).l[i+(*T).l[i]-j+1];
      (*T).l[i+(*T).l[i]-j+1] = auxint;

      auxint = (*T).r[i+j];
      (*T).r[i+j] = (*T).r[i+(*T).l[i]-j+1];
      (*T).r[i+(*T).l[i]-j+1] = auxint;
    }    

  /*  for (i=0; i<(*T).lastNode; i++)
      printf("%3d %3d\n",(*T).l[i],(*T).r[i]);
      fflush(stdout); 
      exit(-1); */
  
  return(pos);
}

/**************************************************************/
Rule2 *Search_b(int ant, int con, Rules2 b) {
  Rule2 *gre;

  for (gre=b[ant]; (gre!=NULL) && (gre->con!=con); gre=gre->next);
  return(gre);
}

/**************************************************************/
Rule1 *Search_a(int ant, int con1, int con2, Rules1 a) {
  Rule1 *reg;

  reg = a[ant];
  while ((reg != NULL) && !((reg->con1 == con1)&&(reg->con2 == con2)))
  reg = reg->next;
  return(reg);
}

/*****************************************************************************/
void ViterbiSpan (TGram G, int lsam, int *index, Trellis Q, Trellis F1, 
		  Trellis F2, Trellis S, TTree tree) {
  int i, j, k, l, X, n;
  Rule1 *reg;
  Rule2 *gre;

  for (i=0; i<lsam; i++)
    for (j=0; j<G.NoTer; j++) {
      gre = G.b[j];
      while ((gre != NULL) && (gre->con != index[i]))
	gre = gre->next;
      if (gre != NULL) {
        Q[i][i][j] = gre->pro;
	F1[i][i][j] = j;
        S[i][i][j] = 0;
      }
      else {
	Q[i][i][j] = MINLOG;
	F1[i][i][j] = F2[i][i][j] = S[i][i][j] = -1;

      }
    }

  for (n=0; n<tree.lastNode; n=n+tree.l[n]+1) {
    for (i=1; i<tree.l[n]; i++)
      for (j=0; j<tree.l[n] - i; j++)
	for (k=0; k<G.NoTer; k++) {
	  Q[tree.l[n+1+j]][tree.r[n+1+j+i]][k] = MINLOG;
	  reg = G.a[k];
	  while (reg != NULL) {
	    for (l=n+1+j; l<n+1+j+i; l++) { 
	      X = LogProduct(reg->pro,
			     Q[tree.l[n+1+j]][tree.r[l]][reg->con1],
			     Q[tree.l[l+1]][tree.r[n+1+j+i]][reg->con2]);
	      if (X > Q[tree.l[n+1+j]][tree.r[n+1+j+i]][k]) {
		Q[tree.l[n+1+j]][tree.r[n+1+j+i]][k] = X;
		F1[tree.l[n+1+j]][tree.r[n+1+j+i]][k] = reg->con1;
		F2[tree.l[n+1+j]][tree.r[n+1+j+i]][k] = reg->con2;
		S[tree.l[n+1+j]][tree.r[n+1+j+i]][k] = tree.r[l];	
	      }
	    }
	    reg = reg->next;
	  }
	}
  }
}

/**************************************************************/
void Smooth(TGram G, int smooth, TTabLog T) {
  int i, Cont, Sumatot, ContxSua;
  Rule1 *reg1;
  Rule2 *gre1;
  float LogBase;

  LogBase = log(G.Base);
  
  for (i=0; i<G.NoTer; i++) {
    Cont = 0;
    Sumatot = MINLOG;
    reg1 = G.a[i];
    while (reg1 != NULL) {
      if (reg1->pro <= smooth) { reg1->pro = smooth; Cont++;}
      else Sumatot = LogAdd(Sumatot, reg1->pro, T);
      reg1 = reg1->next;
    }

    gre1 = G.b[i];
    while (gre1 != NULL) {
      if (gre1->pro <= smooth) { gre1->pro = smooth; Cont++;}
      else Sumatot = LogAdd(Sumatot, gre1->pro, T);
      gre1 = gre1->next;
    }

    if (Sumatot == MINLOG) Sumatot = 0;
    ContxSua = (int) (log(1.0 - exp((double)smooth*LogBase)*Cont)/LogBase);

    reg1 = G.a[i];
    while (reg1 != NULL) {
      if (reg1->pro > smooth)
        reg1->pro = LogProduct(reg1->pro, ContxSua, -Sumatot);
      reg1 = reg1->next;
    }
    gre1 = G.b[i];
    while (gre1 != NULL) {
      if (gre1->pro > smooth)
        gre1->pro = LogProduct(gre1->pro, ContxSua, -Sumatot);
      gre1 = gre1->next;
    }
  }
}

/**************************************************************/
void MemNullRules (Rules1 *a, Rules2 *b, int NoTer) {
  int i;

  if (((*a) = (Rules1) malloc(NoTer * sizeof(Rule1 *))) == NULL) 
    Error(1,"");
  for (i=0;i<NoTer;i++)
    (*a)[i] = NULL;

  if (((*b) = (Rules2) malloc(NoTer * sizeof(Rule2 *))) == NULL) 
    Error(1,"");
  for (i=0;i<NoTer;i++)
    (*b)[i] = NULL;
}

/**************************************************************/
void DeleteNullRules (TGram *G, Rules1 a_nul, Rules2 b_nul) {
  int i;
  Rule1 *reg1, *reg2, *reg3;
  Rule2 *gre1, *gre2, *gre3;

  for (i=0; i<(*G).NoTer; i++) {
    reg1 = (*G).a[i];
    reg2 = NULL;
    while (reg1 != NULL) 
      if (reg1->pro <= MINLOG)
        if (reg2 == NULL) {
          reg3 = reg1; (*G).a[i] = reg1->next; reg1 = reg1->next;
          reg3->next = a_nul[i]; a_nul[i] = reg3;
        } else {
          reg3 = reg1; reg2->next = reg1->next; reg1 = reg1->next;
          reg3->next = a_nul[i]; a_nul[i] = reg3;
        }
      else {
        reg2 = reg1;
        reg1 = reg1->next;
      } 
    gre1 = (*G).b[i];
    gre2 = NULL;
    while (gre1 != NULL) 
      if (gre1->pro <= MINLOG)
        if (gre2 == NULL) {
          gre3 = gre1; (*G).b[i] = gre1->next; gre1 = gre1->next;
          gre3->next = b_nul[i]; b_nul[i] = gre3;
        } else {
          gre3 = gre1; gre2->next = gre1->next; gre1 = gre1->next;
          gre3->next = b_nul[i]; b_nul[i] = gre3;
        }
      else {
        gre2 = gre1;
        gre1 = gre1->next;
      }
  }

}

/**************************************************************/
void InsertNullRules (TGram *G, Rules1 a_nul, Rules2 b_nul) {
  int i;
  Rule1 *reg, *reg1;
  Rule2 *gre, *gre1;

  for (i=0; i<(*G).NoTer; i++) {
    reg = (*G).a[i];
    reg1 = NULL;
    
    while (reg != NULL) { reg1 = reg; reg = reg->next; }

    if (reg1 != NULL) { reg1->next = a_nul[i]; a_nul[i] = NULL; }
    else { (*G).a[i] = a_nul[i]; a_nul[i] = NULL; }

    gre = (*G).b[i];
    gre1 = NULL;
    
    while (gre != NULL) { gre1 = gre; gre = gre->next; }

    if (gre1 != NULL) { gre1->next = b_nul[i]; b_nul[i] = NULL; }
    else { (*G).b[i] = b_nul[i]; b_nul[i] = NULL; }
  }
}










