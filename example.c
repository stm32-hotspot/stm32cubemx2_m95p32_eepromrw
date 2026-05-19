/**
  ******************************************************************************
  * file           : example.c
  * brief          : example program body
  ******************************************************************************
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "example.h"
#include "m95p32.h"
#include "stdlib.h"
#include <stdio.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define TARGET_ADDR_0000          0x00   /* Base address used in several tests (block 0)          */
#define TARGET_ADDR_0400         0x400  /* Address inside sector used for sector erase test      */
#define TARGET_ADDR_1800        0x1800 /* Address inside block used for block erase test        */

#define SIZE100         100U     /* Generic size for partial page tests                      */
#define SIZE200         200U     /* Number of bytes read/written in page/sector tests        */
#define SIZE256         256U     /* Size used for command parsing buffer                     */
#define SIZE520         520U     /* Size used for status/config write payload                */
#define SIZE600         600U     /* Maximum page payload for command interface               */
#define SIZE1024        1024U    /* Buffer size for 1 KByte operations                       */
#define RXBUFFERSIZE    45U      /* UART RX buffer size for command line input               */

#define M95P32_PAGE_SIZE   512U      /* Page size of M95P32 device                             */
#define M95P32_BLOCK_SIZE  65536U    /* Block size in bytes (64 KBytes)                        */
#define M95P32_BLOCK_COUNT 64U       /* Total number of blocks in the device                   */

/* Private variables ---------------------------------------------------------*/
m95p32_object_t *pM95P320;            /* Pointer to the M95P32 driver object instance           */
uint8_t ReceiveBuff[SIZE1024] = {0};  /* General-purpose receive buffer for data and status     */
uint8_t ReadBuff[SIZE200] = {0};      /* Buffer used for 200-byte page reads                    */
uint8_t data_buff[SIZE1024] = {0};    /* Buffer used for buffer-mode programming tests          */
uint8_t SampleData_Test_Page[SIZE200] = {0}; /* Pattern used in page R/W test                  */
uint8_t aRxBuffer[RXBUFFERSIZE];      /* UART RX buffer for menu/command input                  */
uint8_t pData[SIZE600];               /* Generic data buffer for write/program commands         */

__IO uint8_t UartReady = 0;           /* UART ready flag used by application loop               */
unsigned int add = 0x00;              /* Global address holder used by command handler          */
uint8_t Buff[SIZE256] = {0};          /* Temporary ASCII buffer used during command parsing     */
unsigned int CmdParam[SIZE100] = {0}; /* Parsed command parameters stored as integers           */

volatile uint8_t IT_receive = 0;      /* Flag set by UART interrupt to indicate input type      */
uint8_t User_Choice = 0;              /* Current user menu selection                            */

uint8_t aRxBuffer[RXBUFFERSIZE];      /* Duplicate declaration (same buffer used above)         */

/**
  * ########## Step 1 ##########
  * The init of M95P32 is triggered by the application code
  */
app_status_t app_init(void)
{
  app_status_t return_status = EXEC_STATUS_ERROR;
  /* Retrieve M95P32 object from BSP/driver layer */
  pM95P320 = MX_M95P32_getobject();
  /* Initialize M95P32 device instance 0 */
  if (m95p32_drv_init(pM95P320, MX_M95P32) != 0)
  {
    PRINTF("[ERROR] Step 1: M95P32 EEPROM init error\r\n");
    goto _app_init_exit;
  }
  PRINTF("[INFO] Step 1: M95P32 EEPROM init completed\r\n");

  /* Display intro menu to user over UART */
  IntroScreen();

  /* Start UART interrupt reception for one byte (menu entry) */
  if (HAL_UART_Receive_IT(BASIC_STDIO_UART_GETHANDLE(), (uint8_t *)aRxBuffer, 1) != HAL_OK)
  {
    return_status = EXEC_STATUS_ERROR;
  }
  UartReady = 1;

  /* All initialization steps completed successfully */
  return_status = EXEC_STATUS_INIT_OK;

_app_init_exit:
  return return_status;
}

/**
  * ########## Step 2 ##########
  * Perform Read and Write operations.
  * The values are displayed on the terminal.
  * output: EXEC_STATUS_OK if OK, EXEC_STATUS_ERROR in case of error
  */
app_status_t app_process(void)
{
  app_status_t return_status = EXEC_STATUS_UNKNOWN;
  /* Delegate all EEPROM/command handling to dedicated process function */
  return_status = MX_EEPROM_RW_Process();

  return return_status;
}

/** ########## Step 3 ##########
  * In this example, app_deinit is never called and is provided as a reference only.
  */
app_status_t app_deinit(void)
{
  /* Deinitialize M95P32 driver and related resources */
  if (m95p32_drv_deinit(pM95P320) != 0)
  {
    PRINTF("[ERROR] Step 3: EEPROM deinit error\r\n");
    return EXEC_STATUS_ERROR;
  }

  return EXEC_STATUS_OK;
}

/**
  * @brief  Test M95P32 write-enable and write-disable commands.
  *         Reads status register, enables writes, reads again, disables writes and re-reads.
  * @param  None
  * @retval app_status_t
  */
app_status_t M95P32_TestWriteEnableDisable(void)
{
  printf("[INFO] Test 1: READ STATUS REGISTER\r\n");
  uint8_t regval = 0xFF; /* Local variable holding status register content */

  /* Initial status register read */
  if (m95p32_drv_read_status_register(pM95P320, &regval) != 0)
  {
    printf("Read Status Register ERROR\r\n");
    return EXEC_STATUS_ERROR;
  }

  printf("[INFO] STATUS REGISTER : 0x%x\r\n", regval);

  /* Issue write enable command */
  if (m95p32_drv_write_enable(pM95P320) != 0)
  {
    printf("Write Enable ERROR\r\n");
    return EXEC_STATUS_ERROR;
  }

  printf("[INFO] WRITE ENABLE\r\n");

  /* Read status register after write enable */
  if (m95p32_drv_read_status_register(pM95P320, &regval) != 0)
  {
    printf("Read Status Register ERROR\r\n");
    return EXEC_STATUS_ERROR;
  }
  printf("[INFO] STATUS REGISTER : 0x%x\r\n", regval);

  /* Issue write disable command */
  if (m95p32_drv_write_disable(pM95P320) != 0)
  {
    printf("Read Disable ERROR\r\n");
    return EXEC_STATUS_ERROR;
  }
  printf("[INFO] WRITE DISABLE\r\n");

  /* Final status read after write disable */
  if (m95p32_drv_read_status_register(pM95P320, &regval) != 0)
  {
    printf("Read Status Register ERROR\r\n");
    return EXEC_STATUS_ERROR;
  }
  printf("[INFO] STATUS REGISTER : 0x%x\r\n", regval);

  return EXEC_STATUS_OK;
}

/**
  * @brief  Test page write/read using several SPI modes on M95P32.
  *         Programs first 100 bytes of a page, then verifies content with
  *         Single and Fast Single read commands.
  * @param  None
  * @retval app_status_t
  */
app_status_t M95P32_TestPage(void)
{

  /*
  Test case to verify Page Write and Read
  __________Steps_____________
  *Write Enable
  *Writing first 100 bytes of a page to 0x0f at address: 0x00
  *Verifying if bytes of page are updated to 0x0f
  *Reading 200 Bytes of page at address: 0x00 using Single SPI
  *Reading 200 Bytes of page at address: 0x00 using Fast Read Single SPI
  *Reading 200 Bytes of page at address: 0x00 using Fast Read Dual SPI
  *Reading 200 Bytes of page at address: 0x00 using Fast Read Quad SPI
  */

  unsigned int index;

  /* Prepare page test pattern filled with 0x0F */
  memset(&SampleData_Test_Page, 0x0F, SIZE200);

  printf("\t\t______Test 2 Starts______\r\n");
  printf("\tTest case to verify Page Write and Read\r\n");

  /* Enable write operations for page program */
  if (m95p32_drv_write_enable(pM95P320) != 0)
  {
    PRINTF("Write Enable ERROR\r\n");
    return EXEC_STATUS_ERROR;
  }

  PRINTF(" \n\r Write Page (100 Bytes) at Address 0x00  \n\r");

  /* Program first 100 bytes of page at address 0x00 with pattern 0x0F */
  if (m95p32_drv_page_write(pM95P320, SampleData_Test_Page, TARGET_ADDR_0000, SIZE100) != 0)
  {
    PRINTF("Page Write ERROR\r\n");
    return EXEC_STATUS_ERROR;
  }

  /* Read 200 bytes from start of page using single SPI read */
  if (m95p32_drv_single_read(pM95P320, ReadBuff, TARGET_ADDR_0000, SIZE200) != 0)
  {
    PRINTF("Read ERROR\r\n");
    return EXEC_STATUS_ERROR;
  }

  /* Verify that at least the first 100 bytes match programmed pattern */
  for (index = 0; index < SIZE200; index++)
  {
    if (ReadBuff[index] == SampleData_Test_Page[index])
    {
      continue;
    }
    else
    {
      /* Expected to break after index 99 if remaining bytes are not 0x0F */
      break;
    }
  }

  if (index == SIZE100)
  {
    printf("\n\n\r Write Page				: Passed\r\n");
  }
  else
  {
    printf("\n\n\r Write Page				: Failed\r\n");
    return EXEC_STATUS_ERROR;
  }
  printf("\n");
  memset(&ReadBuff, 0x00, SIZE200);

  /* Read 200 bytes using standard single read for display and verification */
  if (m95p32_drv_single_read(pM95P320, ReadBuff, TARGET_ADDR_0000, SIZE200) != 0)
  {
    PRINTF("Read ERROR after page write\r\n");
    return EXEC_STATUS_ERROR;
  }

  for (index = 0; index < SIZE200; index++)
  {
    if (ReadBuff[index] == SampleData_Test_Page[index])
    {
      continue;
    }
    else
    {
      break;
    }
  }
  if (index == SIZE100)
  {
    printf("\n\r Read data in Single SPI		: Passed\r\n");
    for (index = 0; index < SIZE200; index++)
    {
      printf("  %2.2x", ReadBuff[index]);
    }
  }
  else
  {
    printf("\n\r Read data in Single SPI		: Failed\r\n");
    return EXEC_STATUS_ERROR;
  }
  printf("\n");
  memset(&ReadBuff, 0x00, SIZE200);

  /* Read 200 bytes using Fast Read Single SPI command */
  if (m95p32_drv_fast_read(pM95P320, ReadBuff, TARGET_ADDR_0000, SIZE200) != 0)
  {
    PRINTF("Fast Read ERROR after page write\r\n");
    return EXEC_STATUS_ERROR;

  }

  for (index = 0; index < SIZE200; index++)
  {
    if (ReadBuff[index] == SampleData_Test_Page[index])
    {
      continue;
    }
    else
    {
      break;
    }
  }

  if (index == SIZE100)
  {
    printf("\n\r Read data Fast in Single SPI		: Passed\r\n");
    for (index = 0; index < SIZE200; index++)
    {
      printf("  %2.2x", ReadBuff[index]);
    }
  }
  else
  {
    printf("\n\r Read data Fast in Single SPI		: Failed\r\n");
    return EXEC_STATUS_ERROR;
  }

  printf("\n");
  memset(&ReadBuff, 0x00, SIZE200);
  return EXEC_STATUS_OK;

}

/**
  * @brief  Test page erase on M95P32.
  *         Erases one page and verifies that all bytes are 0xFF.
  * @param  None
  * @retval app_status_t
  */
