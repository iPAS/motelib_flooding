# https://makefiletutorial.com/
# https://stackoverflow.com/questions/448910/what-is-the-difference-between-the-gnu-makefile-variable-assignments-a

TARGET     ?= test_comm

PLATFORM   ?= sim
FORMAT     ?= elf

DEBUG      ?= 1

# DEFAULT_ADDR    ?= 3864
# DEFAULT_PANID   ?= 0x22
# DEFAULT_CHANNEL ?= 0x11

OBJS       += flood.o commqueue.o queue.o delivery_hist.o
LIBS       +=

MOTELIB_DIR ?= motelib
include $(MOTELIB_DIR)/Makerules
