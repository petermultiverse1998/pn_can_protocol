################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../can_com/base_layer_can.c \
../can_com/base_layer_can_test.c \
../can_com/map.c \
../can_com/map_test.c \
../can_com/que.c \
../can_com/que_test.c 

OBJS += \
./can_com/base_layer_can.o \
./can_com/base_layer_can_test.o \
./can_com/map.o \
./can_com/map_test.o \
./can_com/que.o \
./can_com/que_test.o 

C_DEPS += \
./can_com/base_layer_can.d \
./can_com/base_layer_can_test.d \
./can_com/map.d \
./can_com/map_test.d \
./can_com/que.d \
./can_com/que_test.d 


# Each subdirectory must supply rules for building sources it contributes
can_com/%.o can_com/%.su can_com/%.cyclo: ../can_com/%.c can_com/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I"C:/Users/peter/OneDrive/Desktop/Github/pn_can_protocol/pn_can_protocol/can_com" -I"C:/Users/peter/OneDrive/Desktop/Github/pn_can_protocol/pn_can_protocol/user" -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-can_com

clean-can_com:
	-$(RM) ./can_com/base_layer_can.cyclo ./can_com/base_layer_can.d ./can_com/base_layer_can.o ./can_com/base_layer_can.su ./can_com/base_layer_can_test.cyclo ./can_com/base_layer_can_test.d ./can_com/base_layer_can_test.o ./can_com/base_layer_can_test.su ./can_com/map.cyclo ./can_com/map.d ./can_com/map.o ./can_com/map.su ./can_com/map_test.cyclo ./can_com/map_test.d ./can_com/map_test.o ./can_com/map_test.su ./can_com/que.cyclo ./can_com/que.d ./can_com/que.o ./can_com/que.su ./can_com/que_test.cyclo ./can_com/que_test.d ./can_com/que_test.o ./can_com/que_test.su

.PHONY: clean-can_com