app_status_t M95P32_TestPageErase(void)
{

  /*
  Test Case to Verify Page Erase
  __________Steps_____________
  *Write Enable
  *Erasing page (512 Byte) at address 0x00
  *Verifying that after Page erase all bytes are at their default value of 0xff
  */

  uint32_t index;

  uint8_t a_rcvbuff_page_erase[SIZE200] = {0}; /* Local buffer for page erase verification */

  printf("\n\t\t______Test 3 Starts______\r\n\n");
  printf("\t  Test Case to verify Page Erase\r\n");

  /* Enable write operations prior to erase command */
  if (m95p32_drv_write_enable(pM95P320) != 0)
  {
    PRINTF("Write Enable ERROR\r\n");
    return EXEC_STATUS_ERROR;

  }

  PRINTF("\n *PAGE_ERASE* at Address 0x00 \r\n");
  /* Erase page starting at address 0x00 */
  if (m95p32_drv_page_erase(pM95P320, TARGET_ADDR_0000) != 0)
  {
    PRINTF("Page Erase ERROR\r\n");
    return EXEC_STATUS_ERROR;
  }

  /* Read first 200 bytes of the erased page */
  if (m95p32_drv_single_read(pM95P320, a_rcvbuff_page_erase, TARGET_ADDR_0000, SIZE200) != 0)
  {
    PRINTF("Read ERROR after page erase\r\n");
    return EXEC_STATUS_ERROR;
  }

  /* Verify all read bytes equal 0xFF */
  for (index = 0; index < SIZE200; index++)
  {

    if (a_rcvbuff_page_erase[index] != 0xFFU)
    {
      return EXEC_STATUS_ERROR;
    }

  }

  printf(" Data after Page Erase \r\n");
  for (index = 0; index < SIZE200; index++)
  {
    printf("  %2.2x", a_rcvbuff_page_erase[index]);
  }
  printf("\n");
  return EXEC_STATUS_OK;
}

/**
  * @brief  Test sector erase on M95P32.
  *         Programs a page in a sector, erases the sector, then verifies bytes are 0xFF.
  * @param  None
  * @retval app_status_t
  */
app_status_t M95P32_TestSectorErase(void)
{
  /*
  Test Case to Verify Sector Erase
  __________Steps_____________
  *Write Enable
  *Writing first 100 bytes of a page to 0x0f at address: 0x400 (Page Number-3)
  *Write Enable
  *Erasing sector (4Kbyte = 8 Pages) at address 0x00*
  *Verifying that after sector erase all bytes of 8 pages are at their default value of 0xff
  */
  printf("\n\t\t______Test 4 Starts______\r\n");
  printf("\t  Test Case to verify Sector Erase\r\n");

  uint32_t index;

  uint8_t a_rcvbuff_sector_erase[SIZE200] = {0}; /* Buffer to verify data after sector erase   */
  uint8_t SampleData[SIZE200] = {0};             /* Pattern buffer for initial write           */

  /* Enable write access for initial data programming */
  if (m95p32_drv_write_enable(pM95P320) != 0)
  {
    PRINTF("Write Enable ERROR\r\n");
    return EXEC_STATUS_ERROR;

  }
  PRINTF("\n *Sector_ERASE* at Address 0x00 \r\n");

  /* Fill sample data with 0x0F and write one page within target sector */
  memset(&SampleData, 0x0F, SIZE200);

  if (m95p32_drv_page_write(pM95P320, SampleData, TARGET_ADDR_0400, SIZE200) != 0)
  {
    PRINTF("Page Write ERROR\r\n");
    return EXEC_STATUS_ERROR;
  }

  /* Enable write again for the erase operation */
  if (m95p32_drv_write_enable(pM95P320) != 0)
  {
    PRINTF("Write Enable ERROR\r\n");
    return EXEC_STATUS_ERROR;
  }

  /* Erase sector that contains address TARGET_ADDR_0400 */
  if (m95p32_drv_sector_erase(pM95P320, TARGET_ADDR_0400) != 0)
  {
    PRINTF("Sector Erase ERROR\r\n");
    return EXEC_STATUS_ERROR;
  }

  /* Read part of previously programmed area to confirm erase */
  if (m95p32_drv_single_read(pM95P320, a_rcvbuff_sector_erase, TARGET_ADDR_0400, SIZE200) != 0)
  {
    PRINTF("Read ERROR\r\n");
    return EXEC_STATUS_ERROR;
  }

  for (index = 0; index < SIZE200; index++)
  {

    if (a_rcvbuff_sector_erase[index] != 0xFFU)
    {
      return EXEC_STATUS_ERROR;
    }
  }

  printf(" Data after Sector Erase \r\n");
  for (index = 0; index < SIZE200; index++)
  {
    printf("  %2.2x", a_rcvbuff_sector_erase[index]);
  }
  printf("\n");
  return EXEC_STATUS_OK;

}

/**
  * @brief  Test block erase on M95P32.
  *         Programs part of a block, erases that block, then checks bytes are 0xFF.
  * @param  None
  * @retval app_status_t
  */
app_status_t M95P32_TestBlockErase(void)
{
  /*
  Test Case to Verify Block Erase
  __________Steps_____________
  *Write Enable
  *Writing first 100 bytes of a page to 0x0f at address: 0x1800 (Page Number-10)
  *Write Enable
  *Erasing Block (64Kbyte = 128 Pages) at address: 0x00
  *Verifying that after block erase all bytes of 128 pages are at their default value of 0xff
  */

  printf("\n\t\t______Test 5 Starts______\r\n\n");
  printf("\t  Test Case to verify Block Erase\r\n");

  uint32_t index;

  uint8_t a_rcvbuff_block_erase[SIZE200] = {0}; /* Buffer to verify data after block erase     */
  uint8_t SampleData[SIZE200] = {0};            /* Pattern buffer used for initial block write */

  /* Enable write for initial data programming */
  if (m95p32_drv_write_enable(pM95P320) != 0)
  {
    PRINTF("Write Enable ERROR\r\n");
    return EXEC_STATUS_ERROR;
  }
  PRINTF(" *Block_ERASE* at Address 0x00 \r\n");

  /* Program pattern (0x0F) into an address inside block (0x1800) */
  memset(&SampleData, 0x0F, 0x64);

  if (m95p32_drv_page_write(pM95P320, SampleData, TARGET_ADDR_1800, SIZE200) != 0)
  {
    PRINTF("Page Write ERROR\r\n");
    return EXEC_STATUS_ERROR;
  }

  /* Enable write for block erase */
  if (m95p32_drv_write_enable(pM95P320) != 0)
  {
    PRINTF("Write Enable ERROR\r\n");
    return EXEC_STATUS_ERROR;
  }

  /* Use sector erase on address 0x1800 (sample code uses sector_erase, though comment says block) */
  if (m95p32_drv_sector_erase(pM95P320, TARGET_ADDR_1800) != 0)
  {
    PRINTF("Sector Erase ERROR\r\n");
    return EXEC_STATUS_ERROR;
  }

  /* Read part of this block to confirm erase */
  if (m95p32_drv_single_read(pM95P320, a_rcvbuff_block_erase, TARGET_ADDR_1800, SIZE200) != 0)
  {
    PRINTF("Read ERROR\r\n");
    return EXEC_STATUS_ERROR;
  }

  for (index = 0; index < SIZE200; index++)
  {

    if (a_rcvbuff_block_erase[index] != 0xFFU)
    {
      return EXEC_STATUS_ERROR;
    }
  }

  printf(" Data after Block Erase \r\n");
  for (index = 0; index < SIZE200; index++)
  {
    printf("  %2.2x", a_rcvbuff_block_erase[index]);
  }
  return EXEC_STATUS_OK;
}

/**
  * @brief  Test page program in buffer mode on M95P32.
  *         Programs 1024 bytes using buffer mode, verifies contents and erases sector.
  * @param  None
  * @retval app_status_t
  */
app_status_t M95P32_TestBufferMode(void)
{
  /*
  Test Case to Verify Buffer Mode
  __________Steps_____________
  *Write Enable
  *Write volatile Register to Enable Buffer Mode (0x81 02)
  *Write Enable
  *Programming 1024 bytes of a page to 0x09 starting from address: 0x00
  *Write Volatile Register to exit Buffer Mode
  *Verifying that after Page Program in buffer mode the 1024 bytes from address 0x00 have value 0x09
  */

  printf("\n\t\t______Test 6 Starts______\r\n\n");
  printf("\t  Test Case to verify Page Program in Buffer Mode\r\n");

  uint32_t index;

  /* Fill buffer with pattern 0x09 and use buffer-load API to program it */
  memset(&data_buff, 0x09, SIZE1024);
  PageProgram_with_BufferLoad(pM95P320, data_buff,  0x00,  SIZE1024);

  /* Clear local buffer and read back the data for verification */
  memset(&data_buff, 0x00, SIZE1024);

  if (m95p32_drv_single_read(pM95P320, data_buff, 0x00, SIZE1024) != 0)
  {
    PRINTF("Read ERROR\r\n");
    return EXEC_STATUS_ERROR;
  }

  /* Ensure all bytes equal 0x09 */
  for (index = 0x00; index < SIZE1024; index++)
  {

    if (data_buff[index] != 0x09U)
    {
      return EXEC_STATUS_ERROR;
    }
  }
  printf(" \nData after Programming 1024 bytes starting from address 0x00 in buffer mode \r\n");
  for (index = 0U; index < SIZE1024; index++)
  {
    printf("  %2.2x", data_buff[index]);
  }

  /* Clean up: erase sector at base address */
  if (m95p32_drv_write_enable(pM95P320) != 0)
  {
    PRINTF("Write Enable ERROR\r\n");
    return EXEC_STATUS_ERROR;
  }

  if (m95p32_drv_sector_erase(pM95P320, TARGET_ADDR_0000) != 0)
  {
    PRINTF("Sector Erase ERROR\r\n");
    return EXEC_STATUS_ERROR;
  }

  return EXEC_STATUS_OK;
}


/**
  * @brief  Perform page program using buffer mode on M95P32.
  *         Handles erase checks, protection checks, BUFFEN configuration,
  *         buffer loading, final verification and ECC/safety checks.
  * @param  pObj           Pointer to M95P32 object instance
  * @param  pData          Pointer to user data buffer to program
  * @param  start_prog_add Start address in memory
  * @param  nb_byte        Number of bytes to program
  * @retval m95p32_buff_load_status
  */
