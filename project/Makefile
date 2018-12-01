CC=gcc
AR=ar

SRCDIR=src/
INC=include/
LIBS=lib/


TARGET_SRC:=$(SRCDIR)stress.c $(SRCDIR)DiskManager.c $(SRCDIR)FileIndexManager.c $(SRCDIR)dbbpt.c $(SRCDIR)dbinit.c $(SRCDIR)BufferManager.c $(SRCDIR)Optimizer.c $(SRCDIR)join.c
TARGET_OBJ:=$(SRCDIR)stress.o $(SRCDIR)DiskManager.o $(SRCDIR)FileIndexManager.o $(SRCDIR)dbbpt.o $(SRCDIR)dbinit.o $(SRCDIR)BufferManager.o $(SRCDIR)Optimizer.o $(SRCDIR)join.o
TARGET_OBJ_FOR_LIB:=$(TARGET_SRC:.c=.o)

CFLAGS+= -fPIC -I $(INC)
OBJFLAGS+= -fPIC -c -I $(INC)

#CFLAGS+= -g -static -fPIC -I $(INC)
#OBJFLAGS+= -g -static -fPIC -c -I $(INC)

TARGET:=stress

all: $(TARGET)

$(TARGET) : $(TARGET_OBJ)
	$(CC) $(OBJFLAGS) $(SRCDIR)DiskManager.c -o $(SRCDIR)DiskManager.o 
	$(CC) $(OBJFLAGS) $(SRCDIR)FileIndexManager.c -o $(SRCDIR)FileIndexManager.o
	$(CC) $(OBJFLAGS) $(SRCDIR)BufferManager.c -o $(SRCDIR)BufferManager.o
	$(CC) $(OBJFLAGS) $(SRCDIR)dbbpt.c -o $(SRCDIR)dbbpt.o 
	$(CC) $(OBJFLAGS) $(SRCDIR)dbinit.c -o $(SRCDIR)dbinit.o 
	$(CC) $(OBJFLAGS) $(SRCDIR)Optimizer.c -o $(SRCDIR)Optimizer.o
	$(CC) $(OBJFLAGS) $(SRCDIR)join.c -o $(SRCDIR)join.o -pthread
	make static_library
	make library
	$(CC) $(CFLAGS) $(TARGET_OBJ) -o $(TARGET) -pthread

static_library:
	ar rcv $(LIBS)libbpt.a $(TARGET_OBJ)
	ranlib $(LIBS)libbpt.a

library:
	gcc -shared -Wl,-soname,libbpt.so -o $(LIBS)libbpt.so $(TARGET_OBJ_FOR_LIB)

clean:
	rm $(TARGET) $(TARGET_OBJ)
	rm $(LIBS)*

cleanall:
	make clean
	rm default*.db
