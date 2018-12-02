CC=g++
AR=ar
RANLIB=ranlib

SRCDIR=src/
INC=include/
LIBS=lib/

OBJFLAGS+= -c -I $(INC) 

LIB_FULL_NAME= $(LIBS)libbpt.a
TARGET_SRC:=$(SRCDIR)DiskManager.c $(SRCDIR)FileIndexManager.c $(SRCDIR)dbbpt.c $(SRCDIR)dbinit.c $(SRCDIR)BufferManager.c $(SRCDIR)Optimizer.c $(SRCDIR)join.c
TARGET_OBJ_FOR_LIB:=DiskManager.o FileIndexManager.o dbbpt.o dbinit.o BufferManager.o Optimizer.o join.o

$(LIB_FULL_NAME) : $(TARGET_OBJ_FOR_LIB)
	$(AR) rcv $@ $(TARGET_OBJ_FOR_LIB)
	$(RANLIB) $@
	rm *.o

$(TARGET_OBJ_FOR_LIB) : $(TARGET_SRC)
	$(CC) $(OBJFLAGS) $(SRCDIR)DiskManager.c
	$(CC) $(OBJFLAGS) $(SRCDIR)FileIndexManager.c
	$(CC) $(OBJFLAGS) $(SRCDIR)BufferManager.c
	$(CC) $(OBJFLAGS) $(SRCDIR)dbbpt.c 
	$(CC) $(OBJFLAGS) $(SRCDIR)dbinit.c
	$(CC) $(OBJFLAGS) $(SRCDIR)Optimizer.c -lpthread
	$(CC) $(OBJFLAGS) $(SRCDIR)join.c -lpthread

clean:
	rm $(LIBS)libbpt.a $(TARGET_OBJ_FOR_LIB)
