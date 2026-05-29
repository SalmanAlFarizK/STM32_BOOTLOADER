################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/bootloader/bootloader.c 

OBJS += \
./Core/bootloader/bootloader.o 

C_DEPS += \
./Core/bootloader/bootloader.d 


# Each subdirectory must supply rules for building sources it contributes
Core/bootloader/%.o Core/bootloader/%.su Core/bootloader/%.cyclo: ../Core/bootloader/%.c Core/bootloader/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../USB_HOST/App -I"D:/stm32_projects/STM32_Hobby_Projects/STM32_Bootloader/Stm32_Bootloader/Core/bootloader" -I../USB_HOST/Target -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/CDC/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-bootloader

clean-Core-2f-bootloader:
	-$(RM) ./Core/bootloader/bootloader.cyclo ./Core/bootloader/bootloader.d ./Core/bootloader/bootloader.o ./Core/bootloader/bootloader.su

.PHONY: clean-Core-2f-bootloader

