/**
  ******************************************************************************
  * @file    Bootloader.c
  * @author  Erik Pineda-A
  * @brief   To control ucontroller bootloader
  ******************************************************************************
  * @attention
  *
  * Copyright 2024 Team of Defenders
  *
  ******************************************************************************
  */
#ifndef BOOTLOADER_C_
#define BOOTLOADER_C_

#include "Bootloader.h"
#include "FUNCTIONS.h"
#include <string.h>

__IO uint8_t Data_In_Flash = 0;

//#define Memory _Memory
//#define State _State

/******************************************************
 Selects Boot Flash.Address
 Note: Set new boot address
 ******************************************************/
void Boot_Change(BANK_TYPE currentBank)
{
	FLASH_OBProgramInitTypeDef OptionsBytesInit;
	//	__root FLASH_TypeDef * const FLASH_DBG = FLASH;

#ifdef Log_Level_1
	Write_Log ( LOG_BOOTLOAD );
	Write_Log ( LOG_TAIL );
#endif

	/* Unlock Flash Control register and Option Bytes */
	HAL_FLASH_Unlock ();
	HAL_FLASH_OB_Unlock ();

	/* For TrustzoneDisabled IDE confiuguration: OPTIONBYTE_WRP / OPTIONBYTE_RDP / OPTIONBYTE_RDPKEY / OPTIONBYTE_USER / OPTIONBYTE_BOOTADDR
	 For TrustzoneEnabled IDE configuration:   OPTIONBYTE_WRP / OPTIONBYTE_RDP / OPTIONBYTE_RDPKEY / OPTIONBYTE_USER / OPTIONBYTE_BOOTADDR
	 OPTIONBYTE_WMSEC / OPTIONBYTE_BOOT_LOCK */

	//	OptionsBytesInit.OptionType = OPTIONBYTE_BOOTADDR;
	//	OptionsBytesInit.BootAddrConfig = OB_BOOTADDR_NS0; // OB_BOOTADDR_NS0 or OB_BOOTADDR_NS1
	//	OptionsBytesInit.BootAddr = ADDR_FLASH_PAGE_0 + 0x7F; // Range : 0x0 and 0xFFFFFF00
	/* Program Option Bytes */
	//	HAL_FLASHEx_OBProgram(&OptionsBytesInit);

	if (currentBank == BANK2)
	{
		OptionsBytesInit.OptionType = OPTIONBYTE_USER;
		OptionsBytesInit.USERConfig = OB_SWAP_BANK_DISABLE + OB_DUALBANK_DUAL; // OB_BOOTADDR_NS0 or OB_BOOTADDR_NS1
		OptionsBytesInit.USERType = OB_USER_SWAP_BANK + OB_USER_DUALBANK;

	}
	else
	{
		OptionsBytesInit.OptionType = OPTIONBYTE_USER;
		OptionsBytesInit.USERConfig = OB_SWAP_BANK_ENABLE + OB_DUALBANK_DUAL; // OB_BOOTADDR_NS0 or OB_BOOTADDR_NS1
		OptionsBytesInit.USERType = OB_USER_SWAP_BANK + OB_USER_DUALBANK;
	}

	//	OptionsBytesInit.OptionType = OPTIONBYTE_BOOTADDR;
	////	OptionsBytesInit.BootAddrConfig = OB_BOOTADDR_NS1;
	////	OptionsBytesInit.USERConfig = OB_SWAP_BANK_ENABLE ; // OB_BOOTADDR_NS0 or OB_BOOTADDR_NS1
	////	OptionsBytesInit.USERType = OB_USER_SWAP_BANK ;
	//	OptionsBytesInit.BootAddr = 0x08080000;

	/* Program Option Bytes */
	HAL_FLASHEx_OBProgram ( &OptionsBytesInit );

	/* Launch Option Bytes Loading */
	HAL_FLASH_OB_Launch ();
	HAL_FLASH_Lock ();	// Lock the Flash to disable the flash control register access

	//	NVIC_SystemReset();
}

/******************************************************
 Flash Function
 Note: Flash a predefined page to memory
 ******************************************************/
