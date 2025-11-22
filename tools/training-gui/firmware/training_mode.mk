##
# Training Mode Firmware Makefile
# This is a minimal firmware for collecting training images
##

# Application name
override APPL_NAME = training_mode

# Application path
APP_PATH = $(EPII_ROOT)/../../tools/training-gui/firmware

##
# Source Files
##

APPL_CSRC = \
    $(APP_PATH)/training_mode.c

APPL_CXXSRC =

APPL_ASMSRC =

##
# Include Paths
##

APPL_INCLUDES = \
    -I$(APP_PATH) \
    -I$(EPII_ROOT)/library/camera \
    -I$(EPII_ROOT)/library/drivers/inc \
    -I$(EPII_ROOT)/library/common/inc

##
# Library Selection
##

LIB_SEL = \
    sensordp \
    hxevent \
    epii_CM55M_s_library \
    infra

# Camera sensor (HM0360)
LIB_SEL += hm0360

##
# Compiler Defines
##

APPL_DEFINES = \
    -DCFG_CAMERA_SENSOR=HM0360 \
    -DTRAINING_MODE=1 \
    -DDEBUG_LEVEL=2

##
# Compiler Flags
##

# Optimization (size optimization for faster builds)
APPL_OPT = -O2

# C flags
APPL_CFLAGS = \
    -Wall \
    -Wextra \
    -Wno-unused-parameter \
    -Wno-unused-function \
    -ffunction-sections \
    -fdata-sections

##
# Linker Flags
##

APPL_LDFLAGS = \
    -Wl,--gc-sections \
    -Wl,--print-memory-usage

APPL_LIBS = \
    -lm \
    -lc \
    -lgcc
