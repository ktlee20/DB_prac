CC=gcc

SRCDIR=src/
INC=include/
LIBS=lib/

TARGET_SRC:=$(SRCDIR)main.c $(SRCDIR)DiskMagager.c $(SRCDIR)FileIndexManager.c $(SRCDIR)dbbpt.c $(SRCDIR)dbinit.c 
TARGET_OBJ:=$(SRCDIR)main.o $(SRCDIR)DiskManager.o $(SRCDIR)FileIndexManager.o $(SRCDIR)dbbpt.o $(SRCDIR)dbinit.o

CFLAGS+= -g -I $(INC)
OBJFLAGS+= -c -I $(INC)

TARGET:=main

all: $(TARGET)

$(TARGET) : $(TARGET_OBJ)
	$(CC) $(OBJFLAGS) $(SRCDIR)DiskManager.c -o $(SRCDIR)DiskManager.o 
	$(CC) $(OBJFLAGS) $(SRCDIR)FileIndexManager.c -o $(SRCDIR)FileIndexManager.o
	$(CC) $(OBJFLAGS) $(SRCDIR)dbbpt.c -o $(SRCDIR)dbbpt.o 
	$(CC) $(OBJFLAGS) $(SRCDIR)dbinit.c -o $(SRCDIR)dbinit.o 
	$(CC) $(CFLAGS) $(TARGET_OBJ) -o $(TARGET)

clean:
	rm $(TARGET) $(TARGET_OBJ)

cleanall:
	make clean
	rm default.db
