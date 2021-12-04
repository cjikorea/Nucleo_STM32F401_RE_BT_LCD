################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../BlueNRG_2/Target/hci_tl_interface.c 

OBJS += \
./BlueNRG_2/Target/hci_tl_interface.o 

C_DEPS += \
./BlueNRG_2/Target/hci_tl_interface.d 


# Each subdirectory must supply rules for building sources it contributes
BlueNRG_2/Target/hci_tl_interface.o: ../BlueNRG_2/Target/hci_tl_interface.c BlueNRG_2/Target/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32F401xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../BlueNRG_2/App -I../BlueNRG_2/Target -I../Drivers/BSP/STM32F4xx_Nucleo -I../Middlewares/ST/BlueNRG-2/hci/hci_tl_patterns/Basic -I../Middlewares/ST/BlueNRG-2/utils -I../Middlewares/ST/BlueNRG-2/includes -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"BlueNRG_2/Target/hci_tl_interface.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