m95p32_buff_load_status PageProgram_with_BufferLoad(m95p32_object_t *pObj, uint8_t *pData, uint32_t start_prog_add,
                                                    uint32_t nb_byte)
{
  /*
   * Buffer-load program flow (high-level algorithm):
   * 1) Normalize user range to 16-byte boundaries so safety checks are deterministic.
   * 2) Read Status Register and decode BP/TB bits to reject writes in protected regions.
   * 3) Validate erase state across all pages touched by the request.
   * 4) Read safety register before programming to report pre-existing ECC events.
   * 5) Enable BUFFEN (volatile register bit[1]) and keep Write Enable asserted for sequence.
   * 6) Program page fragments until all bytes are transferred.
   * 7) Poll WIP/BUFLD completion bits to avoid overlapping operations.
   * 8) Check safety register for program/erase anomalies.
   * 9) Disable BUFFEN to restore standard operating mode.
   * 10) Read back and compare programmed data with user payload.
   * 11) Re-check ECC flags after verify so integrity status is explicit to users.
   *
   * Addressing notes used by this routine:
   * - start_prog_add is the first byte of user payload in memory array space.
   * - end_prog_add is computed inclusive to simplify overlap comparisons.
   * - targetAddress advances as chunks are programmed/read within each page.
   * - offset tracks current position inside a page boundary.
   *
   * Safety policy summary:
   * - If any BP/TB combination marks the request range as protected, abort early.
   * - If erase-state validation finds non-0xFF data, abort with NOT_ERASED status.
   * - If safety register reports program/erase faults, return dedicated error code.
   *
   * Why this function is strict:
   * The sample demonstrates robust production-like sequencing where each phase
   * checks hardware state before proceeding, so test output remains diagnostic.
   */
  uint32_t result;
  uint32_t start_erase_add;
  uint32_t end_erase_add;
  uint32_t end_prog_add;
  uint32_t top_add_secure;
  uint32_t bot_add_secure;
  uint8_t status_reg_val;
  uint8_t volatile_reg_val;
  uint16_t bytesToWrite;
  uint16_t bytesToRead;
  uint32_t remainingSize;
  uint16_t offset;
  unsigned int targetAddress;
  uint8_t *pbuffdata;
  uint16_t index;
  uint8_t BP_bits;
  uint8_t TB_bit;

  /* Erase Check:
     Compute aligned erase start and end addresses for the region to program.
  */

  /* Compute start_erase_add as 16-byte aligned address <= start_prog_add */
  result = start_prog_add % 16;
  if (result == 0) /* Address is word aligned */
  {
    start_erase_add = start_prog_add;
  }
  else
  {
    start_erase_add = start_prog_add - result;
  }

  /* Compute end_erase_add as 16-byte aligned address >= end_prog_add */
  end_prog_add = start_prog_add + nb_byte - 1;
  result = end_prog_add % 15;

  if (result == 0) /* Address is word aligned */
  {
    end_erase_add = end_prog_add;
  }

  else
  {
    end_erase_add = end_prog_add + (16 - result);
  }

  /* Read status register to check if target region is protected */
  if (m95p32_drv_read_status_register(pObj, &status_reg_val) != 0)
  {
    PRINTF("Read Status Register ERROR\r\n");
    return M95P32_EXEC_ERROR;
  }

  /* Decode block protection and top/bottom configuration */
  BP_bits = (status_reg_val & 0x1C) >> 2;
  TB_bit = (status_reg_val & 0x40) >> 6;

  if (BP_bits == 0)
  {
    /* No protection configured, programming is allowed */
  }
  else if (BP_bits == 7)
  {
    /* Memory fully protected, abort programming */
    return M95P32_MEMORY_PROTECTED;
  }
  else if ((BP_bits > 0) && (BP_bits < 7) && (TB_bit == 0))
  {
    /* Memory protected from top; compute first protected address from bottom */
    bot_add_secure = (M95P32_BLOCK_SIZE * (M95P32_BLOCK_COUNT - (2 ^ (BP_bits - 1))));
    if (end_erase_add >= bot_add_secure)
    {
      /* Region to program overlaps protected area; abort */
      return M95P32_MEMORY_PROTECTED;
    }

  }
  else if ((BP_bits > 0) && (BP_bits < 7) && (TB_bit == 1))
  {
    /* Memory protected from bottom; compute last protected address at top */
    top_add_secure = (M95P32_BLOCK_SIZE * (2 ^ (BP_bits - 1))) - 1;
    if (start_erase_add <= top_add_secure)
    {
      /* Region to program overlaps protected area; abort */
      return M95P32_MEMORY_PROTECTED;
    }

  }
  else
  {
    /* No additional action for other combinations */
  }

  /* Check if the targeted area is already erased (all bytes 0xFF) */

  remainingSize = nb_byte;
  offset = start_prog_add % M95P32_PAGESIZE;
  targetAddress = start_prog_add;

  /* Iterate through affected pages and check erase condition */
  while (remainingSize > 0)
  {
    bytesToRead = (remainingSize < M95P32_PAGE_SIZE - offset) ? remainingSize : (M95P32_PAGE_SIZE - offset);

    memset(&ReceiveBuff, 0x00, SIZE1024);
    if (m95p32_drv_single_read(pObj, ReceiveBuff, targetAddress, bytesToRead) != 0)
    {
      PRINTF("Read ERROR\r\n");
      return M95P32_EXEC_ERROR;
    }

    /* Verify that all bytes are 0xFF */
    for (index = 0; (ReceiveBuff[index] == 0xff) && (index < bytesToRead); index++)
    {
      targetAddress++;
    }
    if (index == bytesToRead)
    {
      printf("Data is in erased state. OK.\r\n");
    }
    else
    {
      printf("Data is NOT in erased state. Abort!\r\n");
      return M95P32_MEMORY_NOT_ERASED;
    }

    remainingSize -= bytesToRead;
    offset = targetAddress % M95P32_PAGE_SIZE;
  }

  /* Read safety register to check that no ECC event occurred so far */
  memset(&ReceiveBuff, 0x00, SIZE1024);
  if (m95p32_drv_read_config_safety_register(pObj, ReceiveBuff, 2) != 0)
  {
    PRINTF("Read Configuration and Safety Register ERROR\r\n");
    return M95P32_SAFETY_REGISTER_ERROR;
  }

  if ((ReceiveBuff[1] & 0x0F) == 0)
  {
    printf("NO ECC correction detected. OK.\r\n");
  }
  else
  {
    printf("ECC correction detected !\r\n");
  }

  /* Inform user if start or end address is not page boundary aligned */
  if (start_prog_add % M95P32_PAGE_SIZE != 0)
  {
    printf("First Page program is managed outside of an array.\r\n");
  }

  if (end_prog_add % 511 != 0)
  {
    printf("Last Page program is managed outside of an array.\r\n");
  }

  /* Setup buffer mode:
     - Enable write,
     - Set BUFFEN bit in volatile register.
  */

  if (m95p32_drv_write_enable(pObj) != 0)
  {
    PRINTF("Write Enable ERROR\r\n");
    return M95P32_EXEC_ERROR;
  }

  /* BUFFEN=1 enables buffer mode (value 0x02) */
  m95p32_drv_write_volatile_register(pObj, 0x02);

  /* Program in buffer mode:
     Write data page by page while monitoring BUFLD bit.
  */

  remainingSize = nb_byte;
  offset = start_prog_add % M95P32_PAGESIZE;
  targetAddress = start_prog_add;
  pbuffdata = pData;

  /* Single write-enable before entire buffer programming sequence */
  if (m95p32_drv_write_enable(pObj) != 0)
  {
    PRINTF("Write Enable ERROR\r\n");
    return M95P32_EXEC_ERROR;
  }

  /* Iterate over pages and perform page program with buffer load */
  while (remainingSize > 0)
  {
    bytesToWrite = (remainingSize < M95P32_PAGE_SIZE - offset) ? remainingSize : (M95P32_PAGE_SIZE - offset);

    if (m95p32_drv_page_prog(pObj, pbuffdata, targetAddress, bytesToWrite) != 0)
    {
      PRINTF("Page Program ERROR\r\n");
      return M95P32_EXEC_ERROR;
    }

    /* Wait until BUFLD bit returns to 0 (buffer load completed) */
    do
    {
      m95p32_drv_read_volatile_register(pObj, &volatile_reg_val);
    } while ((volatile_reg_val & 0x01) != 0);

    pbuffdata += bytesToWrite;
    targetAddress += bytesToWrite;
    remainingSize -= bytesToWrite;
    offset = targetAddress % M95P32_PAGE_SIZE;
  }

  /* Final polling on WIP bit to ensure programming finished */
  do
  {
    if (m95p32_drv_read_status_register(pObj, &status_reg_val) != 0)
    {
      PRINTF("Read Status Register ERROR\r\n");
      return M95P32_EXEC_ERROR;
    }
  } while ((status_reg_val & 0x01) != 0);

  /* Read safety register to ensure no program/erase error flags */
  memset(&ReceiveBuff, 0x00, SIZE1024);
  if (m95p32_drv_read_config_safety_register(pObj, ReceiveBuff, 2) != 0)
  {
    PRINTF("Read Configuration and Safety Register ERROR\r\n");
    return M95P32_SAFETY_REGISTER_ERROR;
  }

  if ((ReceiveBuff[1] & 0x70) == 0)
  {
    printf("Safety Register status OK.\r\n");
  }
  else
  {
    printf("Safety Register status NOT OK.\r\n");
    return M95P32_SAFETY_REGISTER_ERROR; /* Abort due to safety flag */
  }

  /* Exit buffer mode:
     - Enable write,
     - Clear BUFFEN bit via volatile register.
  */

  if (m95p32_drv_write_enable(pObj) != 0)
  {
    PRINTF("Write Enable ERROR\r\n");
    return M95P32_EXEC_ERROR;
  }

  /* Clear BUFFEN bit (write 0x01; BUFFEN is bit 1) */
  if (m95p32_drv_write_volatile_register(pObj, 0x01) != 0)
  {
    PRINTF("Write Volatile Register ERROR\r\n");
    return M95P32_EXEC_ERROR;
  }

  /* Confirm BUFFEN bit is cleared */
  if (m95p32_drv_read_volatile_register(pObj, &volatile_reg_val) != 0)
  {
    PRINTF("Read Volatile Register ERROR\r\n");
    return M95P32_EXEC_ERROR;
  }

  if ((volatile_reg_val & 0x02) == 0)
  {
    printf("BUFFEN bit is 0. OK.\r\n");
  }
  else
  {
    printf("BUFFEN bit is 1. NOT OK !\r\n");
  }

  /* Verify programmed data using standard read operations */

  remainingSize = nb_byte;
  offset = start_prog_add % M95P32_PAGESIZE;
  targetAddress = start_prog_add;
  pbuffdata = pData;

  while (remainingSize > 0)
  {
    bytesToRead = (remainingSize < M95P32_PAGE_SIZE - offset) ? remainingSize : (M95P32_PAGE_SIZE - offset);

    memset(&ReceiveBuff, 0x00, SIZE1024);
    if (m95p32_drv_single_read(pObj, ReceiveBuff, targetAddress, bytesToRead) != 0)
    {
      PRINTF("Read ERROR\r\n");
      return M95P32_EXEC_ERROR;
    }

    /* Compare programmed data with original buffer */
    for (index = 0; (pbuffdata[index] == ReceiveBuff[index]) && (index < bytesToRead); index++)
    {
      targetAddress++;
    }
    if (index == bytesToRead)
    {
      printf("Data programmed correctly.\r\n");
    }
    else
    {
      printf("Data programmed is NOT correct!\r\n");
    }

    pbuffdata += bytesToRead;
    remainingSize -= bytesToRead;
    offset = targetAddress % M95P32_PAGE_SIZE;
  }

  /* Final ECC/safety check after verification */
  memset(&ReceiveBuff, 0x00, SIZE1024);
  if (m95p32_drv_read_config_safety_register(pObj, ReceiveBuff, 2) != 0)
  {
    PRINTF("Read Configuration and Safety Register ERROR\r\n");
    return M95P32_SAFETY_REGISTER_ERROR;
  }
  if ((ReceiveBuff[1] & 0x0F) == 0)
  {
    printf("NO ECC correction detected. OK.\r\n");
  }
  else
  {
    printf("ECC correction detected !\r\n");
  }
  return M95P32_STATUS_OK;

}

/**
  * @brief  Main EEPROM read/write process called from application loop.
  *         Handles:
  *         - Command parsing over UART
  *         - Direct command execution (IT_receive == 1)
  *         - User scenario menu (IT_receive == 2)
  *         - Tips and tricks menu (IT_receive == 3)
  *         - Automatic regression test suite (IT_receive == 4)
  * @param  None
  * @retval app_status_t
  */
