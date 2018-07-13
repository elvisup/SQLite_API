#ifeq ($(LIBTYPE), muclibc)
	#CROSS_COMPILE := mips-linux-uclibc-gnu-
#else
	#CROSS_COMPILE := mips-linux-gnu-
#endif

CROSS_COMPILE :=

CC := $(CROSS_COMPILE)gcc
CPP := $(CROSS_COMPILE)g++
LD := $(CROSS_COMPILE)ld
AR := $(CROSS_COMPILE)ar

CFLAGS := -g -O2 -Wall -lpthread -lrt -ldl -lm

LIBS :=  \
	-L./sqlite/lib -lsqlite3

INCLUDE := -I./sqlite/include \
	   -I./src/inc \
	   -I./include

OBJS := ./src/utils/slog.o \
	./src/hotdb.o

TARGET := libhotdb.a

All: $(TARGET)

%.o:%.c
	$(CC) $(INCLUDE) $(LIBS) -o $@ -c $^

%.o:%.cpp
	$(CPP) $(INCLUDE) $(LIBS) -o $@ -c $^

$(TARGET): $(OBJS)
	$(LD) -EL -r -Bstatic $^ $(LIBS) -o $@
	#strip $(TARGET)

demo:
	$(CC) ./test/test.c -o demo -L./ -lhotdb -I./include $(CFLAGS)
	strip demo

clean:
	rm -rf $(TARGET) $(OBJS) *.db demo