void Reflash ( MEM_PTR *bigData, BANK_TYPE isBank1 )
{
    uint8_t quadWord[ Quad_Word_Shift ] = { 0 };
	uint32_t currentFlashAddress = 0x08080000; // bank 2
	uint32_t bankStartAddress = 0x08080000; // bank 2
	HAL_StatusTypeDef status = HAL_OK;
	FLASH_EraseInitTypeDef EraseInitStruct;
	bool isVerifyWhileProgram = true;

	#ifdef Log_Level_1
	Write_Log(LOG_FLASHING);
	Write_Log(LOG_TAIL);
	#endif

	// Watchdog refresh before the process starts
	status = HAL_IWDG_Refresh(&hiwdg);
	if ( status != HAL_OK)
	{
	    PRINTF("Watchdog refresh failed with status %d\r\n", (uint8_t)status);
	    Error_Handler();
	}

	/* Disable instruction cache prior to internal cache-able memory update */
	status = HAL_ICACHE_Disable();
	if ( status != HAL_OK)
	{
	    PRINTF("Cache disable failed with status %d\r\n", (uint8_t)status);
	    Error_Handler();
	}
	status = HAL_FLASH_Unlock();

	    if ( status != HAL_OK)
	    {
	        PRINTF("Flash unlock failed with status %d\r\n");
	        Error_Handler();
	    }
	if (isBank1 == BANK2)
	{
		currentFlashAddress = 0x08000000;
		bankStartAddress = 0x08000000;
	}

	PRINTF("Erasing at Address %x\r\n",bankStartAddress);

	// Prepare flash erase structure
	EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.Banks = GetBank(bankStartAddress);  // Get the flash bank
	EraseInitStruct.Page = GetPage(bankStartAddress);  // Get the first page to erase
	EraseInitStruct.NbPages = FLASH_PAGE_NB;  // Get the number of pages to erase
	PRINTF("Erasing Bank %d, Starting at Page %d, for number of pages %d\r\n",
	       EraseInitStruct.Banks, EraseInitStruct.Page, EraseInitStruct.NbPages);

	// Erase the flash memory
	status = HAL_FLASHEx_Erase(&EraseInitStruct, &bigData->Flash.Page);
	if ( status != HAL_OK)
	{
	    PRINTF("Flash erase failed with status %d\r\n");
	    Error_Handler();
	}

	PRINTF("Flash erase passed, starting flashing\r\n");


	// maybe due to swap bank always flash to bank2
	//currentFlashAddress = 0x08080000;
	//bankStartAddress = 0x08080000;
	PRINTF("Size of arr: %d\r\n", sizeof(bigData->Flash.Image));
	// Program the flash memory in steps of quad-word (128 bits, 16 bytes)
	for (uint32_t buff = 0; buff < sizeof(bigData->Flash.Image); buff += Quad_Word_Shift)
	{
	    // Ensure that the data pointer is aligned to 16-byte boundaries
	    if ((currentFlashAddress % Quad_Word_Shift) != 0)
	    {
	        PRINTF("Error: Flash address is not 16-byte aligned.\r\n");
	        Error_Handler();  // Handle misalignment case
	    }
	    memcpy( quadWord, &bigData->Flash.Image[buff], Quad_Word_Shift );
	    status = HAL_IWDG_Refresh(&hiwdg);
	    if ( status != HAL_OK)
		{
		    PRINTF("Watchdog refresh failed during flashing with status %d\r\n");
		    Error_Handler();
		}
//HAL_FLASH_Program( FLASH_TYPEPROGRAM_QUADWORD, destination, ( uint32_t ) quadWord );
	    // Program flash
	    status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_QUADWORD, currentFlashAddress,(uint32_t)quadWord);
	    if ( status != HAL_OK)
	    {
	        PRINTF("HAL_FLASH_Program failed at buff = %lu with status %d\r\n", buff);
	        Error_Handler();
	    }
	    else
	    {
	    	if (isVerifyWhileProgram)
	    	{
	            // Check the written value
	            if( memcmp( ( void * ) currentFlashAddress, quadWord, Quad_Word_Shift ) != 0 )
	            {
	                /* Flash content doesn't match SRAM content */
	    	        PRINTF("Memory compare does not match\r\n");
	    	        //Error_Handler();
	            }
	    	}
	    }

	    // Increment to next quad-word address
		currentFlashAddress += Quad_Word_Shift;

	    // Ensure we do not exceed the allowed address range
		// kcs check Max_Page_Shift parameter
	    if (currentFlashAddress > (bankStartAddress + Max_Page_Shift) ||
	        buff >= (OTA_FLASH_SIZE))
	    {
	        break;  // Exit if we reach the limit
	    }

	    // Optionally refresh the watchdog during flashing to avoid reset
	}
	status = HAL_FLASH_Lock();
	if ( status != HAL_OK)
	{
	    PRINTF("Flash lock failed with status %d\r\n");
	    Error_Handler();
	}

	// Re-enable the instruction cache after flash programming
	status = HAL_ICACHE_Enable();
	if ( status != HAL_OK) {
	    PRINTF("Cache enable failed with status %d\r\n");
	    Error_Handler();
	}

	PRINTF("Flash programming completed successfully.\r\n");

	PRINTF("Programming Passed, starting verification\r\n");
	status = HAL_IWDG_Refresh ( &hiwdg );
	if ( status != HAL_OK)
		{
			PRINTF("Watchdog refresh Failed with status %d\r\n");
			Error_Handler ();
		}
	// Re-enable instruction cache
	status = HAL_ICACHE_Enable ();
	if ( status != HAL_OK)
	{
		PRINTF("HAL_ICACHE_Enable Failed with status %d\r\n");
		Error_Handler ();
	}

	// Verify in steps of quad-word (128-bit)

	if (!isVerifyWhileProgram)
	{

	}
	else
		bigData->State |= FLASH_COMPLETE;

	/*Check if there is an issue to program data*/
	//	if (State & FAIL_FLASH)
	//		{while(1){;};}
}
/**
 * @brief  Gets the page of a given FLASH_Address
 * @param  Addr: FLASH_Address of the FLASH Memory
 * @retval The page of a given FLASH_Address
 */
