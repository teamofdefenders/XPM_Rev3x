################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Application/User/Core/Accelerometer.c \
../Application/User/Core/ArducamCamera.c \
../Application/User/Core/Bootloader.c \
../Application/User/Core/Buzzer.c \
../Application/User/Core/Camera.c \
../Application/User/Core/DayNight.c \
../Application/User/Core/Environment.c \
../Application/User/Core/Functions.c \
../Application/User/Core/GPS.c \
../Application/User/Core/Interrupt.c \
../Application/User/Core/PIR.c \
../Application/User/Core/SD.c \
../Application/User/Core/Skywire.c \
../Application/User/Core/Temperature.c \
../Application/User/Core/XPS.c \
C:/Users/sditd/XPM_Rev3x/Core/Src/main.c \
C:/Users/sditd/XPM_Rev3x/Core/Src/stm32u5xx_hal_msp.c \
C:/Users/sditd/XPM_Rev3x/Core/Src/stm32u5xx_it.c \
../Application/User/Core/syscalls.c \
../Application/User/Core/sysmem.c 

OBJS += \
./Application/User/Core/Accelerometer.o \
./Application/User/Core/ArducamCamera.o \
./Application/User/Core/Bootloader.o \
./Application/User/Core/Buzzer.o \
./Application/User/Core/Camera.o \
./Application/User/Core/DayNight.o \
./Application/User/Core/Environment.o \
./Application/User/Core/Functions.o \
./Application/User/Core/GPS.o \
./Application/User/Core/Interrupt.o \
./Application/User/Core/PIR.o \
./Application/User/Core/SD.o \
./Application/User/Core/Skywire.o \
./Application/User/Core/Temperature.o \
./Application/User/Core/XPS.o \
./Application/User/Core/main.o \
./Application/User/Core/stm32u5xx_hal_msp.o \
./Application/User/Core/stm32u5xx_it.o \
./Application/User/Core/syscalls.o \
./Application/User/Core/sysmem.o 

C_DEPS += \
./Application/User/Core/Accelerometer.d \
./Application/User/Core/ArducamCamera.d \
./Application/User/Core/Bootloader.d \
./Application/User/Core/Buzzer.d \
./Application/User/Core/Camera.d \
./Application/User/Core/DayNight.d \
./Application/User/Core/Environment.d \
./Application/User/Core/Functions.d \
./Application/User/Core/GPS.d \
./Application/User/Core/Interrupt.d \
./Application/User/Core/PIR.d \
./Application/User/Core/SD.d \
./Application/User/Core/Skywire.d \
./Application/User/Core/Temperature.d \
./Application/User/Core/XPS.d \
./Application/User/Core/main.d \
./Application/User/Core/stm32u5xx_hal_msp.d \
./Application/User/Core/stm32u5xx_it.d \
./Application/User/Core/syscalls.d \
./Application/User/Core/sysmem.d 


