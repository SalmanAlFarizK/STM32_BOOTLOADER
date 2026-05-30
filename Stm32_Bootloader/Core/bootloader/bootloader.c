/******************************************************************************
 *		Source File Containing Boot loader Informations and
 *		Drivers (API's) Related to interact with user during
 *		Booting process..
 *
 *		@Author: Salman Al Fariz K
 *		@Date: 	 03/04/2026
 *
 *****************************************************************************/

/******************************************************************************
 * Global Includes.
 *****************************************************************************/
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "stm32f4xx_hal.h"
#include "bootloader.h"

/******************************************************************************
 * Macro Definitions.
 *****************************************************************************/
#define FLASH_SECTOR_2_BASE_ADDR	(0x08019000U)
#define RESET_HANDLER_ADDR_OFFSET	(0x04)
#define BL_UART_TX_TIMEOUT			(0x64)
#define BL_UART_RX_TIMEOUT			(0x64)
#define BL_UART_MAX_RX_LEN			(0x100)
#define BL_BOOT_MSG					("Boot Loader Executing...\r\n")
#define BL_JUMP_MSG					("Starting User Application...\r\n")
#define BL_MAX_CMD_LEN				(0x06)
#define BL_CMD_LEN_INDEX			(0x00)
#define BL_CMD_OPCODE_INDEX			(0x01)
#define BL_CMD_CRC_OFFSET			(0x02)

/******************************************************************************
 * Typedefs.
 *****************************************************************************/
typedef void (*pfnUserAppResetHandler)(void);

/******************************************************************************
 * Enum Definitions.
 *****************************************************************************/
/**
 * @brief : Enumeration representing Bootloader command opcodes.
 */
typedef enum _eBlCmdOpCode_
{
	eBlCmdGetVersion 				= 0x51,
	eBlCmdGetHelp					= 0x52,
	eBlCmdGetChipId					= 0x53,
	eBlCmdGetRdpStatus				= 0x54,
	eBlCmdGoToAddr					= 0x55,
	eBlCmdFlashErase				= 0x56,
	eBlCmdMemWrite					= 0x57,
	eBlCmdEnableReadWriteProtect	= 0x58,
	eBlCmdMemRead					= 0x59,
	eBlCmdReadSectorStatus			= 0x5A,
	eBlCmdOTPRead					= 0x5B,
	eBlCmdDisReadWriteProtyection	= 0x5C,
	eBlCmdMax
} eBlCmdOpCode;

/******************************************************************************
 * Structure Definitions.
 *****************************************************************************/
/**
 * @brief : Structure representing Bootloader command.
 */
typedef struct _BlCmdStruct_
{
	uint8_t  ucBlCmdLen;
	uint8_t  ucBlCmdOpCode;
	uint32_t uiBlCmdCrc;
} BlCmdStruct;

/******************************************************************************
 * Global variable declaration.
 *****************************************************************************/
UART_HandleTypeDef huart2;

/******************************************************************************
 * Static Function Declarations.
 *****************************************************************************/
static void MX_USART2_UART_Init(void);
static void BootLoaderGpioInit(void);
static void BootLoaderUartSendData(uint8_t* pucData, uint16_t uhTxLen);
static void BootLoaderUartReadData(uint8_t* pucData, uint16_t* puhRxLen);
static void BootLoaderJumpUserApp(void);
static eBlError BootLoaderParseCmd(uint8_t* pucCmdBuff, uint16_t uhCmdLen,
		BlCmdStruct* ptBlCmdInfo);
static void BootLoaderActions(void);

/******************************************************************************
 * Function Definitions.
 *****************************************************************************/

/******************************************************************************
 * @brief : Function For Initializing Boot Loader, Initialization
 * 			consist of Initializing uart.
 *
 * @fn : BootLoaderInit();
 *
 * @param[in] : None
 *
 * @param[out] : None.
 *
 * @return : None
 *
 *****************************************************************************/
void BootLoaderInit(void)
{
	/* Initialize the UART. */
	MX_USART2_UART_Init();

	/* Initialize User button GPIO in interrupt mode. */
	BootLoaderGpioInit();

	return;
}

