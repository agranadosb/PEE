#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include <stdlib.h>

#include "logt.h"

/**************************************************************/
void CreateTabLog(double Base, TTabLog *T) {
  int i;
  double BaseDouble;

  (*T).Base = Base;
  (*T).logBase = log(Base);
  (*T).Size = (int) ceil(-log(sqrt(Base)-1.0)/log(Base));
  if (((*T).v = (int *)malloc(((*T).Size+1)*sizeof(int))) == NULL) {
    fprintf(stderr, "Error: not enougth memory creating tablog.\n");
    exit(-1);
  }

  BaseDouble = 1.0;
  for (i=0; i<=(*T).Size; i++) {
    (*T).v[i] = (int) (log(1.0+1.0/BaseDouble)/(*T).logBase+0.5);
    BaseDouble = BaseDouble * Base;
  }
}

/**************************************************************/
int LogAdd(int sum1, int sum2, TTabLog T) {
  int dif;
  
  if ((sum1 <= MINLOG) && (sum2 <= MINLOG))
    return(MINLOG);
  else {
    if (sum1 < sum2) {
      dif = sum1 - sum2;
      if (dif < (-T.Size)) return(sum2);
      else return(sum2 + T.v[-dif]);
    }
    else {
      dif = sum2 - sum1;
      if (dif < (-T.Size)) return(sum1);
      else return(sum1 + T.v[-dif]);
    }
  }
} 

/**************************************************************/
int LogProduct(register int c1, register int c2, register int c3) {
  register int product;

  if ((c1 <= MINLOG) || (c2 <= MINLOG) ||(c3 <= MINLOG))
    return(MINLOG);
  product = c1+c2+c3;
  return(product<MINLOG?MINLOG:product);
}





