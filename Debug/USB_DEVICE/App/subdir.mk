################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../USB_DEVICE/App/usb_device.c \
../USB_DEVICE/App/usbd_cdc_if.c \
../USB_DEVICE/App/usbd_desc.c 

C_DEPS += \
./USB_DEVICE/App/usb_device.d \
./USB_DEVICE/App/usbd_cdc_if.d \
./USB_DEVICE/App/usbd_desc.d 

OBJS += \
./USB_DEVICE/App/usb_device.o \
./USB_DEVICE/App/usbd_cdc_if.o \
./USB_DEVICE/App/usbd_desc.o 


# Each subdirectory must supply rules for building sources it contributes
USB_DEVICE/App/%.o: ../USB_DEVICE/App/%.c USB_DEVICE/App/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu17 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L4R5xx -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I../FATFS/Target -I../FATFS/App -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Bluetooth/Inc -I/Users/andrey/Documents/Common/Bluetooth/Stacks/bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/include -I/Users/andrey/Documents/Common/Bluetooth/Stacks/bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/btpskrnl -I/Users/andrey/Documents/Common/Bluetooth/Stacks/bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/btvs -I/Users/andrey/Documents/Common/Bluetooth/Stacks/bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/SBC/include -I/Users/andrey/Documents/Common/Bluetooth/Stacks/bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/profiles/GAVD/include -I/Users/andrey/Documents/Common/Bluetooth/Stacks/bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/profiles/AUDIO/include -I/Users/andrey/Documents/Common/Bluetooth/Stacks/bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/profiles/AVCTP/include -I/Users/andrey/Documents/Common/Bluetooth/Stacks/bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/profiles/A2DP/include -I/Users/andrey/Documents/Common/Bluetooth/Stacks/bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/profiles/HDSET/include -I/Users/andrey/Documents/Common/Bluetooth/Stacks/bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/profiles -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I/Users/andrey/Documents/Common/Bluetooth/Stacks/bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/profiles/AVRCP/include -I"/Users/andrey/Documents/Common/bluetopia/CC256x STM32 Bluetopia SDK/v4.0.2.2/FreeRTOS/Bluetopia/profiles/HRS/include" -I"/Users/andrey/Documents/Common/bluetopia/CC256x STM32 Bluetopia SDK/v4.0.2.2/FreeRTOS/Bluetopia/profiles/GATT/include" -I"/Users/andrey/Documents/Common/bluetopia/CC256x STM32 Bluetopia SDK/v4.0.2.2/FreeRTOS/Bluetopia/profiles/GAPS/include" -I"/Users/andrey/Documents/Common/bluetopia/CC256x STM32 Bluetopia SDK/v4.0.2.2/FreeRTOS/Bluetopia/profiles/DIS/include" -Og -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@"  -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