app_status_t MX_EEPROM_RW_Process(void)
{
  /*
   * Runtime state machine summary:
   * - IT_receive == 1: command-line interpreter path.
   * - IT_receive == 2: user scenario menu path.
   * - IT_receive == 3: specific behaviour/tips menu path.
   * - IT_receive == 4: regression test sequence path.
   *
   * The callback sets IT_receive based on the latest keypress. This function
   * consumes the selected state, executes corresponding operations, clears
   * temporary buffers, and then arms UART interrupt reception again.
   *
   * Design goals:
   * - Keep the CLI non-blocking from the IRQ perspective.
   * - Preserve deterministic behavior after each command.
   * - Re-prompt user after each completed operation.
   */
  int32_t status;
  uint8_t status_reg_val;
  uint8_t volatile_reg_val;
  unsigned int index;
  uint32_t index_2;
  uint32_t index_3;
  app_status_t ret = EXEC_STATUS_OK;

  /* IT_receive == 1: raw command line entered by user */
  if (IT_receive == 1U)
  {
    /*
     * Command-line path notes:
     * - Tokens are parsed as hexadecimal values.
     * - CmdParam[0] is command opcode or menu character.
     * - Remaining CmdParam entries are positional parameters.
     * - Buffers are reset after dispatch to avoid stale data reuse.
     */
    IT_receive = 0;
    memset(aRxBuffer, 0, RXBUFFERSIZE);

    /* Receive command line until CR/LF or timeout */
    HAL_UART_Receive_Until_CR(BASIC_STDIO_UART_GETHANDLE(), (uint8_t *)aRxBuffer, 30, 0xffff);

    /* Insert terminator after CR/LF so that parsing stops cleanly */
    for (int i = 0; i < RXBUFFERSIZE; ++i)
    {
      if (aRxBuffer[i] == 0x0D || aRxBuffer[i] == 0x0A)
      {
        aRxBuffer[i + 1] = 0;
        break;
      }
    }

    /* Prepare temporary ASCII buffer for token extraction */
    memset(&Buff, 0x00, SIZE256);

    index = 0;
    index_2 = 0;
    index_3 = 0;
    /* Parse command line, splitting parameters on spaces and CR/LF */
    while (aRxBuffer[index] != 0U)
    {
      if (aRxBuffer[index] == 0x20U)
      {
        if (index_2 > 0)
        {
          CmdParam[index_3++] = (unsigned int)strtol((char *)Buff, NULL, 16);
          memset(Buff, 0x00, SIZE256);
          index_2 = 0;
        }
      }
      else if ((aRxBuffer[index] == 0x0AU) || (aRxBuffer[index] == 0x0DU))
      {
        if (index_2 > 0)
        {
          CmdParam[index_3++] = (unsigned int)strtol((char *)Buff, NULL, 16);
        }
        memset(Buff, 0x00, SIZE256);
        break;
      }
      else
      {
        if (index_2 < SIZE256 - 1)
        {
          Buff[index_2++] = aRxBuffer[index];
        }
      }
      index++;
    }

    memset(&Buff, 0x00, SIZE256);

    /* Execute command based on CmdParam[0] (opcode or character) */
    switch (CmdParam[0])
    {

      /* Menu control command ('R'): return to top-level UI. */

      case 'R':
        /* Reset to main menu */
        User_Choice = 0;
        IntroScreen();
        break;

      /* Erase command family: sector, block, chip, page. */

      case CMD_ERASE_SECTOR:
        if (CmdParam[2] != 0U)
        {
          printf("Erase Sector expects 0 parameter. Command not executed !\r\n");
          break;
        }
        printf("Erase Sector Add:  0x%6.6x \r\n", CmdParam[1]);
        m95p32_drv_sector_erase(pM95P320, CmdParam[1]);
        break;

      case CMD_ERASE_BLOCK:
        if (CmdParam[2] != 0U)
        {
          printf("Erase Block expects 0 parameter. Command not executed !\r\n");
          break;
        }
        printf("Erase Block Add:  0x%6.6x \r\n", CmdParam[1]);
        m95p32_drv_block_erase(pM95P320, CmdParam[1]);
        break;

      case CMD_ERASE_CHIP:
        if (CmdParam[1] != 0U)
        {
          printf(" Erase Chip expects 0 parameter. Command not executed !\r\n");
          break;
        }
        printf("Erase Chip \r\n");
        m95p32_drv_chip_erase(pM95P320);
        break;

      case CMD_ERASE_PAGE:
        if (CmdParam[2] != 0U)
        {
          printf("Erase Page expects 0 parameter. Command not executed !\r\n");
          break;
        }
        printf("Erase Page Add:  0x%6.6x\r\n", CmdParam[1]);
        m95p32_drv_page_erase(pM95P320, CmdParam[1]);
        break;

      /* Program/write command family: page program and page write. */

      case CMD_PROG_PAGE:
        if (CmdParam[2] <= SIZE600)
        {
          /* Initialize temporary data buffer with constant value */
          memset(&pData, 0x00, SIZE600);
          add = CmdParam[1];
          for (index = 0; index < CmdParam[2]; index++)
          {
            pData[index] = (uint8_t)CmdParam[3];
          }

          /* Check current buffer mode state through volatile register */
          m95p32_drv_read_volatile_register(pM95P320, &volatile_reg_val);

          if (volatile_reg_val == 0x01U)
          {
            /* Standard page program mode */
            printf("Prog Page Add:  0x%6.6x \r\n ", CmdParam[1]);
            m95p32_drv_page_prog(pM95P320, pData, add, index);
          }
          else
          {
            /* Buffer mode program using helper function */
            printf("Prog Page in Buffer Mode Add:  0x%6.6x \r\n", CmdParam[1]);
            PageProgram_with_BufferLoad(pM95P320, pData, add, index);
          }
        }
        else
        {
          printf("Requested size is greater than configured buffer, kindly check MCU application.\r\n");
        }
        break;

      case CMD_WRITE_PAGE:
        if (CmdParam[2] <= SIZE600)
        {
          /* Prepare data payload with constant value CmdParam[3] */
          memset(&pData, 0x00, SIZE600);
          add = CmdParam[1];
          for (index = 0; index < CmdParam[2]; index++)
          {
            pData[index] = (uint8_t)CmdParam[3];
          }
          printf("Write Page Add:  0x%6.6x \r\n", CmdParam[1]);
          m95p32_drv_page_write(pM95P320, pData, add, index);
        }
        else
        {
          printf("Requested size is greater than configured buffer, kindly check MCU application.\r\n");
        }
        break;

      /* Write-enable control command family: WREN / WRDI. */

      case CMD_WREN:
        if (CmdParam[1] != 0U)
        {
          printf("WRITE_ENABLE expects 0 parameter. Command not executed !\r\n");
          break;
        }
        printf("WRITE_ENABLE \r\n");
        m95p32_drv_write_enable(pM95P320);
        break;

      case CMD_WRDI:
        if (CmdParam[1] != 0U)
        {
          printf("WRITE_DISABLE expects 0 parameter. Command not executed !\r\n");
          break;
        }
        printf("WRITE_DISABLE \r\n");
        m95p32_drv_write_disable(pM95P320);
        break;

      /* Register access command family: status, config/safety, volatile. */

      case CMD_READ_STATUS_REG:
        if (CmdParam[1] != 0U)
        {
          printf("CMD_READ_STATUS_REG expects 0 parameter. Command not executed !\r\n");
          break;
        }
        status = m95p32_drv_read_status_register(pM95P320, &status_reg_val);
        if (status != 0)
        {
          printf("__ERROR in READ_STATUS_REG__\r\n");
          ret = EXEC_STATUS_ERROR;
        }
        else
        {
          printf("STATUS_REG value: 0x%2.2x\r\n", status_reg_val);
        }
        break;

      case CMD_READ_CONF_SAFETY_REG:
        if (CmdParam[2] != 0U)
        {
          printf("CMD_READ_CONF_SAFETY_REG expects 1 parameter. Command not executed !\n");
          break;
        }
        printf("CMD_READ_CONF_SAFETY_REG\r\n");
        if (CmdParam[1] == 1U)
        {
          m95p32_drv_read_config_safety_register(pM95P320, ReceiveBuff, 1);
          printf("Configuration Register value: 0x%2.2x\n", ReceiveBuff[0]);
        }
        else
        {
          m95p32_drv_read_config_safety_register(pM95P320, ReceiveBuff, 2);
          printf("Configuration Register value: 0x%2.2x \r\nSafety Register value: 0x%2.2x \r\n",
                 ReceiveBuff[0], ReceiveBuff[1]);
        }
        break;

      case CMD_CLEAR_SAFETY_REG:
        if (CmdParam[1] != 0U)
        {
          printf("CMD_CLEAR_SAFETY_REG expects 0 parameter. Command not executed !\r\n");
          break;
        }
        printf("CMD_CLEAR_SAFETY_REG\r\n");
        m95p32_drv_clear_safety_flag(pM95P320);
        break;

      case CMD_READ_VOLATILE_REG:
        if (CmdParam[1] != 0U)
        {
          printf("CMD_READ_VOLATILE_REG expects 0 parameter. Command not executed !\r\n");
          break;
        }
        m95p32_drv_read_volatile_register(pM95P320, &volatile_reg_val);
        printf("Volatile Register value: 0x%2.2x\r\n", volatile_reg_val);
        break;

      case CMD_WRITE_VOLATILE_REG:
        if (CmdParam[2] != 0U)
        {
          printf("CMD_WRITE_VOLATILE_REG expects 1 parameter only. Command not executed !\r\n");
          break;
        }
        printf("CMD_WRITE_VOLATILE_REG with value : 0x%2.2x\r\n", CmdParam[1]);
        m95p32_drv_write_volatile_register(pM95P320, (uint8_t)CmdParam[1]);
        break;

      case CMD_WRITE_STATUS_CONF_REG:
        if (CmdParam[3] != 0U)
        {
          printf("CMD_WRITE_STATUS_CONF_REG expects 1 or 2 parameters. Command not executed !\n");
          break;
        }
        memset(&pData, 0x00, SIZE520);
        pData[0] = (uint8_t)CmdParam[1];
        pData[1] = (uint8_t)CmdParam[2];
        printf("CMD_WRITE_STATUS_CONF_REG\r\n"
               "Values to be written:\r\n"
               "For Status Register: 0x%2.2x \r\n"
               "For Configuration Register: 0x%2.2x\r\n",
               pData[0], pData[1]);
        m95p32_drv_write_status_config_register(pM95P320, pData, 2);
        break;

      /* Data read command family in normal/fast/dual/quad modes. */

      case CMD_READ_DATA:
        if (CmdParam[2] <= SIZE1024)
        {
          if (CmdParam[3] != 0U)
          {
            printf("CMD_READ_DATA expects 2 parameters. Command not executed !\r\n");
            break;
          }
          memset(&ReceiveBuff, 0x00, SIZE1024);
          m95p32_drv_single_read(pM95P320, ReceiveBuff, CmdParam[1], CmdParam[2]);
          printf(" Read data Add: 0x%6.6x:\n", CmdParam[1]);
          for (index = 0; index < CmdParam[2]; index++)
          {
            printf("  0x%2.2x", ReceiveBuff[index]);
          }
          printf("\n");
        }
        else
        {
          printf("Requested size is greater than configured buffer, kindly check MCU application.\r\n");
        }
        break;

      case CMD_FAST_READ_SINGLE:
        if (CmdParam[2] <= SIZE1024)
        {
          if (CmdParam[3] != 0U)
          {
            printf(" CMD_FAST_READ_SINGLE expects 2 parameters. Command not executed !\n");
            break;
          }
          memset(&ReceiveBuff, 0x00, SIZE1024);
          m95p32_drv_fast_read(pM95P320, ReceiveBuff, CmdParam[1], CmdParam[2]);
          printf(" Fast Read data Add: 0x%6.6x:\n", CmdParam[1]);
          for (index = 0; index < CmdParam[2]; index++)
          {
            printf("  0x%2.2x", ReceiveBuff[index]);
          }
          printf("\n");
        }
        else
        {
          printf("Requested size is greater than configured buffer, kindly check MCU application.\r\n");
        }
        break;

      case CMD_FAST_READ_DUAL:
        if (CmdParam[2] <= SIZE1024)
        {
          if (CmdParam[3] != 0U)
          {
            printf(" CMD_FAST_READ_DUAL expects 2 parameters. Command not executed !\r\n");
            break;
          }
          memset(&ReceiveBuff, 0x00, SIZE1024);
          m95p32_drv_fast_dread(pM95P320, ReceiveBuff, CmdParam[1], CmdParam[2]);
          printf(" Dual Read data Add: 0x%6.6x:\n", CmdParam[1]);
          for (index = 0; index < CmdParam[2]; index++)
          {
            printf("  0x%2.2x", ReceiveBuff[index]);
          }
          printf("\n");
        }
        else
        {
          printf("Requested size is greater than configured buffer, kindly check MCU application.\r\n");
        }
        break;

      case CMD_FAST_READ_QUAD:
        if (CmdParam[2] <= SIZE1024)
        {
          if (CmdParam[3] != 0U)
          {
            printf(" CMD_FAST_READ_Quad expects 2 parameters. Command not executed !\r\n");
            break;
          }
          memset(&ReceiveBuff, 0x00, SIZE1024);
          m95p32_drv_fast_qread(pM95P320, ReceiveBuff, CmdParam[1], CmdParam[2]);
          printf(" Quad Read data Add: 0x%6.6x:\n", CmdParam[1]);
          for (index = 0; index < CmdParam[2]; index++)
          {
            printf("  0x%2.2x", ReceiveBuff[index]);
          }
          printf("\n");
        }
        else
        {
          printf("Requested size is greater than configured buffer, kindly check MCU application.\r\n");
        }
        break;

      /* ID page and identification command family. */

      case CMD_READ_ID_PAGE:
        if (CmdParam[2] <= SIZE1024)
        {
          if (CmdParam[3] != 0U)
          {
            printf(" CMD_READ_ID_PAGE expects 2 parameters. Command not executed !\r\n");
            break;
          }
          memset(&ReceiveBuff, 0x00, SIZE1024);
          m95p32_drv_read_id(pM95P320, ReceiveBuff, CmdParam[1], CmdParam[2]);
          printf(" Read Id Page data Add: 0x%6.6x:\n", CmdParam[1]);
          for (index = 0; index < CmdParam[2]; index++)
          {
            printf("  0x%2.2x", ReceiveBuff[index]);
          }
          printf("\n");
        }
        else
        {
          printf("Requested size is greater than configured buffer, kindly check MCU application.\r\n");
        }
        break;

      case CMD_FAST_READ_ID_PAGE:
        if (CmdParam[2] <= SIZE1024)
        {
          if (CmdParam[3] != 0U)
          {
            printf(" CMD_FAST_READ_ID_PAGE expects 2 parameters. Command not executed !\r\n");
            break;
          }
          memset(&ReceiveBuff, 0x00, SIZE1024);
          m95p32_drv_fast_read_id(pM95P320, ReceiveBuff, CmdParam[1], CmdParam[2]);
          printf(" Fast Read Id Page data Add: 0x%6.6x:\n", CmdParam[1]);
          for (index = 0; index < CmdParam[2]; index++)
          {
            printf(" 0x%2.2x", ReceiveBuff[index]);
          }
          printf("\n");
        }
        else
        {
          printf("Requested size is greater than configured buffer, kindly check MCU application.\r\n");
        }
        break;

      case CMD_WRITE_ID_PAGE:
        if (CmdParam[2] <= SIZE1024)
        {
          memset(&pData, 0x00, SIZE520);
          add = CmdParam[1];
          for (index = 0; index < CmdParam[2]; index++)
          {
            pData[index] = (uint8_t)CmdParam[3];
          }
          m95p32_drv_write_id_page(pM95P320, pData, add, CmdParam[2]);
          printf(" CMD_WRITE_ID_PAGE Add 0x%6.6x data : \n", add);
          for (index_2 = 0; index_2 < index; index_2++)
          {
            printf(" 0x%2.2x", pData[index_2]);
          }
        }
        else
        {
          printf("Requested size is greater than configured buffer, kindly check MCU application.\r\n");
        }
        break;

      /* Power-management command family: deep power-down and release. */

      case CMD_DEEP_POWER_DOWN:
        if (CmdParam[1] != 0U)
        {
          printf(" CMD_DEEP_POWER_DOWN expects 0 parameter. Command not executed !\r\n");
          break;
        }
        m95p32_drv_deep_power_down(pM95P320);
        printf("CMD_DEEP_POWER_DOWN\r\n");
        break;

      case CMD_DEEP_POWER_DOWN_RELEASE:
        if (CmdParam[1] != 0U)
        {
          printf(" CMD_DEEP_POWER_DOWN_RELEASE expects 0 parameter. Command not executed !\r\n");
          break;
        }
        m95p32_drv_deep_power_down_release(pM95P320);
        printf("CMD_DEEP_POWER_DOWN_RELEASE\r\n");
        break;

      /* Discovery command family: JEDEC and SFDP table access. */

      case CMD_READ_JEDEC:
        if (CmdParam[1] != 0U)
        {
          printf(" CMD_READ_JEDEC expects 0 parameter. Command not executed !\r\n");
          break;
        }
        memset(&ReceiveBuff, 0x00, SIZE1024);
        m95p32_drv_read_jedec(pM95P320, ReceiveBuff, 3);
        printf("CMD_READ_JEDEC: ");
        for (index = 0; index < 3U; index++)
        {
          printf(" 0x%2.2x", ReceiveBuff[index]);
        }
        printf("\n");
        break;

      case CMD_READ_SFDP:
        if (CmdParam[2] <= SIZE1024)
        {
          if (CmdParam[3] != 0U)
          {
            printf(" CMD_READ_SFDP expects 2 parameter. Command not executed !\r\n");
            break;
          }
          memset(&ReceiveBuff, 0x00, SIZE1024);
          m95p32_drv_read_sfdp(pM95P320, ReceiveBuff, CmdParam[1], CmdParam[2]);
          printf("CMD_READ_SFDP: \n");
          for (index = 0; index < CmdParam[2]; index++)
          {
            printf("0x%4.4x : 0x%2.2x\n", index + CmdParam[1], ReceiveBuff[index]);
          }
          printf("\n");
        }
        else
        {
          printf("Requested size is greater than configured buffer, kindly check MCU application.\r\n");
        }
        break;

      /* Software reset command family. */

      case CMD_ENABLE_RESET:
        if (CmdParam[1] != 0U)
        {
          printf("CMD_ENABLE_RESET expects 0 parameter. Command not executed !\r\n");
          break;
        }
        m95p32_drv_reset_enable(pM95P320);
        printf("CMD_ENABLE_RESET\r\n");
        break;

      case CMD_SOFT_RESET:
        if (CmdParam[1] != 0U)
        {
          printf("CMD_SOFT_RESET expects 0 parameter. Command not executed !\r\n");
          break;
        }
        m95p32_drv_soft_reset(pM95P320);
        printf("CMD_SOFT_RESET\r\n");
        break;

      default:
        printf("Nucleo_reader : Enter a ranged value : \r\n");
        break;
    }

    /* Prompt user for next command when not a refresh ('R') */
    if (CmdParam[0] != 'R')
    {
      printf("Enter Command set: \r\n");
      User_Choice = 1;
    }

    /* Clear working buffers and restart UART reception */
    memset(&aRxBuffer, 0x00, RXBUFFERSIZE);
    memset(&CmdParam, 0x00, sizeof(CmdParam));
    memset(&Buff, 0x00, SIZE256);

    UartReady = 1;
    HAL_CORTEX_NVIC_EnableIRQ(USART2_IRQn);
    HAL_Delay(100);
    if (HAL_UART_Receive_IT(BASIC_STDIO_UART_GETHANDLE(), (uint8_t *)aRxBuffer, 1) != HAL_OK)
    {
      ret = EXEC_STATUS_ERROR;
    }

  }

  /* IT_receive == 2: user scenario menu (predefined test sequences) */
  if (IT_receive == 2)
  {
    /*
     * User scenario path executes curated demonstrations that chain
     * multiple driver operations and safety checks with fixed flows.
     */
    IT_receive = 0;

    if (User_Choice != 2)
    {
      /* Read new menu selection byte synchronously */
      HAL_UART_Receive(BASIC_STDIO_UART_GETHANDLE(), (uint8_t *)aRxBuffer, 1, 0xffffff);
    }

    User_Choice = 2;

    switch (aRxBuffer[0])
    {
      case 0x31:

        printf("\nProgram with Safety Register Check: \r\n");
        M95P32_PageProg_with_SafetyRegisterCheck();
        break;

      case 0x32:

        printf("\nWrite with Safety Register Check: \r\n");
        M95P32_PageWrite_with_SafetyRegisterCheck();
        break;

      case 0x33:

        printf("\nBuffer Mode: \r\n");
        M95P32_PageProgram_with_BufferLoad();
        break;

      default:
        printf("\nReturning to main menu \r\n");
        User_Choice = 0;
        IntroScreen();
        break;
    }

    if (User_Choice != 0)
    {
      User_Scenario_IntroScreen();
    }

    UartReady = 1;
    HAL_CORTEX_NVIC_EnableIRQ(USART2_IRQn);
    memset(&aRxBuffer, 0x00, RXBUFFERSIZE);
    if (HAL_UART_Receive_IT(BASIC_STDIO_UART_GETHANDLE(), (uint8_t *)aRxBuffer, 1) != HAL_OK)
    {
      ret = EXEC_STATUS_ERROR;
    }
  }

  /* IT_receive == 3: "Tips and Tricks" scenario menu */
  if (IT_receive == 3)
  {
    /*
     * Specific behaviour path focuses on corner cases:
     * - alignment constraints,
     * - writes into protected areas,
     * and safety-register interpretation of those operations.
     */
    IT_receive = 0;

    if (User_Choice != 3)
    {
      HAL_UART_Receive(BASIC_STDIO_UART_GETHANDLE(), (uint8_t *)aRxBuffer, 1, 0xffffff);
    }

    User_Choice = 3;

    switch (aRxBuffer[0])
    {
      case 0x31:

        printf("\n16 word align: \r\n");
        M95P32_16Word_align();

        /* Erase block 0 after tip demonstration */
        m95p32_drv_write_enable(pM95P320);
        m95p32_drv_block_erase(pM95P320, TARGET_ADDR_0000);

        break;

      case 0x32:

        printf("\nWrite in protected area \r\n");
        M95P32_WriteProtectedArea();
        break;

      default:
        printf("\nReturning to main menu \r\n");
        User_Choice = 0;
        IntroScreen();
        break;
    }

    if (User_Choice != 0)
    {
      Specific_Behaviour_IntroScreen();
    }

    UartReady = 1;
    HAL_CORTEX_NVIC_EnableIRQ(USART2_IRQn);

    memset(&aRxBuffer, 0x00, RXBUFFERSIZE);
    if (HAL_UART_Receive_IT(BASIC_STDIO_UART_GETHANDLE(), (uint8_t *)aRxBuffer, 1) != HAL_OK)
    {
      ret = EXEC_STATUS_ERROR;
    }
  }

  /* IT_receive == 4: automatic regression test suite */
  if (IT_receive == 4)
  {
    /*
     * Regression path runs all standalone tests in fixed order and reports
     * pass/fail after each step so manufacturing or validation logs are clear.
     */
    IT_receive = 0;

    int32_t status = EXEC_STATUS_OK;
    /* Test Write Enable and Write Disable*/
    status = M95P32_TestWriteEnableDisable();
    if (status != EXEC_STATUS_OK)
    {
      printf("\t**************Test 1 Failed**************\r\n\n");
      ret = EXEC_STATUS_ERROR;
    }
    else
    {
      printf("\t**************Test 1 Passed**************\r\n\n");
    }

    /* Test Page Read and Write Operation*/
    status = M95P32_TestPage();
    if (status != EXEC_STATUS_OK)
    {
      printf("\t**************Test 2 Failed**************\r\n\n");
      ret = EXEC_STATUS_ERROR;
    }
    else
    {
      printf("\t**************Test 2 Passed**************\r\n\n");
    }

    /* Test Page Erase Operation*/
    status = M95P32_TestPageErase();
    if (status != EXEC_STATUS_OK)
    {
      printf("\t**************Test 3 Failed**************\r\n\n");
      ret = EXEC_STATUS_ERROR;
    }
    else
    {
      printf("\n\n\r Page Erase				: Passed\r\n");
      printf("\t**************Test 3 Passed**************\r\n\n");
    }

    /* Test Sector Erase Operation*/
    status = M95P32_TestSectorErase();
    if (status != EXEC_STATUS_OK)
    {
      printf("\t**************Test 4 Failed**************\r\n\n");
      ret = EXEC_STATUS_ERROR;
    }
    else
    {
      printf("\n\n\r Sector Erase				: Passed\r\n");
      printf("\t**************Test 4 Passed**************\r\n\n");
    }

    /* Test Block Erase Operation*/
    status = M95P32_TestBlockErase();
    if (status != EXEC_STATUS_OK)
    {
      printf("\t**************Test 5 Failed**************\r\n\n");
      ret = EXEC_STATUS_ERROR;
    }
    else
    {
      printf("\n\n\r Block Erase				: Passed\r\n");
      printf("\t**************Test 5 Passed**************\r\n\n");
    }

    /* Test Buffer Mode Operation*/
    status = M95P32_TestBufferMode();
    if (status != EXEC_STATUS_OK)
    {
      printf("\t**************Test 6 Failed**************\r\n\n");
    }
    else
    {
      printf("\n\n\r Buffer Mode				: Passed\r\n");
      printf("\t**************Test 6 Passed**************\r\n\n");
    }

    if (status != EXEC_STATUS_OK)
    {
      ret = EXEC_STATUS_ERROR;
    }

    /*Erase memory after regression suite for clean state*/
    m95p32_drv_write_enable(pM95P320);
    m95p32_drv_chip_erase(pM95P320);

    IntroScreen();

    UartReady = 1;
    HAL_CORTEX_NVIC_EnableIRQ(USART2_IRQn);
    if (HAL_UART_Receive_IT(BASIC_STDIO_UART_GETHANDLE(), (uint8_t *)aRxBuffer, 1) != HAL_OK)
    {
      ret = EXEC_STATUS_ERROR;
    }

  }

  return ret;
}

