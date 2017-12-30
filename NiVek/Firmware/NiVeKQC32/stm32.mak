#This file is generated by VisualGDB.
#It contains GCC settings automatically derived from the board support package (BSP).
#DO NOT EDIT MANUALLY. THE FILE WILL BE OVERWRITTEN. 
#Use VisualGDB Project Properties dialog or modify Makefile or per-configuration .mak files instead.

#VisualGDB provides BSP_ROOT via environment when running Make. The line below will only be active if GNU Make is started manually.
BSP_ROOT ?= $(LOCALAPPDATA)/VisualGDB/EmbeddedBSPs/arm-eabi/com.sysprogs.arm.stm32
TOOLCHAIN_ROOT := k:/Electronics/SysGCC/arm-eabi

#Embedded toolchain
CC := $(TOOLCHAIN_ROOT)/bin/arm-eabi-gcc.exe
CXX := $(TOOLCHAIN_ROOT)/bin/arm-eabi-g++.exe
LD := $(CXX)
AR := $(TOOLCHAIN_ROOT)/bin/arm-eabi-ar.exe
OBJCOPY := $(TOOLCHAIN_ROOT)/bin/arm-eabi-objcopy.exe

#Additional flags
PREPROCESSOR_MACROS += ARM_MATH_CM4 stm32_flash_layout STM32F405RG STM32F40XX
INCLUDE_DIRS += $(BSP_ROOT)/STM32F4xxxx/CMSIS/Device/ST/STM32F4xx/Include $(BSP_ROOT)/STM32F4xxxx/CMSIS/Include $(BSP_ROOT)/STM32F4xxxx/CMSIS/RTOS $(BSP_ROOT)/STM32F4xxxx/STM32F4xx_StdPeriph_Driver/inc
LIBRARY_DIRS += 
LIBRARY_NAMES += 
ADDITIONAL_LINKER_INPUTS += 
MACOS_FRAMEWORKS += 

CFLAGS += -mcpu=cortex-m4 -mthumb
CXXFLAGS += -mcpu=cortex-m4 -mthumb
ASFLAGS += -mcpu=cortex-m4 -mthumb
LDFLAGS += -mcpu=cortex-m4 -mthumb  -T$(BSP_ROOT)/STM32F4xxxx/LinkerScripts/STM32F405xG_flash.lds
COMMONFLAGS +=  -mfloat-abi=hard