# Each subdirectory must supply rules for building sources it contributes
Application/User/Core/%.o Application/User/Core/%.su Application/User/Core/%.cyclo: ../Application/User/Core/%.c Application/User/Core/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DACCELERATION_SENSOR -DBUZZER -DCamera -DNO_FORMAT_PIC -DCAMERA_RAW_OUT -DCollect_Info -DDEBUG -DDEBUG_MSG -DDEV_INIT -DDevice_Sleep -DEXTERNAL_XPS_STORAGE -DFixed_Modem_Time -DInit_Components -DInit_Device -DInit_HeartBeat -DINTERRUPT -DLog -DLog_2_Start -DLog_Level_0 -DLog_Level_1 -DLog_Level_2 -DNO_MODEM_RESTART -DPDP_CONTEXT -DPIR_SENSOR -DSKYWIRE_MODEM -DSKYWIRE_MODEM_OUT -DSTM32U575xx -DTEMPERATURE_SENSOR -DUART_USB -DUpdt_Components -DUSE_HAL_DRIVER -DVALID_PACKAGE -UDEBUG_MSG -UNO_FORMAT_PIC -UNO_MODEM_RESTART -UPDP_CONTEXT -UCERTIFICATE -c -I../../Core/Inc -I../../Drivers/STM32U5xx_HAL_Driver/Inc -I../../Drivers/STM32U5xx_HAL_Driver/Inc/Legacy -I../../Drivers/CMSIS/Device/ST/STM32U5xx/Include -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Application/User/Core/main.o: C:/Users/sditd/XPM_Rev3x/Core/Src/main.c Application/User/Core/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DACCELERATION_SENSOR -DBUZZER -DCamera -DNO_FORMAT_PIC -DCAMERA_RAW_OUT -DCollect_Info -DDEBUG -DDEBUG_MSG -DDEV_INIT -DDevice_Sleep -DEXTERNAL_XPS_STORAGE -DFixed_Modem_Time -DInit_Components -DInit_Device -DInit_HeartBeat -DINTERRUPT -DLog -DLog_2_Start -DLog_Level_0 -DLog_Level_1 -DLog_Level_2 -DNO_MODEM_RESTART -DPDP_CONTEXT -DPIR_SENSOR -DSKYWIRE_MODEM -DSKYWIRE_MODEM_OUT -DSTM32U575xx -DTEMPERATURE_SENSOR -DUART_USB -DUpdt_Components -DUSE_HAL_DRIVER -DVALID_PACKAGE -UDEBUG_MSG -UNO_FORMAT_PIC -UNO_MODEM_RESTART -UPDP_CONTEXT -UCERTIFICATE -c -I../../Core/Inc -I../../Drivers/STM32U5xx_HAL_Driver/Inc -I../../Drivers/STM32U5xx_HAL_Driver/Inc/Legacy -I../../Drivers/CMSIS/Device/ST/STM32U5xx/Include -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Application/User/Core/stm32u5xx_hal_msp.o: C:/Users/sditd/XPM_Rev3x/Core/Src/stm32u5xx_hal_msp.c Application/User/Core/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DACCELERATION_SENSOR -DBUZZER -DCamera -DNO_FORMAT_PIC -DCAMERA_RAW_OUT -DCollect_Info -DDEBUG -DDEBUG_MSG -DDEV_INIT -DDevice_Sleep -DEXTERNAL_XPS_STORAGE -DFixed_Modem_Time -DInit_Components -DInit_Device -DInit_HeartBeat -DINTERRUPT -DLog -DLog_2_Start -DLog_Level_0 -DLog_Level_1 -DLog_Level_2 -DNO_MODEM_RESTART -DPDP_CONTEXT -DPIR_SENSOR -DSKYWIRE_MODEM -DSKYWIRE_MODEM_OUT -DSTM32U575xx -DTEMPERATURE_SENSOR -DUART_USB -DUpdt_Components -DUSE_HAL_DRIVER -DVALID_PACKAGE -UDEBUG_MSG -UNO_FORMAT_PIC -UNO_MODEM_RESTART -UPDP_CONTEXT -UCERTIFICATE -c -I../../Core/Inc -I../../Drivers/STM32U5xx_HAL_Driver/Inc -I../../Drivers/STM32U5xx_HAL_Driver/Inc/Legacy -I../../Drivers/CMSIS/Device/ST/STM32U5xx/Include -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Application/User/Core/stm32u5xx_it.o: C:/Users/sditd/XPM_Rev3x/Core/Src/stm32u5xx_it.c Application/User/Core/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DACCELERATION_SENSOR -DBUZZER -DCamera -DNO_FORMAT_PIC -DCAMERA_RAW_OUT -DCollect_Info -DDEBUG -DDEBUG_MSG -DDEV_INIT -DDevice_Sleep -DEXTERNAL_XPS_STORAGE -DFixed_Modem_Time -DInit_Components -DInit_Device -DInit_HeartBeat -DINTERRUPT -DLog -DLog_2_Start -DLog_Level_0 -DLog_Level_1 -DLog_Level_2 -DNO_MODEM_RESTART -DPDP_CONTEXT -DPIR_SENSOR -DSKYWIRE_MODEM -DSKYWIRE_MODEM_OUT -DSTM32U575xx -DTEMPERATURE_SENSOR -DUART_USB -DUpdt_Components -DUSE_HAL_DRIVER -DVALID_PACKAGE -UDEBUG_MSG -UNO_FORMAT_PIC -UNO_MODEM_RESTART -UPDP_CONTEXT -UCERTIFICATE -c -I../../Core/Inc -I../../Drivers/STM32U5xx_HAL_Driver/Inc -I../../Drivers/STM32U5xx_HAL_Driver/Inc/Legacy -I../../Drivers/CMSIS/Device/ST/STM32U5xx/Include -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Application-2f-User-2f-Core

