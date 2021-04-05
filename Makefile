# 16-bit mote address
#DEFAULT_ADDR = 6
# 16-bit personal address 
#DEFAULT_PANID = 2555
# Channel NO.22
#DEFAULT_CHANNEL = 26

# 16-bit mote address
#DEFAULT_ADDR = 3864
# 16-bit personal address 
#DEFAULT_PANID = 0x22
# Channel NO.22
#DEFAULT_CHANNEL = 0x17


# Platform to build the code for
#PLATFORM := iwing-mrf
PLATFORM := sim


# Required target without extension
#TARGET := blink
#TARGET := sense-radio
#TARGET := blink_tutor
#TARGET := test-sht11
#TARGET := test-uart
#TARGET := test-uart2
#TARGET := count-button
#TARGET ?= count-button-radio
#TARGET := sense-light
#TARGET := send-answer
#TARGET := test-sdi12

#TARGET = app1
#TARGET = sense-light
#TARGET = sense_to_base
#TARGET = voter
#TARGET = blink

# Use with sim-flood-gw.py & sim-flood.py
#TARGET := flood_orig
TARGET := flood


# Final format for the built target, e.g., elf, hex
FORMAT := elf
#FORMAT := hex


# Extra object files used for the target
OBJS :=


#####################################
# Directory storing mote library
MOTELIB_DIR := $(shell pwd)/../motelib
include $(MOTELIB_DIR)/Makerules

# Enable all debuging message
#DEBUG=1 # This will disable UART TX
#UART_VIA_USB = 0
#####################################
