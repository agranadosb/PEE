#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <getopt.h>

/*****************************************************************************/
/******************************************************************************/

void Syntax(char *command) {
  fprintf(stdout,"\nUsage: %s -F n [-s seed]",command);
  fprintf(stdout," [-c num] [-l s] [-L S]\n\n");
  fprintf(stdout,"-F n:\tgeometric figure (0 by default)\n");
  fprintf(stdout,"\t\tn=0 right triangle [s=2 S=15]\n");
  fprintf(stdout,"\t\tn=1 equilateral triangle [s=2 S=15]\n");
  fprintf(stdout,"\t\tn=2 isosceles triangle [s=2 S=15]\n");
  fprintf(stdout,"-s see:\tseed for the generator (1 by default).\n");
  fprintf(stdout,"-c num:\tnumber of samples to be generated (1 by default).\n");
  fprintf(stdout,"-l s:\tminimum size of the side\n");
  fprintf(stdout,"-L S:\tmaximum size of the side\n\n");
  exit(0);
}

/******************************************************************************/

int main(int argc,char *argv[]) {   
  extern char *optarg;
  extern int opterr, optind;
  int seed, option, lonMin, lonMax, nsamples, figure;

  seed = 1;
  nsamples = 1;
  lonMin = 0;
  lonMax = 0;
  figure = 0;
  
  if (argc == 1) Syntax(argv[0]);
  while ((option=getopt(argc,argv,"hF:s:c:l:L:")) != EOF ) {
    switch(option) {
    case 'F': figure=atoi(optarg);break;
    case 's': seed = atoi(optarg);break;
    case 'c': nsamples = atoi(optarg);break;
    case 'l': lonMin = atoi(optarg);break;
    case 'L': lonMax = atoi(optarg);break;
    case 'h': Syntax(argv[0]);
    default: Syntax(argv[0]);
    }
  }

  srand(seed);

  if (figure == 0) {/* right triangle */
    if (lonMin <= 2) lonMin=2;
    if (lonMax >= 15) lonMax=15;
    while (nsamples > 0) {
      int c1;
      do
	c1 = lonMin + (lonMax-lonMin)*(double)rand()/RAND_MAX;
      while (c1 == 0);
      int c2;
      do
	c2 = lonMin + (lonMax-lonMin)*(double)rand()/RAND_MAX;
      while (c2 == 0);
      int h = sqrt(c1*c1+c2*c2) + 0.5;
      fprintf(stdout,"[ [");
      while (c1>0) {fprintf(stdout," b"); c1--;}
      fprintf(stdout," ] [");
      while (c2>0) {fprintf(stdout," d"); c2--;}
      fprintf(stdout," ] ] [");
      while (h>0) {fprintf(stdout," g"); h--;}
      fprintf(stdout," ]\n");
      nsamples--;
    }
  }
  else if (figure == 1) {
    if (lonMin <= 2) lonMin=2;
    if (lonMax >= 15) lonMax=15;
    while (nsamples > 0) {
      int c1;
      do
	c1 = lonMin + (lonMax-lonMin)*(double)rand()/RAND_MAX;
      while (c1 == 0);
      int c2 = c1, c3 = c1;
      fprintf(stdout,"[");
      while (c1>0) {fprintf(stdout," b"); c1--;}
      fprintf(stdout," ] [");
      while (c2>0) {fprintf(stdout," d"); c2--;}
      fprintf(stdout," ] [");
      while (c3>0) {fprintf(stdout," g"); c3--;}
      fprintf(stdout," ]\n");
      nsamples--;
    }    
  }
  else if (figure == 2) {
    if (lonMin <= 2) lonMin=2;
    if (lonMax >= 15) lonMax=15;
    while (nsamples > 0) {
      int c1;
      do
	c1 = lonMin + (lonMax-lonMin)*(double)rand()/RAND_MAX;
      while (c1 == 0);
      int c3 = c1, c2;
      do
	c2 = lonMin + (lonMax-lonMin)*(double)rand()/RAND_MAX;
      while ((c2 == 0) || (c2 >= 2*c1));
      fprintf(stdout,"[");
      while (c1>0) {fprintf(stdout," b"); c1--;}
      fprintf(stdout," ] [");
      while (c2>0) {fprintf(stdout," d"); c2--;}
      fprintf(stdout," ] [");
      while (c3>0) {fprintf(stdout," g"); c3--;}
      fprintf(stdout," ]\n");
      nsamples--;
    }    
  }
  
  return(0);
}
