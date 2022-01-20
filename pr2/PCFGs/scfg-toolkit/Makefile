
CFLAGS= -ansi -Wall -pedantic -O3

OBJECTS= error.o ht.o utils.o cfg.o logt.o

all: scfg_cgr scfg_gstr scfg_learn scfg_prob

clean:
	rm -if *.o
	rm -if *~*

scfg_cgr: $(OBJECTS) scfg_cgr.c 
	$(CC) scfg_cgr.c $(OBJECTS) -o scfg_cgr $(CFLAGS) -lm

scfg_gstr: $(OBJECTS) scfg_gstr.c 
	$(CC) scfg_gstr.c $(OBJECTS) -o scfg_gstr $(CFLAGS) -lm

scfg_learn: $(OBJECTS) scfg_learn.c 
	$(CC) scfg_learn.c $(OBJECTS) -o scfg_learn $(CFLAGS) -lm

scfg_prob: $(OBJECTS) scfg_prob.c 
	$(CC) scfg_prob.c $(OBJECTS) -o scfg_prob $(CFLAGS) -lm



cfg.o: cfg.c cfg.h utils.h error.h ht.h
	$(CC) -c cfg.c  $(CFLAGS) 

utils.o: utils.c utils.h error.h ht.h
	$(CC) -c utils.c $(CFLAGS) 

error.o: error.c error.h
	$(CC) -c error.c $(CFLAGS) 

ht.o: ht.c ht.h
	$(CC) -c ht.c $(CFLAGS) 

logt.o: logt.c logt.h
	$(CC) -c logt.c $(CFLAGS) 