/******************************************************************************
 * @brief : Function For Initializing Boot Loader, Initialization
 * 			consist of Initializing uart.
 *
 * @fn : BootLoaderInit();
 *
 * @param[in] : None
 *
 * @param[out] : None.
 *
 * @return : None
 *
 *****************************************************************************/
eBlError BootLoaderCheckForUpdates(void)
{
	static GPIO_PinState eGpioPrevState = GPIO_PIN_RESET;
	GPIO_PinState eGpioCurrState = GPIO_PIN_RESET;

	eGpioCurrState = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);

	if(GPIO_PIN_SET == eGpioCurrState && GPIO_PIN_RESET == eGpioPrevState)
	{
		BootLoaderActions();
	}
	else
	{
		BootLoaderUartSendData((uint8_t*)BL_JUMP_MSG, sizeof(BL_JUMP_MSG) - 1);

		BootLoaderJumpUserApp();
	}

	eGpioPrevState = eGpioCurrState;

	return 0;
}

/******************************************************************************
 * @brief : Function For Initializing Boot Loader, Initialization
 * 			consist of Initializing uart.
 *
 * @fn : BootLoaderInit();
 *
 * @param[in] : None
 *
 * @param[out] : None.
 *
 * @return : None
 *
 *****************************************************************************/
static void BootLoaderGpioInit(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/*Configure GPIO pin : PA0 */
	GPIO_InitStruct.Pin = GPIO_PIN_0;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	return;
}

/******************************************************************************
 * @brief : Function For Initializing Boot Loader, Initialization
 * 			consist of Initializing uart.
 *
 * @fn : BootLoaderInit();
 *
 * @param[in] : None
 *
 * @param[out] : None.
 *
 * @return : None
 *
 *****************************************************************************/
static void BootLoaderUartSendData(uint8_t* pucData, uint16_t uhTxLen)
{
	if((NULL != pucData)
    && (0 < uhTxLen))
	{
		HAL_UART_Transmit(&huart2, pucData, uhTxLen, BL_UART_TX_TIMEOUT);
	}

	return;
}

/******************************************************************************
 * @brief : Function For Initializing Boot Loader, Initialization
 * 			consist of Initializing uart.
 *
 * @fn : BootLoaderInit();
 *
 * @param[in] : None
 *
 * @param[out] : None.
 *
 * @return : None
 *
 *****************************************************************************/
static void BootLoaderUartReadData(uint8_t* pucData, uint16_t* puhRxLen)
{
	/* Variable initialization. */
	uint16_t uhRxIndex = 0;
	uint32_t uiCurrTick = 0;

	if((NULL != pucData) && (NULL != puhRxLen))
	{
		/* Get current tick value. */
		uiCurrTick = HAL_GetTick();

		/* Recieve byte by byte data through uart. */
		while(((HAL_GetTick() - uiCurrTick) <= BL_UART_RX_TIMEOUT)
	    && (uhRxIndex < BL_UART_MAX_RX_LEN))
		{
			/* Receive data over uart. */
			if(HAL_OK == HAL_UART_Receive(&huart2, pucData + uhRxIndex, 1, 10))
			{
				uhRxIndex++;
			}
		}
	}

	/* Update the received data length to the output reference. */
	*puhRxLen = uhRxIndex;

	return;
}

/******************************************************************************
 * @brief : Function For Initializing Boot Loader, Initialization
 * 			consist of Initializing uart.
 *
 * @fn : BootLoaderInit();
 *
 * @param[in] : None
 *
 * @param[out] : None.
 *
 * @return : None
 *
 *****************************************************************************/
