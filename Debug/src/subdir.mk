################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/UART.c \
../src/main.c \
../src/read_RTC.c \
../src/syscalls.c \
../src/system_stm32f10x.c 

OBJS += \
./src/UART.o \
./src/main.o \
./src/read_RTC.o \
./src/syscalls.o \
./src/system_stm32f10x.o 

C_DEPS += \
./src/UART.d \
./src/main.d \
./src/read_RTC.d \
./src/syscalls.d \
./src/system_stm32f10x.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -mfloat-abi=soft -DSTM32 -DSTM32F1 -DSTM32F103RBTx -DNUCLEO_F103RB -DDEBUG -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER -I"C:/Users/DJPifPaf/workspace/Clock_02_HH_display/Utilities/STM32F1xx-Nucleo" -I"C:/Users/DJPifPaf/workspace/Clock_02_HH_display/StdPeriph_Driver/inc" -I"C:/Users/DJPifPaf/workspace/Clock_02_HH_display/inc" -I"C:/Users/DJPifPaf/workspace/Clock_02_HH_display/CMSIS/device" -I"C:/Users/DJPifPaf/workspace/Clock_02_HH_display/CMSIS/core" -O0 -g3 -Wall -fmessage-length=0 -ffunction-sections -c -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


