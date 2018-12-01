#ifeq ($(LIBTYPE), muclibc)
	#CROSS_COMPILE := mips-linux-uclibc-gnu-
#else
	#CROSS_COMPILE := mips-linux-gnu-
#endif

CROSS_COMPILE := mips-linux-gnu-

CC := $(CROSS_COMPILE)gcc
CPP := $(CROSS_COMPILE)g++
LD := $(CROSS_COMPILE)ld
AR := $(CROSS_COMPILE)ar
STRIP := $(CROSS_COMPILE)strip

CFLAGS := -g -O2 -Wall -lpthread -lrt -ldl -lm

LIBS :=  \
	-L../ -lhotdb

INCLUDE := -I../include

OBJS := ./hotdb_demo.o

TARGET := hotdb_demo

All: $(TARGET)

%.o:%.c
	$(CC) $(INCLUDE) $(LIBS) -o $@ -c $^

%.o:%.cpp
	$(CPP) $(INCLUDE) $(LIBS) -o $@ -c $^

$(TARGET): $(OBJS)
	$(CPP) $(OBJS) -o $@ $(INCLUDES) $(LIBS) $(CFLAGS)
	$(STRIP) $(TARGET)

clean:
	rm -rf $(TARGET) $(OBJS) *.db demo