static void BootLoaderJumpUserApp(void)
{
	/* Variable Initialization. */
	pfnUserAppResetHandler pfnAppResetHandler = NULL;
	uint32_t uiMspVal = 0;
	uint32_t uiResetHandlerAddr = 0;

    // 1. Disable interrupts
    __disable_irq();

    // 2. Stop SysTick
    SysTick->CTRL = 0;

    // 3. Clear pending interrupts (important!)
    for (int i = 0; i < 8; i++) {
        NVIC->ICER[i] = 0xFFFFFFFF;
        NVIC->ICPR[i] = 0xFFFFFFFF;
    }

    // 4. Deinit clocks/peripherals
    HAL_DeInit();
    HAL_RCC_DeInit();

    // 5. Set vector table to application
    SCB->VTOR = FLASH_SECTOR_2_BASE_ADDR;

	/* Fetch the MSP value from the base address. */
	uiMspVal = *((volatile uint32_t *)FLASH_SECTOR_2_BASE_ADDR);

	/* Configure the MSP value. */
	__set_MSP(uiMspVal);

	/* Fetch the reset handler address. */
	uiResetHandlerAddr = *((volatile uint32_t *)(FLASH_SECTOR_2_BASE_ADDR +
									RESET_HANDLER_ADDR_OFFSET));

	uiResetHandlerAddr |= 0x01;

	/* Assign the reset handler function pointer to the reset handler address. */
	pfnAppResetHandler = (pfnUserAppResetHandler)uiResetHandlerAddr;

	/* Trigger the reset handler of user application. */
	pfnAppResetHandler();

	return;
}

/******************************************************************************
 * @brief : Function For Initializing Boot Loader, Initialization
 * 			consist of Initializing uart.
 *
 * @fn : BootLoaderInit();
 *
 * @param[in] : None
 *
 * @param[out] : None.
 *
 * @return : None
 *
 *****************************************************************************/
static eBlError BootLoaderParseCmd(uint8_t* pucCmdBuff, uint16_t uhCmdLen,
		BlCmdStruct* ptBlCmdInfo)
{
	/* Variable initialization. */
	eBlError eBlErr = eBlParamError;

	/* Validity check. */
	if((NULL != pucCmdBuff) && (BL_MAX_CMD_LEN <= uhCmdLen)
    && (NULL != ptBlCmdInfo))
	{
		/* Copy the length. */
		ptBlCmdInfo->ucBlCmdLen = pucCmdBuff[BL_CMD_LEN_INDEX];

		/* Copy the opcode. */
		ptBlCmdInfo->ucBlCmdOpCode = pucCmdBuff[BL_CMD_OPCODE_INDEX];

		/* Copy the crc. */
		memcpy(&ptBlCmdInfo->uiBlCmdCrc, pucCmdBuff + BL_CMD_CRC_OFFSET,
				sizeof(uint32_t));

		/* Update the error status. */
		eBlErr = eBlSuccess;
	}

	/* Return the error status. */
	return eBlErr;
}

/******************************************************************************
 * @brief : Function For Initializing Boot Loader, Initialization
 * 			consist of Initializing uart.
 *
 * @fn : BootLoaderInit();
 *
 * @param[in] : None
 *
 * @param[out] : None.
 *
 * @return : None
 *
 *****************************************************************************/
static void BootLoaderActions(void)
{
	/* Send a boogt loader message. */
	BootLoaderUartSendData((uint8_t*)BL_BOOT_MSG, sizeof(BL_BOOT_MSG) - 1);

	return;
}

/******************************************************************************
 * @brief : Function For Initializing Boot Loader, Initialization
 * 			consist of Initializing uart.
 *
 * @fn : BootLoaderInit();
 *
 * @param[in] : None
 *
 * @param[out] : None.
 *
 * @return : None
 *
 *****************************************************************************/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	uint8_t ucData[] = "Button Pressed\r\n";
	uint16_t uhDataLen = strlen((char*)ucData);

	if(GPIO_PIN_0 == GPIO_Pin)
	{
		BootLoaderUartSendData(ucData, uhDataLen);
	}

	return;
}

/******************************************************************************
 * @brief : Function For Initializing Boot Loader UART
 * @fn : BootLoaderInit();
 *
 * @param[in] : None
 *
 * @param[out] : None.
 *
 * @return : None
 *
 *****************************************************************************/
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    //Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}