/**
  * @brief  Receive bytes in blocking mode until CR (0x0D) or LF (0x0A) is received,
  *         or until Size or Timeout is reached.
  * @param  huart    Pointer to UART handle structure.
  * @param  pData    Pointer to destination buffer.
  * @param  Size     Maximum number of bytes to receive.
  * @param  Timeout  Timeout in milliseconds.
  * @retval HAL status:
  *         HAL_OK, HAL_ERROR, HAL_BUSY or HAL_TIMEOUT.
  */
hal_status_t HAL_UART_Receive_Until_CR(hal_uart_handle_t *huart, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
  if ((huart == NULL) || (pData == NULL) || (Size == 0U))
  {
    return HAL_ERROR;
  }

  /* Reject request if UART is not idle */
  if (huart->rx_state != HAL_UART_RX_STATE_IDLE)
  {
    return HAL_BUSY;
  }

  hal_status_t status = HAL_OK;
  uint16_t received = 0;
  uint32_t tickstart = HAL_GetTick();

  huart->rx_state = HAL_UART_RX_STATE_ACTIVE;

  /* Receive byte-by-byte until CR/LF, size or timeout */
  while (received < Size)
  {
    /* Wait for RXNE flag, enforcing timeout */
    while (!LL_USART_IsActiveFlag((USART_TypeDef *)huart->instance, USART_ISR_RXNE))
    {
      if ((HAL_GetTick() - tickstart) > Timeout)
      {
        status = HAL_TIMEOUT;
        huart->rx_state = HAL_UART_RX_STATE_IDLE;
        return status;
      }
    }

    /* Cache received byte */
    uint8_t data = (uint8_t)(LL_USART_ReceiveData8((USART_TypeDef *)huart->instance));
    pData[received++] = data;

    /* Stop reception when CR or LF is received */
    if ((data == 0x0D) || (data == 0x0A))
    {
      status = HAL_OK;
      break;
    }
  }

  huart->rx_state = HAL_UART_RX_STATE_IDLE;
  return status;
}

