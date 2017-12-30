#
#       !!!! Do NOT edit this makefile with an editor which replace tabs by spaces !!!!   
#
##############################################################################################
#
# On command line:
#
# make all = Create project
#
# make clean = Clean project files.
#
# To rebuild project do "make clean" and "make all".o
#
# Included originally in the yagarto projects. Original Author : Michael Fischer
# Modified to suit our purposes by Hussam Al-Hertani
# Use at your own risk!!!!!
##############################################################################################
# Start of default section
#

STM32F4 = true
USE_OTG_MODE = true

TRGT = K:\Electronics\gcc\bin\arm-none-eabi-
CC   = $(TRGT)gcc
CP   = $(TRGT)objcopy
AS   = $(TRGT)gcc -x assembler-with-cpp
HEX  = $(CP) -O ihex
BIN  = $(CP) -O binary -S

MCU  = cortex-m4
 
# List all default C defines here, like -D_DEBUG=1
DDEFS = -DSTM32F0XX -DUSE_STDPERIPH_DRIVER
# List all default ASM defines here, like -D_DEBUG=1
DADEFS =
 
# List all default directories to look for include files here
DINCDIR =
 
# List the default directory to look for the libraries here
DLIBDIR =
 
# List all default libraries here
DLIBS =
 
#
# End of default section
##############################################################################################
 
##############################################################################################
# Start of user section
#
 
#
# Define project name and Ram/Flash mode here
PROJECT        = iotogglem0
RUN_FROM_FLASH = 1
 
# List all user C define here, like -D_DEBUG=1
UDEFS = 
 
# Define ASM defines here
UADEFS =
  
# List C source files here
#LIBSDIR    = ..\..\STM32F0-Discovery_FW_V1.0.0\Libraries
#CORELIBDIR = $(LIBSDIR)\CMSIS\Include
#DEVDIR  = $(LIBSDIR)\CMSIS\ST\STM32F0xx
#STMSPDDIR    = $(LIBSDIR)\STM32F0xx_StdPeriph_Driver

STMSPSRCDDIR  = .\stm32f4periph\src
STM32F4SPINCDDIR = .\stm32f4periph\inc
STMLINKER = stm32f4_linker.ld

CORELIBDIR = .\CMSIS\Include

LINKER = .\linker

SRC  = .\src\main.c
SRC += .\src\twb_config.c
SRC += .\src\nivek_system.c
SRC += .\src\commo\msg_parser.c
SRC += .\src\commo\commo.c
SRC += .\src\commo\wifi.c


SRC += .\src\commo\usb\twb_usb.c
SRC += .\src\commo\usb\usb_bsp.c
SRC += .\src\commo\usb\usb_core.c
SRC += .\src\commo\usb\usb_core_cfg.c
SRC += .\src\commo\usb\usb_core_io.c
SRC += .\src\commo\usb\usb_core_ep.c
SRC += .\src\commo\usb\usb_core_int.c
SRC += .\src\commo\usb\usb_dcd_int.c
SRC += .\src\commo\usb\usb_dcd.c
SRC += .\src\commo\usb\usbd_cdc_core.c
SRC += .\src\commo\usb\usbd_core.c
SRC += .\src\commo\usb\usbd_desc.c
SRC += .\src\commo\usb\usbd_ioreq.c
SRC += .\src\commo\usb\usbd_req.c
SRC += .\src\commo\usb\usbd_usr.c

SRC += .\src\filters\median.c

SRC += .\src\sensors\adxl345.c
SRC += .\src\sensors\hmc5883.c
SRC += .\src\sensors\sonar.c
SRC += .\src\sensors\bmp085.c
SRC += .\src\sensors\ms5611.c
SRC += .\src\sensors\gps.c
SRC += .\src\sensors\itg3200.c
SRC += .\src\sensors\mpu60x0_acc.c
SRC += .\src\sensors\mpu60x0_gryo.c
SRC += .\src\sensors\l3g20.c
SRC += .\src\sensors\lsm303_acc.c
SRC += .\src\sensors\lsm303_mag.c
SRC += .\src\sensors\lipo_adc.c
SRC += .\src\sensors\snsr_main.c
SRC += .\src\sensors\snsr_commo.c
SRC += .\src\sensors\snsr_mgmt.c
SRC += .\src\sensors\snsr_zero.c
SRC += .\src\sensors\snsr_actions.c
SRC += .\src\sensors\fusion\complementary.c



