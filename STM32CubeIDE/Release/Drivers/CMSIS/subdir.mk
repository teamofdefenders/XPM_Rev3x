################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/Users/sditd/XPM_Rev3x/Core/Src/system_stm32u5xx.c 

OBJS += \
./Drivers/CMSIS/system_stm32u5xx.o 

C_DEPS += \
./Drivers/CMSIS/system_stm32u5xx.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/CMSIS/system_stm32u5xx.o: C:/Users/sditd/XPM_Rev3x/Core/Src/system_stm32u5xx.c Drivers/CMSIS/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -DBUZZER -DCamera -DCAMERA_RAW_OUT -DCollect_Info -DDEV_INIT -DDevice_Sleep -DEXTERNAL_XPS_STORAGE -DFixed_Modem_Time -DNO_FORMAT_PIC -DInit_Components -DInit_Device -DInit_HeartBeat -DINTERRUPT -DLog -DLog_2_Start -DLog_Level_0 -DLog_Level_1 -DLog_Level_2 -DNO_FORMAT_PIC -DNO_MODEM_RESTART -DPDP_CONTEXT -DPIR_SENSOR -DSKYWIRE_MODEM -DSKYWIRE_MODEM_OUT -DSTM32U575xx -DTEMPERATURE_SENSOR -DUART_USB -DUpdt_Components -DUSE_HAL_DRIVER -DVALID_PACKAGE -UDEBUG_MSG -UNO_FORMAT_PIC -UNO_MODEM_RESTART -UPDP_CONTEXT -UCERTIFICATE -c -I../../Core/Inc -I../../Drivers/STM32U5xx_HAL_Driver/Inc -I../../Drivers/STM32U5xx_HAL_Driver/Inc/Legacy -I../../Drivers/CMSIS/Device/ST/STM32U5xx/Include -I../../Drivers/CMSIS/Include -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Drivers-2f-CMSIS

clean-Drivers-2f-CMSIS:
	-$(RM) ./Drivers/CMSIS/system_stm32u5xx.cyclo ./Drivers/CMSIS/system_stm32u5xx.d ./Drivers/CMSIS/system_stm32u5xx.o ./Drivers/CMSIS/system_stm32u5xx.su

.PHONY: clean-Drivers-2f-CMSIS

