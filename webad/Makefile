export LD_LIBRARY_PATH=/usr/lib
CURRENT_DIR=`pwd`
SRC 			= $(wildcard *.c)
OBJS 			= $(patsubst %.c,%.o,$(SRC))

APP_INCLUDE += -I./include
LIBRARY	+= -lmnl -lnetfilter_queue -lpthread
TARGET			= webad

CFLAGS 			= $(CFLAGS_EXTRA) -Wall -g -o  


all: $(TARGET)
	@echo Generation TARGET
$(TARGET):
	$(CC) $(CFLAGS) $(TARGET) $(SRC) $(APP_INCLUDE)  $(LIBRARY)


.PHONY : clean
clean :
	@rm -f $(TARGET)
