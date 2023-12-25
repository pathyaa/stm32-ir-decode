################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Application/NEC/NEC.c 

OBJS += \
./Application/NEC/NEC.o 

C_DEPS += \
./Application/NEC/NEC.d 


# Each subdirectory must supply rules for building sources it contributes
Application/NEC/%.o Application/NEC/%.su Application/NEC/%.cyclo: ../Application/NEC/%.c Application/NEC/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103x6 -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I"D:/home/stm32-workspace/git/stm32-nec/stm32-ir/Application" -I"D:/home/stm32-workspace/git/stm32-nec/stm32-ir/Application/NEC" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Application-2f-NEC

clean-Application-2f-NEC:
	-$(RM) ./Application/NEC/NEC.cyclo ./Application/NEC/NEC.d ./Application/NEC/NEC.o ./Application/NEC/NEC.su

.PHONY: clean-Application-2f-NEC