clean-Application-2f-User-2f-Core:
	-$(RM) ./Application/User/Core/Accelerometer.cyclo ./Application/User/Core/Accelerometer.d ./Application/User/Core/Accelerometer.o ./Application/User/Core/Accelerometer.su ./Application/User/Core/ArducamCamera.cyclo ./Application/User/Core/ArducamCamera.d ./Application/User/Core/ArducamCamera.o ./Application/User/Core/ArducamCamera.su ./Application/User/Core/Bootloader.cyclo ./Application/User/Core/Bootloader.d ./Application/User/Core/Bootloader.o ./Application/User/Core/Bootloader.su ./Application/User/Core/Buzzer.cyclo ./Application/User/Core/Buzzer.d ./Application/User/Core/Buzzer.o ./Application/User/Core/Buzzer.su ./Application/User/Core/Camera.cyclo ./Application/User/Core/Camera.d ./Application/User/Core/Camera.o ./Application/User/Core/Camera.su ./Application/User/Core/DayNight.cyclo ./Application/User/Core/DayNight.d ./Application/User/Core/DayNight.o ./Application/User/Core/DayNight.su ./Application/User/Core/Environment.cyclo ./Application/User/Core/Environment.d ./Application/User/Core/Environment.o ./Application/User/Core/Environment.su ./Application/User/Core/Functions.cyclo ./Application/User/Core/Functions.d ./Application/User/Core/Functions.o ./Application/User/Core/Functions.su ./Application/User/Core/GPS.cyclo ./Application/User/Core/GPS.d ./Application/User/Core/GPS.o ./Application/User/Core/GPS.su ./Application/User/Core/Interrupt.cyclo ./Application/User/Core/Interrupt.d ./Application/User/Core/Interrupt.o ./Application/User/Core/Interrupt.su ./Application/User/Core/PIR.cyclo ./Application/User/Core/PIR.d ./Application/User/Core/PIR.o ./Application/User/Core/PIR.su ./Application/User/Core/SD.cyclo ./Application/User/Core/SD.d ./Application/User/Core/SD.o ./Application/User/Core/SD.su ./Application/User/Core/Skywire.cyclo ./Application/User/Core/Skywire.d ./Application/User/Core/Skywire.o ./Application/User/Core/Skywire.su ./Application/User/Core/Temperature.cyclo ./Application/User/Core/Temperature.d ./Application/User/Core/Temperature.o ./Application/User/Core/Temperature.su ./Application/User/Core/XPS.cyclo ./Application/User/Core/XPS.d ./Application/User/Core/XPS.o ./Application/User/Core/XPS.su ./Application/User/Core/main.cyclo ./Application/User/Core/main.d ./Application/User/Core/main.o ./Application/User/Core/main.su ./Application/User/Core/stm32u5xx_hal_msp.cyclo ./Application/User/Core/stm32u5xx_hal_msp.d ./Application/User/Core/stm32u5xx_hal_msp.o ./Application/User/Core/stm32u5xx_hal_msp.su ./Application/User/Core/stm32u5xx_it.cyclo ./Application/User/Core/stm32u5xx_it.d ./Application/User/Core/stm32u5xx_it.o ./Application/User/Core/stm32u5xx_it.su ./Application/User/Core/syscalls.cyclo ./Application/User/Core/syscalls.d ./Application/User/Core/syscalls.o ./Application/User/Core/syscalls.su ./Application/User/Core/sysmem.cyclo ./Application/User/Core/sysmem.d ./Application/User/Core/sysmem.o ./Application/User/Core/sysmem.su

.PHONY: clean-Application-2f-User-2f-Core

