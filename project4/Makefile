CC=g++
AR=ar
RANLIB=ranlib

SRCDIR=src/
INC=include/
LIBS=lib/

OBJFLAGS+= -c -I $(INC) 

LIB_FULL_NAME= $(LIBS)libbpt.a
TARGET_SRC:=$(SRCDIR)DiskManager.c $(SRCDIR)FileIndexManager.c $(SRCDIR)dbbpt.c $(SRCDIR)dbinit.c $(SRCDIR)BufferManager.c $(SRCDIR)Optimizer.c $(SRCDIR)join.c
TARGET_OBJ_FOR_LIB:=$(TARGET_SRC:%.c=%.o) 

$(LIB_FULL_NAME) : $(TARGET_OBJ_FOR_LIB)
	$(AR) rcv $@ $(TARGET_OBJ_FOR_LIB)
	$(RANLIB) $@

$(TARGET_OBJ_FOR_LIB) : $(TARGET_SRC)
	$(CC) $(OBJFLAGS) $(SRCDIR)DiskManager.c -o $(SRCDIR)DiskManager.o
	$(CC) $(OBJFLAGS) $(SRCDIR)FileIndexManager.c -o $(SRCDIR)FileIndexManager.o
	$(CC) $(OBJFLAGS) $(SRCDIR)BufferManager.c -o $(SRCDIR)BufferManager.o
	$(CC) $(OBJFLAGS) $(SRCDIR)dbbpt.c -o $(SRCDIR)dbbpt.o
	$(CC) $(OBJFLAGS) $(SRCDIR)dbinit.c -o $(SRCDIR)dbinit.o
	$(CC) $(OBJFLAGS) $(SRCDIR)Optimizer.c -o $(SRCDIR)Optimizer.o -lpthread
	$(CC) $(OBJFLAGS) $(SRCDIR)join.c -o $(SRCDIR)join.o -lpthread

clean:
	rm $(LIBS)libbpt.a $(TARGET_OBJ_FOR_LIB)
