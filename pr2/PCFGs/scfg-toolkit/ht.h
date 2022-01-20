typedef struct snode {
  char *str;
  int val;
  struct snode *next;
} Tnode;

typedef struct {
  int size;
  Tnode **v;
}  THashTable;

/**************************************************************/
extern int CreateHashTab(THashTable *T);
extern int isInHashTab(char *str, THashTable T);
extern void InsertInHashTab(char *str, int ocu, THashTable T);
extern int DeleteFromHashTab(char *str, THashTable T);
extern void ErrorHashTab(int nerror, char *msg);
extern void WriteHashTab(THashTable T, char ***VectStr);