SRC += .\src\common\memory_mgmt.c
SRC += .\src\system\cli.c
SRC += .\src\common\twb_i2c.c
SRC += .\src\common\twb_strings.c
SRC += .\src\common\twb_math.c
SRC += .\src\common\math\asinf.c

SRC += .\src\common\twb_serial_eeprom.c
SRC += .\src\common\twb_leds.c
SRC += .\src\common\twb_common.c
SRC += .\src\common\twb_debug.c
SRC += .\src\common\twb_timer.c

SRC += .\src\flight\pid.c
SRC += .\src\flight\ctrl_loop.c
SRC += .\src\flight\ctrl_loop_commo.c
SRC += .\src\flight\autopilot.c
SRC += .\src\flight\ctrl_loop_pid.c
SRC += .\src\flight\pwm_source.c
SRC += .\src\flight\pwm_capture.c


SRC += .\src\sensors\fusion\kalman.c
#SRC += .\src\sensors\fusion\kalman_calc.c
#SRC += .\src\common\matmath.c

ifdef stm32f0
SRC += .\src\stm32f0xx_it.c
SRC += .\src\system_stm32f0xx.c
SRC += .\src\common\twb_i2c_stm32f0.c

SRC += $(STMSPSRCDDIR)\stm32f0xx_dma.c
SRC += $(STMSPSRCDDIR)\stm32f0xx_adc.c
SRC += $(STMSPSRCDDIR)\stm32f0xx_gpio.c

SRC += $(STMSPSRCDDIR)\stm32f0xx_i2c.c
SRC += $(STMSPSRCDDIR)\stm32f0xx_rcc.c

SRC += $(STMSPSRCDDIR)\stm32f0xx_tim.c
SRC += $(STMSPSRCDDIR)\stm32f0xx_usart.c
SRC += $(STMSPSRCDDIR)\stm32f0xx_exti.c
SRC += $(STMSPSRCDDIR)\stm32f0xx_misc.c
SRC += $(STMSPSRCDDIR)\stm32f0xx_syscfg.c
endif

ifdef STM32F4
SRC += .\src\stm32f4xx_it.c
SRC += .\src\system_stm32f4xx.c
SRC += .\src\common\twb_i2c_stm32f4.c

SRC += $(STMSPSRCDDIR)\misc.c
SRC += $(STMSPSRCDDIR)\stm32f4xx_dma.c
SRC += $(STMSPSRCDDIR)\stm32f4xx_adc.c
SRC += $(STMSPSRCDDIR)\stm32f4xx_gpio.c

SRC += $(STMSPSRCDDIR)\stm32f4xx_i2c.c
SRC += $(STMSPSRCDDIR)\stm32f4xx_rcc.c

SRC += $(STMSPSRCDDIR)\stm32f4xx_tim.c
SRC += $(STMSPSRCDDIR)\stm32f4xx_usart.c
SRC += $(STMSPSRCDDIR)\stm32f4xx_exti.c
#SRC += $(STMSPSRCDDIR)\stm32f0xx_misc.c
SRC += $(STMSPSRCDDIR)\stm32f4xx_syscfg.c
endif


# List ASM source files here
ifdef STM32F0
ASRC = .\startup\startup_stm32f0xx.s
endif

ifdef STM32F4
#ASRC = .\src\common\fmath.s
ASRC += .\startup\startup_stm32f4xx.s

endif