/**
  * @brief  Programs specific Page with checks on safety register.
  * @param  None
  * @retval None
  */
void M95P32_PageProg_with_SafetyRegisterCheck()
{
  /*
   * This scenario demonstrates a full erase->program->verify cycle while
   * checking Safety Register flags at each milestone.
   *
   * Key flags observed in this routine:
   * - PUF: power-up completion indication.
   * - ERF: erase operation fault indication.
   * - PRF: program operation fault indication.
   * - ECC bits: correction events during reads.
   */
  uint8_t sample_data;
  uint8_t status_reg_val = 0;
  uint32_t index = 0;

  /* Poll status register until WIP (bit 0) is cleared after power-up */
  do
  {
    m95p32_drv_read_status_register(pM95P320, &status_reg_val);
  } while ((status_reg_val & 0x01) != 0);
  printf("Power UP Done.\r\n");

  /* Check Power Up Flag (PUF, bit 6) in safety register and wait until it is cleared */
  memset(&ReceiveBuff, 0x00, SIZE1024);
  m95p32_drv_read_config_safety_register(pM95P320, ReceiveBuff, 2);
  while ((ReceiveBuff[1] & 0x40) != 0)  /* ReceiveBuff[1] holds the Safety Register */
  {
    /* Re-read safety register until PUF bit is cleared */
    m95p32_drv_read_config_safety_register(pM95P320, ReceiveBuff, 2);
  }
  printf("Power UP OK.\r\n");

  /* Enable write operations */
  m95p32_drv_write_enable(pM95P320);

  /* Erase page at base address 0x00 */
  m95p32_drv_page_erase(pM95P320, TARGET_ADDR_0000);

  /* Wait for erase to complete (WIP cleared) */
  status_reg_val = 0;
  do
  {
    m95p32_drv_read_status_register(pM95P320, &status_reg_val);
  } while ((status_reg_val & 0x01) != 0);
  printf("Page Erase Done.\r\n");

  /* Check Erase Flag (ERF, bit 5) in safety register to confirm erase status */
  memset(&ReceiveBuff, 0x00, SIZE1024);
  m95p32_drv_read_config_safety_register(pM95P320, ReceiveBuff, 2);
  if ((ReceiveBuff[1] & 0x20) == 0)
  {
    printf("Page Erase Done without issues.\r\n");

  }
  else
  {
    printf("Page Erase NOT DONE.\r\n");
  }

  /* Verify page erase by reading full page and checking all bytes are 0xFF */
  memset(&ReceiveBuff, 0x00, SIZE1024);
  m95p32_drv_single_read(pM95P320, ReceiveBuff, 0, M95P32_PAGE_SIZE);

  for (index = 0; index < M95P32_PAGE_SIZE; index++)
  {
    if (ReceiveBuff[index] != 0xFF)
    {
      break;
    }
  }
  if (index != M95P32_PAGE_SIZE)
  {
    printf("Page NOT Erased Correctly.\r\n");
  }
  else
  {
    printf("Page Erased Correctly.\r\n");
  }

  /* Check that no ECC-related flags are set after erase verification */
  memset(&ReceiveBuff, 0x00, SIZE1024);
  m95p32_drv_read_config_safety_register(pM95P320, ReceiveBuff, 2);
  if ((ReceiveBuff[1] & 0x0F) == 0)
  {
    printf("NO ECC detected during Erase Check.\r\n");
  }
  else
  {
    printf("ECC detected during Erase Check.\r\n");
  }

  /* Enable write operations for page program */
  m95p32_drv_write_enable(pM95P320);

  /* Prepare programming pattern (depends on SPI mode) */
#ifdef USE_QUADSPI
  memset(&pData, 0x0F, SIZE600);
#else
  memset(&pData, 0x55, SIZE600);
#endif /* USE_QUADSPI */

  add = 0;
  /* Program one full page starting at address 0x00 */
  m95p32_drv_page_prog(pM95P320, pData, add, M95P32_PAGE_SIZE);

  /* Check Program Flag (PRF, bit 4) in safety register */
  memset(&ReceiveBuff, 0x00, SIZE1024);
  m95p32_drv_read_config_safety_register(pM95P320, ReceiveBuff, 2);
  if ((ReceiveBuff[1] & 0x10) == 0)
  {
    printf("Page Program Done without issues.\r\n");

  }
  else
  {
    printf("Page Program NOT DONE.\r\n");
  }

  /* Verify page program by reading page and comparing with expected pattern */
  memset(&ReceiveBuff, 0x00, SIZE1024);
  m95p32_drv_single_read(pM95P320, ReceiveBuff, 0, M95P32_PAGE_SIZE);

#ifdef USE_QUADSPI
  sample_data = 0x0F;
#else
  sample_data = 0x55;
#endif /* USE_QUADSPI */

  for (index = 0; index < M95P32_PAGE_SIZE; index++)
  {
    if (ReceiveBuff[index] != sample_data)
    {
      break;
    }
  }
  if (index != M95P32_PAGE_SIZE)
  {
    printf("Page NOT Programmed Correctly.\r\n");
  }
  else
  {
    printf("Page Programmed Correctly.\r\n");
  }

  /* Final safety register check for ECC flags after program verification */
  memset(&ReceiveBuff, 0x00, SIZE1024);
  m95p32_drv_read_config_safety_register(pM95P320, ReceiveBuff, 2);
  if ((ReceiveBuff[1] & 0x0F) == 0)
  {
    printf("NO ECC detected during Page Program Check.\r\n");
  }
  else
  {
    printf("ECC detected during Page Program Check.\r\n");
  }

}

/**
  * @brief  Tests page write operation while monitoring safety register.
  *         Performs two consecutive page writes with different patterns.
  * @param  None
  * @retval None
  */
void M95P32_PageWrite_with_SafetyRegisterCheck()
{
  /*
   * Write-with-safety scenario:
   * - Perform two page writes using distinct patterns.
   * - Validate payload correctness after each write.
   * - Observe safety register after each operation for faults/ECC events.
   */
  uint8_t status_reg_val = 0;
  uint32_t index = 0;

  /* Wait until device is ready (WIP bit cleared) after power-up */
  do
  {
    m95p32_drv_read_status_register(pM95P320, &status_reg_val);
  } while ((status_reg_val & 0x01) != 0);
  printf("Power UP Done.\r\n");

  /* Poll Power Up Flag (PUF, bit 6) in safety register until cleared */
  memset(&ReceiveBuff, 0x00, SIZE1024);
  m95p32_drv_read_config_safety_register(pM95P320, ReceiveBuff, 2);
  while ((ReceiveBuff[1] & 0x40) != 0)  /* ReceiveBuff[1] holds the Safety Register */
  {
    m95p32_drv_read_config_safety_register(pM95P320, ReceiveBuff, 2);
  }
  printf("Power UP OK.\r\n");

  /* Enable write operations */
  m95p32_drv_write_enable(pM95P320);

  /* First page write with data pattern 0x56 */
  memset(&pData, 0x56, SIZE600);

  add = 0;
  m95p32_drv_page_write(pM95P320, pData, add, M95P32_PAGE_SIZE);

  /* Check erase (ERF) and program (PRF) flags in safety register (bits 5 and 4) */
  memset(&ReceiveBuff, 0x00, SIZE1024);
  m95p32_drv_read_config_safety_register(pM95P320, ReceiveBuff, 2);
  if ((ReceiveBuff[1] & 0x30) == 0)
  {
    printf("Page Write Done without issues.\r\n");

  }
  else
  {
    printf("Page Write NOT DONE.\r\n");
  }

  /* Verify first write by reading back page and expecting 0x56 pattern */
  memset(&ReceiveBuff, 0x00, SIZE1024);
  m95p32_drv_single_read(pM95P320, ReceiveBuff, 0, M95P32_PAGE_SIZE);

  for (index = 0; index < M95P32_PAGE_SIZE; index++)
  {
    if (ReceiveBuff[index] != 0x56)
    {
      break;
    }
  }
  if (index != M95P32_PAGE_SIZE)
  {
    printf("Page NOT Written Correctly.\r\n");
  }
  else
  {
    printf("Page Written Correctly.\r\n");
  }

  /* ECC check after first write */
  memset(&ReceiveBuff, 0x00, SIZE1024);
  m95p32_drv_read_config_safety_register(pM95P320, ReceiveBuff, 2);
  if ((ReceiveBuff[1] & 0x0F) == 0)
  {
    printf("NO ECC detected during Page Write Check.\r\n");
  }
  else
  {
    printf("ECC detected during Page Write Check.\r\n");
  }

  /* Second page write with data pattern 0xA9 (no erase in between) */
  memset(&pData, 0xA9, SIZE600);

  add = 0;
  m95p32_drv_page_write(pM95P320, pData, add, M95P32_PAGE_SIZE);

  /* Check safety flags (ERF and PRF) again after second write */
  memset(&ReceiveBuff, 0x00, SIZE1024);
  m95p32_drv_read_config_safety_register(pM95P320, ReceiveBuff, 2);
  if ((ReceiveBuff[1] & 0x30) == 0)
  {
    printf("Page Write Done without issues.\r\n");

  }
  else
  {
    printf("Page Write NOT DONE.\r\n");
  }

  /* Verify second write by reading back page and expecting 0xA9 pattern */
  memset(&ReceiveBuff, 0x00, SIZE1024);
  m95p32_drv_single_read(pM95P320, ReceiveBuff, 0, M95P32_PAGE_SIZE);

  for (index = 0; index < M95P32_PAGE_SIZE; index++)
  {
    if (ReceiveBuff[index] != 0xA9)
    {
      break;
    }
  }
  if (index != M95P32_PAGE_SIZE)
  {
    printf("Page NOT Written Correctly.\r\n");
  }
  else
  {
    printf("Page Written Correctly.\r\n");
  }

  /* ECC check after second write */
  memset(&ReceiveBuff, 0x00, SIZE1024);
  m95p32_drv_read_config_safety_register(pM95P320, ReceiveBuff, 2);
  if ((ReceiveBuff[1] & 0x0F) == 0)
  {
    printf("NO ECC detected during Page Write Check.\r\n");
  }
  else
  {
    printf("ECC detected during Page Write Check.\r\n");
  }

}