uint32_t GetPage ( uint32_t Addr )
{
	if (Addr < (FLASH_BASE + FLASH_BANK_SIZE))
	{ /* Bank 1 */
		return ((Addr - FLASH_BASE) / FLASH_PAGE_SIZE);
	}
	else
	{ /* Bank 2 */
		return ((Addr - (FLASH_BASE + FLASH_BANK_SIZE)) / FLASH_PAGE_SIZE);
	}
}

/******************************************************
 Flash Function
 Note: returns the bank of a given Address
 ******************************************************/
uint32_t GetBank ( uint32_t Addr )
{
	if (Addr < ADDR_FLASH_PAGE_64)
	{
		return FLASH_BANK_1;
	}
	else if (Addr >= ADDR_FLASH_PAGE_64)
	{
		return FLASH_BANK_2;
	}
	else
	{
		return FLASH_BANK_1;
	}
}
BANK_TYPE getSwapBank(void)
{
	BANK_TYPE currentBank = BANK1;
	FLASH_OBProgramInitTypeDef OptionsBytes;

	HAL_FLASH_Unlock ();
	HAL_FLASH_OB_Unlock ();

	HAL_FLASHEx_OBGetConfig(&OptionsBytes);

	if((OptionsBytes.USERConfig & OB_SWAP_BANK_ENABLE)== OB_SWAP_BANK_ENABLE)
	{
		currentBank = BANK2;
	}

	HAL_FLASH_Lock ();	// Lock the Flash to disable the flash control register access
	return currentBank;

}

#endif  //BOOTLOADER_C_

#ifdef BOOT_CONFIGURE
/******************************************************
		Initial Update Function
	Note: Sets the correct update address and begins
		  to flash update from external ram
******************************************************/
void Upgrade_Firmware(MEM_PTR *Data_Ptr)
{	uint8_t ptr = TEST_MODE;

	if (ptr == Value_PTR_A)
	{	Memory[0] = Value_PTR_A;
		FLASH_Start_Address = ADDR_FLASH_PAGE_0;

		Reflash(Data_Ptr);

		FLASH_Start_Address = ADDR_FLASH_PAGE_0;
	}
	else if (ptr == Value_PTR_B)
	{	Memory[0] = Value_PTR_B;
		FLASH_Start_Address = ADDR_FLASH_PAGE_0 + Bank_Shift;

		Reflash(Data_Ptr);

		FLASH_Start_Address = ADDR_FLASH_PAGE_0 + Bank_Shift;
	}
	else
		{return;}

	FLASH_Address = FLASH_Start_Address;

	Boot_Change();
}
#endif
