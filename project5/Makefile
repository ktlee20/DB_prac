CC=g++
AR=ar
RANLIB=ranlib

SRCDIR=src/
INC=include/
LIBS=lib/

OBJFLAGS+= -std=c++11 -g -static -c -I $(INC) 

LIB_FULL_NAME= $(LIBS)libbpt.a
TARGET_SRC:=$(SRCDIR)DiskManager.cpp $(SRCDIR)FileIndexManager.cpp $(SRCDIR)dbbpt.cpp $(SRCDIR)dbinit.cpp $(SRCDIR)BufferManager.cpp $(SRCDIR)Optimizer.cpp $(SRCDIR)join.cpp $(SRCDIR)LockManager.cpp
TARGET_OBJ_FOR_LIB:=$(TARGET_SRC:.cpp=.o)

$(LIB_FULL_NAME) : $(TARGET_OBJ_FOR_LIB)
	$(AR) rcv $@ $(TARGET_OBJ_FOR_LIB)
	$(RANLIB) $@

$(TARGET_OBJ_FOR_LIB) : $(TARGET_SRC)
	$(CC) $(OBJFLAGS) $(SRCDIR)DiskManager.cpp -o $(SRCDIR)DiskManager.o
	$(CC) $(OBJFLAGS) $(SRCDIR)FileIndexManager.cpp -o $(SRCDIR)FileIndexManager.o
	$(CC) $(OBJFLAGS) $(SRCDIR)BufferManager.cpp -o $(SRCDIR)BufferManager.o
	$(CC) $(OBJFLAGS) $(SRCDIR)dbbpt.cpp -o $(SRCDIR)dbbpt.o
	$(CC) $(OBJFLAGS) $(SRCDIR)dbinit.cpp -o $(SRCDIR)dbinit.o
	$(CC) $(OBJFLAGS) $(SRCDIR)Optimizer.cpp -lpthread -o $(SRCDIR)Optimizer.o
	$(CC) $(OBJFLAGS) $(SRCDIR)join.cpp -lpthread -o $(SRCDIR)join.o
	$(CC) $(OBJFLAGS) $(SRCDIR)LockManager.cpp -o $(SRCDIR)LockManager.o

clean:
	rm $(LIBS)libbpt.a $(TARGET_OBJ_FOR_LIB)
