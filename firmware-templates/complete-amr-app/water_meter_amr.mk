##
# Water Meter AMR Application Makefile
##

# Application name
override APPL_NAME = water_meter_amr

# Application path
APP_PATH = $(EPII_ROOT)/app/scenario_app/$(APPL_NAME)

##
# Source Files
##

# C source files
APPL_CSRC = \
    $(APP_PATH)/src/main.c \
    $(APP_PATH)/src/camera.c \
    $(APP_PATH)/src/lorawan_comm.c \
    $(APP_PATH)/src/power_mgmt.c \
    $(APP_PATH)/src/digit_detection.c

# C++ source files (for TensorFlow Lite)
APPL_CXXSRC = \
    $(APP_PATH)/src/inference.cpp

# Assembly source files
APPL_ASMSRC =

##
# Include Paths
##

APPL_INCLUDES = \
    -I$(APP_PATH)/include \
    -I$(APP_PATH)/models \
    -I$(APP_PATH)/config

##
# Library Selection
##

# Core libraries
LIB_SEL = \
    sensordp \
    hxevent \
    httpd \
    mqtt \
    mbedtls \
    epii_CM55M_s_library \
    infra

# Camera sensor (HM0360)
LIB_SEL += hm0360

# TensorFlow Lite Micro
LIB_SEL += tflmtag2209_u55tag2205

##
# Compiler Defines
##

APPL_DEFINES = \
    -DCFG_CAMERA_SENSOR=HM0360 \
    -DCFG_ENABLE_ML=1 \
    -DCFG_ENABLE_LORAWAN=1 \
    -DDEBUG_LEVEL=2

##
# Compiler Flags
##

# Optimization
APPL_OPT = -O2

# C flags
APPL_CFLAGS = \
    -Wall \
    -Wextra \
    -Wno-unused-parameter \
    -Wno-unused-function \
    -ffunction-sections \
    -fdata-sections

# C++ flags
APPL_CXXFLAGS = \
    -std=c++11 \
    -fno-rtti \
    -fno-exceptions \
    -fno-threadsafe-statics \
    -fno-use-cxa-atexit

##
# Linker Flags
##

APPL_LDFLAGS = \
    -Wl,--gc-sections \
    -Wl,--print-memory-usage \
    -Wl,-Map=$(OUT_DIR)/output.map

APPL_LIBS = \
    -lm \
    -lc \
    -lgcc \
    -lstdc++