/**
  * @brief  Page program example using buffer load helper.
  *         Erases sector, checks safety, then programs 600 bytes in buffer mode.
  * @param  None
  * @retval None
  */
void M95P32_PageProgram_with_BufferLoad()
{
  /*
   * Wrapper scenario for buffer-load helper:
   * - Ensure device is ready and safety state is sane.
   * - Erase target sector before buffer programming.
   * - Invoke helper with a fixed-size test payload.
   */
  uint8_t status_reg_val = 0;

  /* Wait until device is ready after power-up */
  do
  {
    m95p32_drv_read_status_register(pM95P320, &status_reg_val);
  } while ((status_reg_val & 0x01) != 0);
  printf("Power UP Done.\r\n");

  /* Wait until Power Up Flag is cleared in safety register */
  memset(&ReceiveBuff, 0x00, SIZE1024);
  m95p32_drv_read_config_safety_register(pM95P320, ReceiveBuff, 2);
  while ((ReceiveBuff[1] & 0x40) != 0)  /* ReceiveBuff[1] corresponds to Safety Register */
  {
    m95p32_drv_read_config_safety_register(pM95P320, ReceiveBuff, 2);
  }
  printf("Power UP OK.\r\n");

  /* Enable write operations prior to sector erase */
  m95p32_drv_write_enable(pM95P320);

  /* Erase sector containing address 0x00 */
  m95p32_drv_sector_erase(pM95P320, TARGET_ADDR_0000);

  /* Check safety register value after erase (expect no errors) */
  memset(&ReceiveBuff, 0x00, SIZE1024);
  m95p32_drv_read_config_safety_register(pM95P320, ReceiveBuff, 2);
  if (ReceiveBuff[1] == 0)
  {
    printf("Safety Register status OK.\r\n");
  }
  else
  {
    printf("Safety Register status NOT OK.\r\n");
  }

  /* Prepare parameters for buffer-mode page program */
  uint32_t start_prog_add = 0;
  uint32_t nb_byte = SIZE600;

  memset(&pData, 0x89, SIZE600);
  if (nb_byte > SIZE600)
  {
    printf("Data buffer size is greater than allocated memory, Aborted!\r\n");
  }
  else
  {
    /* Program from address 0x00 with pattern 0x89 and nb_byte=600 using buffer load */
    PageProgram_with_BufferLoad(pM95P320, pData,  start_prog_add,  nb_byte);
  }
}

/**
  * @brief  Demonstrates 16-word alignment constraints on M95P32
  *         through erase and page program operations.
  * @param  None
  * @retval None
  */
void M95P32_16Word_align()
{
  /*
   * Alignment demonstration:
   * - Compare behavior of aligned and unaligned program addresses.
   * - Use safety register to explain why unaligned attempt is rejected.
   * - Read back data to correlate flags with actual memory content.
   */
  uint8_t status_reg_val = 0;
  uint32_t index;

  /* Wait until device is ready after power-up */
  do
  {
    m95p32_drv_read_status_register(pM95P320, &status_reg_val);
  } while ((status_reg_val & 0x01) != 0);
  printf("Power UP Done.\r\n");

  /* Check PUF and other high bits in safety register */
  memset(&ReceiveBuff, 0x00, SIZE1024);
  m95p32_drv_read_config_safety_register(pM95P320, ReceiveBuff, 2);
  if ((ReceiveBuff[1] & 0xF0) == 0)  /* ReceiveBuff[1] corresponds to Safety Register */
  {
    printf("Power UP OK.\r\n");
  }
  else
  {
    printf("Power UP NOT OK.\r\n");
  }

  /* Enable writes */
  m95p32_drv_write_enable(pM95P320);

  /* Erase full page at address 0x00 */
  m95p32_drv_page_erase(pM95P320, 0x00);

  /* Read safety register and check that PAMAF/ERF bits are clear */
  memset(&ReceiveBuff, 0x00, SIZE1024);
  m95p32_drv_read_config_safety_register(pM95P320, ReceiveBuff, 2);
  printf("Page Erased from address 0x00. Safety Register Value is: 0x%x \r\n", ReceiveBuff[1]);
  if ((ReceiveBuff[1] & 0xA0) == 0)
  {
    printf("PAMAF and ERF bit not set. OK.\r\n");
  }
  else
  {
    printf("Error ! PAMAF or ERF bit is set. NOT OK! \r\n");
  }

  /* Verify page erase by checking all bytes are 0xFF */
  memset(&ReceiveBuff, 0x00, SIZE1024);
  m95p32_drv_single_read(pM95P320, ReceiveBuff, 0x00, M95P32_PAGESIZE);

  for (index = 0; (ReceiveBuff[index] == 0xff) && (index < M95P32_PAGESIZE); index++)
  {
    /* Scan forward until first non-0xFF byte */
  }

  if (index == M95P32_PAGESIZE)
  {
    printf("Erased page data is in erased state <0xFF>. OK.\r\n");
  }
  else
  {
    printf("Erased page data is NOT in erased state. Abort!\r\n");
    return;
  }

  /* Check safety register for ECC flags after erase */
  memset(&ReceiveBuff, 0x00, SIZE1024);
  m95p32_drv_read_config_safety_register(pM95P320, ReceiveBuff, 2);
  printf("Safety Register Value is: 0x%x \r\n", ReceiveBuff[1]);
  if ((ReceiveBuff[1] & 0x0F) == 0)
  {
    printf("No ECC flag set. OK.\r\n\n");
  }
  else
  {
    printf("ECC bits are set. NOT OK!\r\n");
  }

  /* 1) Program 1 byte at address 0x00 (word-aligned) */

  m95p32_drv_write_enable(pM95P320);
  memset(&pData, 0x55, SIZE600);
  m95p32_drv_page_prog(pM95P320, pData, 0, 1);

  /* Wait for program completion */
  do
  {
    m95p32_drv_read_status_register(pM95P320, &status_reg_val);
  } while ((status_reg_val & 0x01) != 0);

  /* Safety register check after aligned program */
  memset(&ReceiveBuff, 0x00, SIZE1024);
  m95p32_drv_read_config_safety_register(pM95P320, ReceiveBuff, 2);
  printf("Programmed 1 byte from address 0x00 with value 0x55. Safety Register Value is: 0x%x \r\n", ReceiveBuff[1]);
  if ((ReceiveBuff[1] & 0xF0) == 0)
  {
    printf("No flag set. OK.\r\n");
  }
  else
  {
    printf("Flag set. NOT OK!\r\n");
  }

  /* Read back the byte at 0x00 and check ECC */
  memset(&ReceiveBuff, 0x00, SIZE1024);
  m95p32_drv_single_read(pM95P320, ReceiveBuff, 0x00, 1);

  memset(&ReceiveBuff, 0x00, SIZE1024);
  m95p32_drv_read_config_safety_register(pM95P320, ReceiveBuff, 2);
  printf("Read 1 byte from address 0x00. Safety Register Value is: 0x%x \r\n", ReceiveBuff[1]);
  if ((ReceiveBuff[1] & 0x0F) == 0)
  {
    printf("No ECC flag set. OK.\r\n\n");
  }
  else
  {
    printf("ECC Flag set. NOT OK!\r\n");
  }

  /* 2) Program 1 byte at address 0x100 (16-word aligned boundary) */

  m95p32_drv_write_enable(pM95P320);
  memset(&pData, 0xA5, SIZE600);
  m95p32_drv_page_prog(pM95P320, pData, 0x100, 1);

  do
  {
    m95p32_drv_read_status_register(pM95P320, &status_reg_val);
  } while ((status_reg_val & 0x01) != 0);

  memset(&ReceiveBuff, 0x00, SIZE1024);
  m95p32_drv_read_config_safety_register(pM95P320, ReceiveBuff, 2);
  printf("Programmed 1 byte from address 0x100 with value 0xA5. Safety Register Value is: 0x%x \r\n", ReceiveBuff[1]);
  if ((ReceiveBuff[1] & 0xF0) == 0x00)
  {
    printf("No Flag set. OK.\r\n");
  }
  else
  {
    printf("Flag set. NOT OK!\r\n");
  }

  /* Read back data at aligned address 0x100 */
  memset(&ReceiveBuff, 0x00, SIZE1024);
  m95p32_drv_single_read(pM95P320, ReceiveBuff, 0x100, 1);
  printf("Data at address 0x100 is: 0x%X \r\n", ReceiveBuff[0]);
  if (ReceiveBuff[0] == 0xA5)
  {
    printf("Page Program successful\r\n");
  }
  else
  {
    printf("Page Program NOT successful\r\n");
  }

  /* Safety check after aligned program at 0x100 */
  memset(&ReceiveBuff, 0x00, SIZE1024);
  m95p32_drv_read_config_safety_register(pM95P320, ReceiveBuff, 2);
  printf("Read 1 byte from address 0x100. Safety Register Value is: 0x%x \r\n", ReceiveBuff[1]);
  if ((ReceiveBuff[1] & 0x0F) == 0)
  {
    printf("No ECC flag set. OK.\r\n\n");
  }
  else
  {
    printf("ECC Flag set. NOT OK!\r\n");
  }

  /* 3) Program 1 byte at address 0x101 (not aligned on 16-word boundary) */

  m95p32_drv_write_enable(pM95P320);
  memset(&pData, 0x55, SIZE600);
  m95p32_drv_page_prog(pM95P320, pData, 0x101, 1);

  do
  {
    m95p32_drv_read_status_register(pM95P320, &status_reg_val);
  } while ((status_reg_val & 0x01) != 0);

  /* Check safety flags (expect PRF to be set for misaligned access) */
  memset(&ReceiveBuff, 0x00, SIZE1024);
  m95p32_drv_read_config_safety_register(pM95P320, ReceiveBuff, 2);
  printf("Programmed 1 byte from address 0x101 with value 0x55. Safety Register Value is: 0x%x \r\n", ReceiveBuff[1]);
  if ((ReceiveBuff[1] & 0xF0) == 0)
  {
    printf("No flag set. Error!\r\n");
  }
  else
  {
    if ((ReceiveBuff[1] & 0x10) == 0x10)
    {
      printf("PRF Flag set as expected!\r\n");
    }
  }

  /* Clear all safety flags before reading back */
  m95p32_drv_clear_safety_flag(pM95P320);

  /* Read back byte at unaligned address 0x101 */
  memset(&ReceiveBuff, 0x00, SIZE1024);
  m95p32_drv_single_read(pM95P320, ReceiveBuff, 0x101, 1);
  printf("Data at address 0x101 is: 0x%X \r\n", ReceiveBuff[0]);
  if (ReceiveBuff[0] == 0x55)
  {
    printf("Page Programmed. Error.\r\n");
  }
  else
  {
    printf("Page Program NOT successful\r\n");
  }

  /* Final safety check for ECC flags */
  memset(&ReceiveBuff, 0x00, SIZE1024);
  m95p32_drv_read_config_safety_register(pM95P320, ReceiveBuff, 2);
  printf("Safety Register Value is: 0x%x \r\n", ReceiveBuff[1]);
  if ((ReceiveBuff[1] & 0x0F) == 0)
  {
    printf("No ECC flag set. OK.\r\n");
  }
  else
  {
    printf("ECC Flag set. NOT OK!\r\n");
  }

}

