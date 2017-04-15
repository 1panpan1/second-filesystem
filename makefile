CC = g++
CFLAGS = -g -Wno-reorder
INCLUDE = ./project/include
SRC = ./project/src
OBJ_PATH = ./project/obj
OBJ = $(OBJ_PATH)/Bitmap.o $(OBJ_PATH)/BlockDevice.o $(OBJ_PATH)/BufferManager.o $(OBJ_PATH)/File.o $(OBJ_PATH)/FileSystem.o $(OBJ_PATH)/fsmain.o $(OBJ_PATH)/func.o $(OBJ_PATH)/Inode.o

all:FStm
FStm: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o FStm

$(OBJ_PATH)/Bitmap.o: $(SRC)/Bitmap.cpp $(INCLUDE)/Bitmap.h $(INCLUDE)/FileSystem.h 
	$(CC) $(CFLAGS) -I "$(INCLUDE)" -c $(SRC)/Bitmap.cpp -o $(OBJ_PATH)/Bitmap.o

$(OBJ_PATH)/BlockDevice.o:  $(SRC)/BlockDevice.cpp $(INCLUDE)/BlockDevice.h $(INCLUDE)/FileSystem.h $(INCLUDE)/message.h
	$(CC) $(CFLAGS) -I "$(INCLUDE)" -c $(SRC)/BlockDevice.cpp -o $(OBJ_PATH)/BlockDevice.o

$(OBJ_PATH)/BufferManager.o : $(SRC)/BufferManager.cpp $(INCLUDE)/BufferManager.h $(INCLUDE)/BlockDevice.h $(INCLUDE)/message.h
	$(CC) $(CFLAGS) -I "$(INCLUDE)" -c $(SRC)/BufferManager.cpp -o $(OBJ_PATH)/BufferManager.o

$(OBJ_PATH)/File.o: $(SRC)/File.cpp $(INCLUDE)/File.h $(INCLUDE)/FileSystem.h $(INCLUDE)/BufferManager.h $(INCLUDE)/message.h 
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -c $(SRC)/File.cpp -o $(OBJ_PATH)/File.o

$(OBJ_PATH)/FileSystem.o: $(SRC)/FileSystem.cpp $(INCLUDE)/FileSystem.h $(INCLUDE)/BlockDevice.h $(INCLUDE)/Bitmap.h $(INCLUDE)/inode.h $(INCLUDE)/message.h
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -c $(SRC)/FileSystem.cpp -o $(OBJ_PATH)/FileSystem.o 

$(OBJ_PATH)/fsmain.o: $(SRC)/fsmain.cpp $(INCLUDE)/FileSystem.h $(INCLUDE)/func.h 
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -c $(SRC)/fsmain.cpp -o $(OBJ_PATH)/fsmain.o 

$(OBJ_PATH)/func.o: $(SRC)/func.cpp $(INCLUDE)/FileSystem.h $(INCLUDE)/func.h $(INCLUDE)/message.h
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -c $(SRC)/func.cpp -o $(OBJ_PATH)/func.o 

$(OBJ_PATH)/Inode.o: $(SRC)/Inode.cpp $(INCLUDE)/inode.h $(INCLUDE)/FileSystem.h $(INCLUDE)/BufferManager.h 
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -c $(SRC)/Inode.cpp -o $(OBJ_PATH)/Inode.o

clean:
	rm -f ./project/obj/*.o