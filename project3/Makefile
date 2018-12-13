CC=gcc
AR=ar

SRCDIR=src/
INC=include/
LIBS=lib/


TARGET_SRC:=$(SRCDIR)stress.cpp $(SRCDIR)DiskManager.cpp $(SRCDIR)FileIndexManager.cpp $(SRCDIR)dbbpt.cpp $(SRCDIR)dbinit.cpp $(SRCDIR)BufferManager.cpp $(SRCDIR)Optimizer.cpp $(SRCDIR)join.cpp $(SRCDIC)LockManager.cpp
TARGET_OBJ_FOR_LIB:=$(SRCDIR)DiskManager.o $(SRCDIR)FileIndexManager.o $(SRCDIR)dbbpt.o $(SRCDIR)dbinit.o $(SRCDIR)BufferManager.o $(SRCDIR)Optimizer.o $(SRCDIR)join.o $(SRCDIC)LockManager.o
TARGET_OBJ:=$(TARGET_SRC:.c=.o)

CFLAGS+= -fPIC -I $(INC)
OBJFLAGS+= -fPIC -c -I $(INC)

TARGET:=stress

all: $(TARGET)

$(TARGET) : $(TARGET_OBJ)
	$(CC) $(OBJFLAGS) $(SRCDIR)DiskManager.cpp -o $(SRCDIR)DiskManager.o 
	$(CC) $(OBJFLAGS) $(SRCDIR)FileIndexManager.cpp -o $(SRCDIR)FileIndexManager.o
	$(CC) $(OBJFLAGS) $(SRCDIR)BufferManager.cpp -o $(SRCDIR)BufferManager.o
	$(CC) $(OBJFLAGS) $(SRCDIR)dbbpt.cpp -o $(SRCDIR)dbbpt.o 
	$(CC) $(OBJFLAGS) $(SRCDIR)dbinit.cpp -o $(SRCDIR)dbinit.o 
	$(CC) $(OBJFLAGS) $(SRCDIR)Optimizer.cpp -o $(SRCDIR)Optimizer.o 
	$(CC) $(OBJFLAGS) $(SRCDIR)join.cpp -o $(SRCDIR)join.o 
	$(CC) $(OBJFLAGS) $(SRCDIR)LockManager.cpp -o $(SRCDIR)LockManager.o 
	$(CC) $(OBJFLAGS) $(SRCDIR)stress.cpp -o $(SRCDIR)stress.o 
	make static_library
	make library
	$(CC) $(CFLAGS) $(TARGET_OBJ) -o $(TARGET)

static_library:
	ar cr $(LIBS)libbpt.a $(TARGET_OBJ_FOR_LIB)
	ranlib $(LIBS)libbpt.a

library:
	gcc -shared -Wl,-soname,libbpt.so -o $(LIBS)libbpt.so $(TARGET_OBJ_FOR_LIB)

clean:
	rm $(TARGET) $(TARGET_OBJ)
	rm $(LIBS)*

cleanall:
	make clean
	rm default*.db
