################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/main.c \
../Src/systeminit.c \
../Src/usbd_driver.c \
../Src/usbd_framework.c 

OBJS += \
./Src/main.o \
./Src/systeminit.o \
./Src/usbd_driver.o \
./Src/usbd_framework.o 

C_DEPS += \
./Src/main.d \
./Src/systeminit.d \
./Src/usbd_driver.d \
./Src/usbd_framework.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o: ../Src/%.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSTM32 -DSTM32F407G_DISC1 -DSTM32F4 -DSTM32F407VGTx -DSTM32F407xx -c -I../Inc -I"E:/Mayur/Embedded projects/USB-Project/USB-Drivers/Inc/CMSIS/Include" -I"E:/Mayur/Embedded projects/USB-Project/USB-Drivers/Inc/CMSIS/Device/ST/STM32F4xx/Include" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Src

clean-Src:
	-$(RM) ./Src/main.d ./Src/main.o ./Src/systeminit.d ./Src/systeminit.o ./Src/usbd_driver.d ./Src/usbd_driver.o ./Src/usbd_framework.d ./Src/usbd_framework.o

.PHONY: clean-Src

