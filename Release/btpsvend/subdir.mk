################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
/Users/andrey/Documents/Common/Bluetooth/Stacks/bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/btpsvend/BTPSVEND.c 

C_DEPS += \
./btpsvend/BTPSVEND.d 

OBJS += \
./btpsvend/BTPSVEND.o 


# Each subdirectory must supply rules for building sources it contributes
btpsvend/BTPSVEND.o: /Users/andrey/Documents/Common/Bluetooth/Stacks/bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/btpsvend/BTPSVEND.c btpsvend/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32L4R5xx -D__SUPPORT_LOW_ENERGY__ -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I../FATFS/Target -I../FATFS/App -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Bluetooth/Inc -I../bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/include -I../bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/profiles/DIS/include -I../bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/profiles/GAPS/include -I../bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/profiles/GATT/include -I../bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/profiles/HRS/include -I../bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/profiles/AVRCP/include -I../bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/profiles -I../bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/profiles/HDSET/include -I../bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/profiles/A2DP/include -I../bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/profiles/AVCTP/include -I../bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/profiles/AUDIO/include -I../bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/profiles/GAVD/include -I../bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/SBC/include -I../bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/btvs -I../bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/btpskrnl -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

