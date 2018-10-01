CC=gcc

SRCDIR=src/
INC=include/
LIBS=lib/

TARGET_SRC:=$(SRCDIR)main.c $(SRCDIR)page.c $(SRCDIR)startend.c  $(SRCDIR)dbbpt.c 
TARGET_OBJ:=$(SRCDIR)main.o $(SRCDIR)page.o $(SRCDIR)startend.o $(SRCDIR)dbbpt.o 

CFLAGS+= -g -I $(INC)
OBJFLAGS+= -c -I $(INC)

TARGET:=main

all: $(TARGET)

$(TARGET) : $(TARGET_OBJ)
	$(CC) $(OBJFLAGS) $(SRCDIR)page.c -o $(SRCDIR)page.o 
	$(CC) $(OBJFLAGS) $(SRCDIR)startend.c -o $(SRCDIR)startend.o
	$(CC) $(OBJFLAGS) $(SRCDIR)dbbpt.c -o $(SRCDIR)dbbpt.o 
	$(CC) $(CFLAGS) $(TARGET_OBJ) -o $(TARGET)

clean:
	rm $(TARGET) $(TARGET_OBJ)

cleanall:
	make clean
	rm default.db