UINCDIR = $(DEVDIR)\STMSPINCDDIR \
		  $(STM32F4SPINCDDIR) \
		  $(STM32F0SPINCDDIR) \
          $(CORELIBDIR) \
          .\inc\commo\usb \
          .\inc\commo \
          .\inc\common \
          .\inc

     
# List the user directory to look for the libraries here
ULIBDIR =
 
# List all user libraries here
ULIBS =
 
# Define optimisation level here
OPT = -Os -mfpu=fpv4-sp-d16 -mfloat-abi=hard
#OPT = -Os

 
#
# End of user defines
##############################################################################################
#
# Define linker script file here
#
ifeq ($(RUN_FROM_FLASH), 0)
LDSCRIPT = $(LINKER)\$(STMLINKER)
FULL_PRJ = $(PROJECT)_ram
else
LDSCRIPT = .\linker\$(STMLINKER)
FULL_PRJ = $(PROJECT)_rom
endif
 
INCDIR  = $(patsubst %,-I%,$(DINCDIR) $(UINCDIR))
LIBDIR  = $(patsubst %,-L%,$(DLIBDIR) $(ULIBDIR))
 
ifeq ($(RUN_FROM_FLASH), 0)
DEFS    = $(DDEFS) $(UDEFS) -DRUN_FROM_FLASH=0 -DVECT_TAB_SRAM
else
DEFS    = $(DDEFS) $(UDEFS) -DRUN_FROM_FLASH=1
endif
 
ADEFS   = $(DADEFS) $(UADEFS)
OBJS  = $(ASRC:.s=.o) $(SRC:.c=.o)
LIBS    = $(DLIBS) $(ULIBS)
MCFLAGS = -mcpu=$(MCU)
 
ASFLAGS = $(MCFLAGS) -g -gdwarf-2 -mthumb  -Wa,-amhls=$(<:.s=.lst) $(ADEFS)
CPFLAGS = $(MCFLAGS) $(OPT) -gdwarf-2 -mthumb   -fomit-frame-pointer -Wall -Wstrict-prototypes -fverbose-asm -Wa,-ahlms=$(<:.c=.lst) $(DEFS)
LDFLAGS = $(MCFLAGS) -mthumb -nostartfiles -T$(LDSCRIPT) -Wl,-Map=$(FULL_PRJ).map,--cref,--no-warn-mismatch $(LIBDIR)
 
# Generate dependency information
CPFLAGS += -MD -MP -MF .dep\$(@F).d

OPTIMIZE = -Os
 
#
# makefile rules
#
 
all: $(OBJS) $(FULL_PRJ).elf  $(FULL_PRJ).hex $(FULL_PRJ).bin
ifeq ($(RUN_FROM_FLASH), 0)
	$(TRGT)size $(PROJECT)_ram.elf
else
		$(TRGT)size -A -t -x $(PROJECT)_rom.elf
#       $(TRGT)size -A -t -x $(OBJS)
endif
 
 %o: %c
	$(CC) -std=gnu99 -c $(OPTIMIZE) $(CPFLAGS) -I . $(INCDIR) $< -o $@

%o: %s
	$(AS) -c $(OPTIMIZE) $(ASFLAGS) $< -o $@

%elf: $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) $(LIBS) -o $@

%hex: %elf
	$(HEX) $< $@
	
%bin: %elf
	$(BIN)  $< $@
	
clean:
	del $(OBJS)
	#del $(FULL_PRJ).elf
	#del $(FULL_PRJ).map
	#del $(FULL_PRJ).hex
	#del $(FULL_PRJ).bin
#	del $(SRC:.c=.c.bak)
#	del $(SRC:.c=.lst)
#   del $(ASRC:.s=.s.bak)
#	del $(ASRC:.s=.lst)
	rmdir  .dep /S /Q
    
    
# 
# Include the dependency files, should be the last of the makefile
#
#-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)
#23-July-2012 /dev/null gives an error on Win 7 64-bit : Hussam
-include $(shell mkdir .dep) $(wildcard .dep/*)

# *** EOF ***