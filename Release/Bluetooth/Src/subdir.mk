################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Bluetooth/Src/DIS.c \
../Bluetooth/Src/GAPS.c \
../Bluetooth/Src/HCITRANS.c \
../Bluetooth/Src/HRPDemo.c \
../Bluetooth/Src/HRS.c 

C_DEPS += \
./Bluetooth/Src/DIS.d \
./Bluetooth/Src/GAPS.d \
./Bluetooth/Src/HCITRANS.d \
./Bluetooth/Src/HRPDemo.d \
./Bluetooth/Src/HRS.d 

OBJS += \
./Bluetooth/Src/DIS.o \
./Bluetooth/Src/GAPS.o \
./Bluetooth/Src/HCITRANS.o \
./Bluetooth/Src/HRPDemo.o \
./Bluetooth/Src/HRS.o 


# Each subdirectory must supply rules for building sources it contributes
Bluetooth/Src/%.o: ../Bluetooth/Src/%.c Bluetooth/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32L4R5xx -D__SUPPORT_LOW_ENERGY__ -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I../FATFS/Target -I../FATFS/App -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Bluetooth/Inc -I/Users/andrey/Documents/Common/Bluetooth/Stacks/bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/include -I/Users/andrey/Documents/Common/Bluetooth/Stacks/bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/btpskrnl -I/Users/andrey/Documents/Common/Bluetooth/Stacks/bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/btvs -I/Users/andrey/Documents/Common/Bluetooth/Stacks/bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/SBC/include -I/Users/andrey/Documents/Common/Bluetooth/Stacks/bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/profiles/GAVD/include -I/Users/andrey/Documents/Common/Bluetooth/Stacks/bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/profiles/AUDIO/include -I/Users/andrey/Documents/Common/Bluetooth/Stacks/bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/profiles/AVCTP/include -I/Users/andrey/Documents/Common/Bluetooth/Stacks/bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/profiles/A2DP/include -I/Users/andrey/Documents/Common/Bluetooth/Stacks/bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/profiles/HDSET/include -I/Users/andrey/Documents/Common/Bluetooth/Stacks/bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/profiles/AVRCP/include -I/Users/andrey/Documents/Common/Bluetooth/Stacks/bluetopia/v4.0.2.2/FreeRTOS/Bluetopia/profiles -I"/Users/andrey/Documents/Common/bluetopia/CC256x STM32 Bluetopia SDK/v4.0.2.2/FreeRTOS/Bluetopia/profiles/HRS/include" -I"/Users/andrey/Documents/Common/bluetopia/CC256x STM32 Bluetopia SDK/v4.0.2.2/FreeRTOS/Bluetopia/profiles/GATT/include" -I"/Users/andrey/Documents/Common/bluetopia/CC256x STM32 Bluetopia SDK/v4.0.2.2/FreeRTOS/Bluetopia/profiles/GAPS/include" -I"/Users/andrey/Documents/Common/bluetopia/CC256x STM32 Bluetopia SDK/v4.0.2.2/FreeRTOS/Bluetopia/profiles/DIS/include" -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

