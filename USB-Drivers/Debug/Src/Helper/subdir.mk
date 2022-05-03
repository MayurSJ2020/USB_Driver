################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/Helper/logger.c 

OBJS += \
./Src/Helper/logger.o 

C_DEPS += \
./Src/Helper/logger.d 


# Each subdirectory must supply rules for building sources it contributes
Src/Helper/%.o: ../Src/Helper/%.c Src/Helper/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSTM32 -DSTM32F407G_DISC1 -DSTM32F4 -DSTM32F407VGTx -DSTM32F407xx -c -I../Inc -I"E:/Mayur/Embedded projects/USB-Project/USB-Drivers/Inc/CMSIS/Include" -I"E:/Mayur/Embedded projects/USB-Project/USB-Drivers/Inc/CMSIS/Device/ST/STM32F4xx/Include" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Src-2f-Helper

clean-Src-2f-Helper:
	-$(RM) ./Src/Helper/logger.d ./Src/Helper/logger.o

.PHONY: clean-Src-2f-Helper

