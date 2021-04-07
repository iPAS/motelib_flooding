# https://makefiletutorial.com/
# https://stackoverflow.com/questions/448910/what-is-the-difference-between-the-gnu-makefile-variable-assignments-a

TARGET     ?= flood

PLATFORM   ?= sim
FORMAT     ?= elf

DEBUG      ?= 0

OBJS       +=
LIBS       +=

MOTELIB_DIR ?= motelib
include $(MOTELIB_DIR)/Makerules