/**
  * @brief  Demonstrates behavior when writing to fully protected memory.
  *         Sets BP bits, attempts a write, checks PAMAF and restores protection.
  * @param  None
  * @retval None
  */
void M95P32_WriteProtectedArea()
{
  /*
   * Protection demonstration:
   * - Enable full array protection through status register BP bits.
   * - Attempt a write to protected address.
   * - Confirm PAMAF flag is raised and data remains unchanged.
   * - Restore unprotected status for subsequent operations.
   */
  uint8_t status_reg_val = 0;

  /* Wait until device ready after power-up */
  do
  {
    m95p32_drv_read_status_register(pM95P320, &status_reg_val);
  } while ((status_reg_val & 0x01) != 0);
  printf("Power UP Done.\r\n");

  /* Wait until PUF is cleared in safety register */
  memset(&ReceiveBuff, 0x00, SIZE1024);
  m95p32_drv_read_config_safety_register(pM95P320, ReceiveBuff, 2);
  while ((ReceiveBuff[1] & 0x40) != 0)  /* ReceiveBuff[1] corresponds to Safety Register */
  {
    m95p32_drv_read_config_safety_register(pM95P320, ReceiveBuff, 2);
  }
  printf("Power UP OK.\r\n");

  /* Enable writes before modifying status register protection bits */
  m95p32_drv_write_enable(pM95P320);

  /* Set BPx bits to 1 (0x1C) to fully protect memory array */
  memset(&pData, 0x00, SIZE520);
  pData[0] = 0x1C;
  m95p32_drv_write_status_config_register(pM95P320, pData, 1);

  /* Verify status register content after write */
  m95p32_drv_read_status_register(pM95P320, &status_reg_val);
  printf("Status Register value is: 0x%X\r\n", status_reg_val);
  if (status_reg_val == 0x1C)
  {
    printf("Chip fully protected.\r\n");
  }
  else
  {
    printf("Chip not protected.\r\n");
  }

  /* Read existing value at address 0x600 for reference */
  memset(&ReceiveBuff, 0x00, SIZE1024);
  m95p32_drv_single_read(pM95P320, ReceiveBuff, 0x600, 1);
  printf("Data stored at address 0x600 is 0x%X \r\n", ReceiveBuff[0]);

  /* Attempt to write into the protected range */

  m95p32_drv_write_enable(pM95P320);

  /* Prepare data 0x87 for attempted write at 0x600 */
  memset(&pData, 0x87, SIZE600);

  add = 0x600;
  m95p32_drv_page_write(pM95P320, pData, add, 1);

  /* Disable write; note that due to protection the previous write is not effective */
  m95p32_drv_write_disable(pM95P320);

  /* Check PAMAF bit (bit 7) for "protected array modified attempt" */
  memset(&ReceiveBuff, 0x00, SIZE1024);
  m95p32_drv_read_config_safety_register(pM95P320, ReceiveBuff, 2);
  printf("Attempt to write 0x87 at address 0x600. Safety Register Value is: 0x%X \r\n", ReceiveBuff[1]);
  if ((ReceiveBuff[1] & 0x80) == 0x80)
  {
    printf("PAMAF bit set, Modify operation to a protected memory area has been attempted.\r\n");
  }
  else
  {
    printf("Error ! PAMAF bit must be set! \r\n");
  }

  /* Clear safety flags after detection */
  m95p32_drv_clear_safety_flag(pM95P320);

  /* Read data at address 0x600 and confirm it has not changed */
  memset(&ReceiveBuff, 0x00, SIZE1024);
  m95p32_drv_single_read(pM95P320, ReceiveBuff, 0x600, 1);
  printf("Data stored at address 0x600 is 0x%X\r\n", ReceiveBuff[0]);

  /* Check safety register for ECC flags (only PAMAF expected previously) */
  memset(&ReceiveBuff, 0x00, SIZE1024);
  m95p32_drv_read_config_safety_register(pM95P320, ReceiveBuff, 2);
  if ((ReceiveBuff[1] & 0x0F) == 0)
  {
    printf("NO ECC correction detected. OK. \r\n");
  }
  else
  {
    printf("ECC correction detected !\r\n");
  }

  /* Enable write to restore protection bits (clear BPx) */
  m95p32_drv_write_enable(pM95P320);

  memset(&pData, 0x00, SIZE520);
  m95p32_drv_write_status_config_register(pM95P320, pData, 1);

  /* Confirm that BPx bits were cleared */
  m95p32_drv_read_status_register(pM95P320, &status_reg_val);
  printf("Status Register value : 0x%X\r\n", status_reg_val);

}

/**
  * @brief Print main menu on UART console.
  * @param None
  * @retval None
  */
void IntroScreen(void)
{
  UARTConsolePrint("\r\n"
                   "**********************************\r\n"
                   "Main Menu \r\n"
                   "**********************************\r\n"
                   "Command Set            : 1\r\n"
                   "User Scenario          : 2\r\n"
                   "Specific Behaviour     : 3\r\n"
                   "Run Test Cases         : 4\r\n\n"
                   "**********************************\r\n\n"
                   "Enter Command Number   :\r\n");

}

/**
  * @brief Print user scenario menu on UART console.
  * @param None
  * @retval None
  */
void User_Scenario_IntroScreen(void)
{
  UARTConsolePrint("\r\n"
                   "***************************************************\r\n"
                   "User Scenario \r\n"
                   "***************************************************\r\n"
                   "Program with safety register check          : 1\r\n"
                   "Write with safety register check            : 2\r\n"
                   "Buffer Mode                                 : 3\r\n"

                   "Enter any other option to return to Main Menu!\r\n"
                   "***************************************************\r\n\n"

                   "Enter Command Number : \r\n");
}

/**
  * @brief Print specific behaviour (tips and tricks) menu on UART console.
  * @param None
  * @retval None
  */
void Specific_Behaviour_IntroScreen(void)
{
  UARTConsolePrint("\r\n"
                   "***************************************************\r\n"
                   "Specific Behaviour \r\n"
                   "***************************************************\r\n"
                   "16 word align             : 1\r\n"
                   "Write in protected area   : 2\r\n"

                   "Enter any other option to return to Main Menu!\r\n"
                   "***************************************************\r\n\n"

                   "Enter Command Number : \r\n");
}

/**
  * @brief  Helper function to transmit a string over UART console.
  * @param  puartmsg: pointer to null-terminated message
  * @retval HAL status returned by HAL_UART_Transmit
  */
hal_status_t UARTConsolePrint(char *puartmsg)
{
  return HAL_UART_Transmit(BASIC_STDIO_UART_GETHANDLE(), (uint8_t *)puartmsg, (uint16_t)strlen(puartmsg), 500);
}

/**
  * @brief  UART receive-complete callback.
  *         Decodes the received byte to update IT_receive and User_Choice state,
  *         and displays corresponding menus.
  * @param  huart     UART handle
  * @param  size_byte Number of bytes received (unused)
  * @param  rx_event  RX event type (unused)
  * @retval None
  */
void HAL_UART_RxCpltCallback(hal_uart_handle_t *huart, uint32_t size_byte, hal_uart_rx_event_types_t rx_event)
{
  /*
   * Callback policy:
   * - Decode one-byte menu key and map it to an execution path.
   * - Use IT_receive as a deferred-work selector for main loop processing.
   * - Keep IRQ-time logic short: no heavy EEPROM operation here.
   * - Disable USART IRQ only when a full request is pending to avoid re-entry.
   *
   * Menu key map from main screen:
   * - '1' => Command set parser.
   * - '2' => User scenario menu.
   * - '3' => Specific behaviour menu.
   * - '4' => Regression tests.
   * - '*' => Reprint banner/main menu.
   */
  if (User_Choice == 1)
  {
    /* Command Set already selected: next byte is interpreted as command line start */
    IT_receive = 1;
  }
  else if (User_Choice == 2)
  {
    /* User Scenario already selected */
    IT_receive = 2;
  }
  else if (User_Choice == 3)
  {
    /* Specific Behaviour already selected */
    IT_receive = 3;
  }
  else if (User_Choice == 4)
  {
    /* Run Test Cases already selected */
    IT_receive = 4;
  }
  else
  {
    /* No scenario previously selected: interpret single key from main menu */
    char MajoBuffer[40];

    if (aRxBuffer[0] == 0x2AU) /* '*' character: display banner and re-show main menu */
    {
      UartReady = 0;
      aRxBuffer[0] = 0;
      sprintf(MajoBuffer, "STM32L0_nucleo_reader\r\n");

      UARTConsolePrint(MajoBuffer);
      IT_receive = 0;

      if (HAL_UART_Receive_IT(BASIC_STDIO_UART_GETHANDLE(), (uint8_t *)aRxBuffer, 1) == HAL_OK)
      {
        IntroScreen();
      }

    }
    else if (aRxBuffer[0] == 0x31U)  /* '1': Command Set menu */
    {
      IT_receive = 1;
      printf("\nEnter Command set: \r\n");

    }
    else if (aRxBuffer[0] == 0x32U)  /* '2': User Scenario menu */
    {
      IT_receive = 2;
      User_Scenario_IntroScreen();

    }
    else if (aRxBuffer[0] == 0x33U)  /* '3': Specific Behaviour menu */
    {
      IT_receive = 3;
      Specific_Behaviour_IntroScreen();

    }
    else if (aRxBuffer[0] == 0x34U)  /* '4': Run Test Cases */
    {
      IT_receive = 4;

    }
    else
    {
      /* Unknown selection from main menu: prompt again */
      UARTConsolePrint("\r\nEnter correct option!\r\n");
      UartReady = 0;
      aRxBuffer[0] = 0;

      if (HAL_UART_Receive_IT(BASIC_STDIO_UART_GETHANDLE(), (uint8_t *)aRxBuffer, 1) == HAL_OK)
      {
        IntroScreen();
      }
    }
  }

  /* If a high-level processing path was selected, disable USART interrupt
     until the application finishes handling the request */
  if (IT_receive != 0)
  {
    HAL_CORTEX_NVIC_DisableIRQ(USART2_IRQn);
  }
}

/**
  * @brief  UART transmit-complete callback.
  *         Sets flag used by application to track TX completion.
  * @param  UartHandle: UART handle
  * @retval None
  */
void HAL_UART_TxCpltCallback(hal_uart_handle_t *UartHandle)
{
  /* Signal that UART transmission is complete */
  UartReady = 1;
}
