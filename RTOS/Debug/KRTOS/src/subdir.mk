################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../KRTOS/src/osKernel.c 

OBJS += \
./KRTOS/src/osKernel.o 

C_DEPS += \
./KRTOS/src/osKernel.d 


# Each subdirectory must supply rules for building sources it contributes
KRTOS/src/%.o KRTOS/src/%.su KRTOS/src/%.cyclo: ../KRTOS/src/%.c KRTOS/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DSTM32 -DSTM32F1 -DSTM32F103C8Tx -c -I"C:/RTOS/STM32Cube/RTOS/KRTOS/inc" -I../Inc -I"C:/RTOS/STM32Cube/RTOS/Inc/CMSIS/Include" -I"C:/RTOS/STM32Cube/RTOS/Inc/CMSIS/Device/ST/STM32F1xx/Include" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-KRTOS-2f-src

clean-KRTOS-2f-src:
	-$(RM) ./KRTOS/src/osKernel.cyclo ./KRTOS/src/osKernel.d ./KRTOS/src/osKernel.o ./KRTOS/src/osKernel.su

.PHONY: clean-KRTOS-2f-src

