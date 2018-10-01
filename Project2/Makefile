CC=gcc

SRCDIR=src/
INC=include/
LIBS=lib/

TARGET_SRC:=$(SRCDIR)main.c $(SRCDIR)page.c $(SRCDIR)startend.c
TARGET_OBJ:=$(SRCDIR)main.o $(SRCDIR)page.o $(SRCDIR)startend.o

CFLAGS+= -g -I $(INC)
OBJFLAGS+= -c -I $(INC)

TARGET:=main

all: $(TARGET)

$(TARGET) : $(TARGET_OBJ)
	$(CC) $(OBJFLAGS) $(SRCDIR)page.c -o $(SRCDIR)page.o 
	$(CC) $(OBJFLAGS) $(SRCDIR)startend.c -o $(SRCDIR)startend.o
	$(CC) $(CFLAGS) $(TARGET_OBJ) -o $(TARGET)

clean:
	rm $(TARGET) $(TARGET_OBJ)

cleanall:
	make clean
	rm default.db
