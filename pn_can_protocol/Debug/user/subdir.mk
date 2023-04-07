################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../user/pn_can_protocol.c \
../user/sync_layer_can.c \
../user/user.c 

OBJS += \
./user/pn_can_protocol.o \
./user/sync_layer_can.o \
./user/user.o 

C_DEPS += \
./user/pn_can_protocol.d \
./user/sync_layer_can.d \
./user/user.d 


# Each subdirectory must supply rules for building sources it contributes
user/%.o user/%.su user/%.cyclo: ../user/%.c user/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I"C:/Users/peter/OneDrive/Desktop/Github/pn_can_protocol/pn_can_protocol/can_com" -I"C:/Users/peter/OneDrive/Desktop/Github/pn_can_protocol/pn_can_protocol/user" -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-user

clean-user:
	-$(RM) ./user/pn_can_protocol.cyclo ./user/pn_can_protocol.d ./user/pn_can_protocol.o ./user/pn_can_protocol.su ./user/sync_layer_can.cyclo ./user/sync_layer_can.d ./user/sync_layer_can.o ./user/sync_layer_can.su ./user/user.cyclo ./user/user.d ./user/user.o ./user/user.su

.PHONY: clean-user

