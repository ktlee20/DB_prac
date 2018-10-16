CC=gcc
AR=ar

SRCDIR=src/
INC=include/
LIBS=lib/


TARGET_SRC:=$(SRCDIR)stress.c $(SRCDIR)DiskMagager.c $(SRCDIR)FileIndexManager.c $(SRCDIR)dbbpt.c $(SRCDIR)dbinit.c 
TARGET_OBJ:=$(SRCDIR)stress.o $(SRCDIR)DiskManager.o $(SRCDIR)FileIndexManager.o $(SRCDIR)dbbpt.o $(SRCDIR)dbinit.o

CFLAGS+= -I $(INC)
OBJFLAGS+= -c -I $(INC)
DFLAGS += -fPIC -c -I $(INC)

TARGET:=stress

all: $(TARGET)

$(TARGET) : $(TARGET_OBJ)
	$(CC) $(OBJFLAGS) $(SRCDIR)DiskManager.c -o $(SRCDIR)DiskManager.o 
	$(CC) $(OBJFLAGS) $(SRCDIR)FileIndexManager.c -o $(SRCDIR)FileIndexManager.o
	$(CC) $(OBJFLAGS) $(SRCDIR)dbbpt.c -o $(SRCDIR)dbbpt.o 
	$(CC) $(OBJFLAGS) $(SRCDIR)dbinit.c -o $(SRCDIR)dbinit.o 
	make static_library
	$(CC) $(CFLAGS) $(TARGET_OBJ) -o $(TARGET)

static_library:
	ar cr $(LIBS)libbpt.a $(TARGET_OBJ)

clean:
	rm $(TARGET) $(TARGET_OBJ)
	rm $(LIBS)*

cleanall:
	make clean
	rm default.db
