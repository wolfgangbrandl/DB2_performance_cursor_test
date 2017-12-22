CC              =       /usr/bin/gcc
CC2             =       /usr/bin/gcc
DATASOURCE      =       SAMPLE
QUALIFIER       =       TAEDB
USER		=	db2it01
PW		=	XXXXXXXXXXXXXXXXXXXXXX
DB2INSTANCEPATH =       /home/$(DB2INSTANCE)
CFLAGS          =       -g -w  -fvisibility=hidden -Wunused -Wall
LIBDIR          =       -L$(DB2LIB) -ldb2 -L.
INCLUDE         =       -I$(DB2_HOME)/include -I./
SHELL           =       /bin/bash
REPLVER         :=      $(shell date +%y%m%d%H%M%S)


.SUFFIXES:  .c .sqc
.sqc.c:
	db2 connect to $(DATASOURCE) 
	echo VERSION $(REPLVER)
	db2 prep $> bindfile QUALIFIER $(QUALIFIER) VERSION $(REPLVER) NOLINEMACRO
	cp $*.bnd $*.bnd.tmp
	db2 bind  $*.bnd QUALIFIER $(QUALIFIER)
	cp $@ $@.tmp
	db2 terminate


all: addstmt libdb2tr.so perf2curtest 


db2trace.o:  db2trace.c
	$(CC) -g -c db2trace.c $(INCLUDE) -fPIC
libdb2tr.so:       db2trace.o
	$(CC) -o $@ -shared -lc -lm -ldl -L. -L/lib -L/usr/lib -L$(DB2LIB) -ldb2 -fPIC $>

perf2curtest.c:       perf2curtest.sqc
perf2curtest.o:	perf2curtest.c
	$(CC) -g -c perf2curtest.c -fPIC $(INCLUDE)
perf2curtest: perf2curtest.o libdb2tr.so
	$(CC) $(CFLAGS) $(INCLUDE) $(LIBDIR) -o $@ $>

addstmt.o:	addstmt.c
	$(CC) -g -c $> -fPIC $(INCLUDE)
addstmt: addstmt.o
	$(CC) $(CFLAGS) -o $@ $>

bindblockingyes:
	db2 connect to $(DATASOURCE)
	db2 bind  perf1curtest.bnd BLOCKING UNAMBIG QUALIFIER $(QUALIFIER)
	db2 bind  perf2curtest.bnd BLOCKING UNAMBIG QUALIFIER $(QUALIFIER)
	db2 bind  perf3curtest.bnd BLOCKING UNAMBIG QUALIFIER $(QUALIFIER)
	db2 terminate
bindblockingno:
	db2 connect to $(DATASOURCE)
	db2 bind  perf1curtest.bnd BLOCKING NO QUALIFIER $(QUALIFIER)
	db2 bind  perf2curtest.bnd BLOCKING NO QUALIFIER $(QUALIFIER)
	db2 bind  perf3curtest.bnd BLOCKING NO QUALIFIER $(QUALIFIER)
	db2 terminate
packexe:
	tar cvf perfcurtest.tar ./libdb2tr.so ./perf1curtest ./perf1curtest.bnd ./perf1curtest.c ./perf2curtest ./perf2curtest.bnd ./perf2curtest.c addstmt perf3curtest.c perf3curtest ./perf3curtest.bnd
pack:
	tar cvf perfcurtest.tar ./libdb2tr.so ./perf1curtest ./perf1curtest.bnd ./makefile ./perf1curtest.c addstmt ./perf2curtest ./perf2curtest.bnd ./perf2curtest.c perf3curtest.c perf3curtest ./perf3curtest.bnd
packsource:
	tar cvf perfcurtestsource.tar ./*.sqc ./*.c ./makefile
qualtest1:
	time ./perf1curtest -d$(DATASOURCE) -u$(USER) -p$(PW) -q$(QUALIFIER) | tee out
qualtest2:
	time ./perf2curtest -d$(DATASOURCE) -q$(QUALIFIER) | tee out
qualtest3:
	time ./perf3curtest -d$(DATASOURCE) -u$(USER) -p$(PW) -q$(QUALIFIER) | tee out
addtest:
	./addstmt -iout -sperf2curtest.c -oout.stmt4
history:
	mkdir -p version
	cp perf2curtest.sqc ./version/perf2curtest.sqc.`date +%y%m%d%H%M%S`
	cp perf1curtest.sqc ./version/perf1curtest.sqc.`date +%y%m%d%H%M%S`
	cp db2trace.c ./version/db2trace.c.`date +%y%m%d%H%M%S`
	cp addstmt.c ./version/addstmt.c.`date +%y%m%d%H%M%S`
	cp makefile ./version/makefile.`date +%y%m%d%H%M%S`


clean:
	rm -f *.c.c *.i *.o *.bnd *.tmp perf1curtest perf1curtest.c *.trc *.so ./lib/*.so db2trace.o *.tar addstmt perf2curtest perf2curtest.c perf3curtest perf3curtest.c

