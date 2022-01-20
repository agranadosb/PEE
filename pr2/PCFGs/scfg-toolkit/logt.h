#define MINLOG -230000000

typedef struct {
  int Size;
  double Base, logBase;
  int *v;
} TTabLog;
  
/**************************************************************/
extern void CreateTabLog(double Base, TTabLog *T);
extern int LogAdd(int s1, int s2, TTabLog T);
extern int LogProduct(register int c1, register int c2, register int c3);

