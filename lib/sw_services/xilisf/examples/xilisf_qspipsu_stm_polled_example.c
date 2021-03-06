/******************************************************************************
* Copyright (c) 2012 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xilisf_qspipsu_flash_polled_example.c
*
*
* This file contains a design example using the XILISF Library in
* interrupt mode with a serial FLASH device. This examples performs
* some transfers in Auto mode and Manual start mode, to illustrate the modes
* available.
* The hardware which this example runs on, must have a serial FLASH (Numonyx
* N25Q, Winbond W25Q, or Spansion S25FL) for it to run. This example has been
* tested with the Numonyx Serial Flash (N25Q128).
*
* @note
*
* None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------
* 5.4   sk  08/07/15 First Release.
* 5.7   rk  07/27/16 Added the dummy bytecnt other than normal reads.
* 5.14  akm 08/01/19 Initialized Status variable to XST_FAILURE.
*
*</pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"	/* EDK generated parameters */
#include "xscugic.h"		/* Interrupt controller device driver */
#include "xil_exception.h"
#include "xil_printf.h"
#include "xilisf.h"             /* Serial Flash Library header file */

/************************** Constant Definitions *****************************/
/** @name Device ID's
 *
 * @{
 */
/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define QSPI_DEVICE_ID		XPAR_XQSPIPSU_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define QSPI_INTR_ID		XPAR_XQSPIPS_0_INTR
/*@}*/

/**
 * The following constants define the offsets within a FlashBuffer data
 * type for each kind of data.  Note that the read data offset is not the
 * same as the write data because the QSPI driver is designed to allow full
 * duplex transfers such that the number of bytes received is the number
 * sent and received.
 */
#define DATA_OFFSET	5 /**< Data byte offset for fast, dual
							and quad reads */
#define DUMMY_SIZE	1 /**< Number of dummy bytes for fast,
						dual and quad reads */

/**
 * The following constants specify the page size and number of
 * pages for the FLASH.  The page size specifies a max number of
 * bytes that can be written to the FLASH with a single transfer.
 */
#define NUM_PAGES	0x10000	/**< Number of Pages in the flash */
#define PAGE_SIZE	256	/**< Page Size for Read/Write Operation */

/**
 * Number of flash pages to be written.
 */
#define PAGE_COUNT	2	/**< Number of Pages for r/w Operation */

/*
 * Max page size to initialize write and read buffer
 */
#define MAX_PAGE_SIZE 1024

/**
 * Flash address to which data is ot be written.
 */
#define TEST_ADDRESS	0x1000000	/**< Test Address in the flash  */
#define UNIQUE_VALUE	0x08		/**< Unique Value for Test */


/**
 * The following constants specify the max amount of data and the size of the
 * the buffer required to hold the data and overhead to transfer the data to
 * and from the FLASH.
 */
#define MAX_DATA	PAGE_COUNT * PAGE_SIZE	/**< Max Data Calculated by
					multiplying Page count and Page Size */

/**
 * The following constant defines the slave select signal that is used to
 * to select the FLASH device on the QSPI bus, this signal is typically
 * connected to the chip select of the device
 */
#define FLASH_QSPI_SELECT		0x01	/**< Interrupt Mode Enable */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

int FlashErase(XIsf *InstancePtr, u32 Address, u32 ByteCount);
int FlashWrite(XIsf *InstancePtr, u32 Address, u32 ByteCount,
							u8 Command);
int FlashRead(XIsf *InstancePtr, u32 Address, u32 ByteCount,
							u8 Command);
int QspiFlashPollExample(XScuGic *IntcInstancePtr, XQspiPsu *QspiInstancePtr,
			 u16 QspiDeviceId, u16 QspiIntrId);
static u32 SectorMask(u32 SectorSize);

/************************** Variable Definitions *****************************/

/**
 * The instances to support the device drivers are global such that they
 * are initialized to zero each time the program runs. They could be local
 * but should at least be static so they are zeroed.
 */
static XScuGic IntcInstance;
static XQspiPsu QspiInstance;
static XIsf Isf;
static XQspiPsu_Config *ConfigPtr;     /**< Pointer to Configuration ROM data */

/**
 * The following variables are shared between non-interrupt processing and
 * interrupt processing such that they must be global.
 */
volatile int TransferInProgress;

/**
 * The following variable allows a test value to be added to the values that
 * are written to the FLASH such that unique values can be generated to
 * guarantee the writes to the FLASH were successful
 */
int Test_Polled = 7;

/**
 * The following variables are used to read and write to the eeprom and they
 * are global to avoid having large buffers on the stack
 */
u8 ReadBuffer[(PAGE_COUNT * MAX_PAGE_SIZE) + (DATA_OFFSET + DUMMY_SIZE)*8] __attribute__ ((aligned(64)));
u8 WriteBuffer[(PAGE_COUNT * MAX_PAGE_SIZE) + DATA_OFFSET];
u8 IsfWriteBuffer[PAGE_SIZE + XISF_CMD_SEND_EXTRA_BYTES];/**< IsfWrite Buffer
					used in XilISF Initialization */

/*****************************************************************************/
/**
*
* Main function to call the QSPI Flash example.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None
*
******************************************************************************/
int main(void)
{
	int Status = XST_FAILURE;

	xil_printf("QSPIPSU FLASH Polling Example Test \r\n");

	/*
	 * Run the Qspipsu Interrupt example.
	 */
	Status = QspiFlashPollExample(&IntcInstance, &QspiInstance,
				      QSPI_DEVICE_ID, QSPI_INTR_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("QSPIPSU FLASH Polling Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran QSPIPSU FLASH Polling Example Test\r\n");
	return XST_SUCCESS;
}


/****************************************************************************/
/**
* The purpose of this function is to illustrate how to use the XQspiPsu
* device driver in interrupt mode. This function writes and reads data
* from a serial FLASH.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note
*
* This function calls other functions which contain loops that may be infinite
* if interrupts are not working such that it may not return. If the device
* slave select is not correct and the device is not responding on bus it will
* read a status of 0xFF for the status register as the bus is pulled up.
*
*****************************************************************************/
int QspiFlashPollExample(XScuGic *IntcInstancePtr, XQspiPsu *QspiInstancePtr,
			 u16 QspiDeviceId, u16 QspiIntrId)
{
	u8 *BufferPtr;
	u8 UniqueValue;
	int Count;
	int Page;
	int Status = XST_FAILURE;
	u32 Options;

	/*
	* Lookup the device configuration in the temporary CROM table. Use this
	* configuration info down below when initializing this component.
	*/
	ConfigPtr = XQspiPsu_LookupConfig(QspiDeviceId);
	if (ConfigPtr == NULL) {
		return XST_DEVICE_NOT_FOUND;
	}

	Status = XQspiPsu_CfgInitialize(QspiInstancePtr, ConfigPtr,
			ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Set the QSPI options
	 */
	Options = XQSPIPSU_MANUAL_START_OPTION;
	XIsf_SetSpiConfiguration(&Isf, QspiInstancePtr, Options,
			XISF_SPI_PRESCALER);

	/* Initialize the XILISF Library */
	XIsf_Initialize(&Isf, QspiInstancePtr, FLASH_QSPI_SELECT,
				   IsfWriteBuffer);

	/*
	 * Initialize the write buffer for a pattern to write to the FLASH
	 * and the read buffer to zero so it can be verified after the read,
	 * the test value that is added to the unique value allows the value
	 * to be changed in a debug environment to guarantee
	 */
	for (UniqueValue = UNIQUE_VALUE, Count = 0; Count < PAGE_SIZE;
		Count++, UniqueValue++) {
		WriteBuffer[Count] = (u8)(UniqueValue + Test_Polled);
	}
	memset(ReadBuffer, 0x00, sizeof(ReadBuffer));

	Status = FlashErase(&Isf, TEST_ADDRESS, MAX_DATA);
	if(Status != XST_SUCCESS){
		return XST_FAILURE;
	}

	/*
	 * Write the data in the write buffer to the serial FLASH a page at a
	 * time, starting from TEST_ADDRESS
	 */

	for (Page = 0; Page < PAGE_COUNT; Page++) {
		Status = FlashWrite(&Isf,
			(Page * PAGE_SIZE) + TEST_ADDRESS, PAGE_SIZE,
				XISF_WRITE);
		if(Status != XST_SUCCESS){
			return XST_FAILURE;
		}
	}

	/******************************************************
	 ******************QUAD OP FAST READ*******************
	 ******************************************************/

	/*
	 * Read the contents of the FLASH from TEST_ADDRESS, using QUAD IO
	 * Fast Read command
	 */
	Status = FlashRead(&Isf, TEST_ADDRESS, MAX_DATA,
						XISF_QUAD_OP_FAST_READ);
	if(Status != XST_SUCCESS){
		return XST_FAILURE;
	}

	/*
	 * Setup a pointer to the start of the data that was read into the read
	 * buffer and verify the data read is the data that was written
	 */

	BufferPtr = ReadBuffer;
	for (UniqueValue = UNIQUE_VALUE, Count = 0; Count < MAX_DATA;
			Count++, UniqueValue++) {
		if (BufferPtr[Count] != (u8)(UniqueValue + Test_Polled)) {
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}



/*****************************************************************************/
/**
*
* This function writes to the  serial FLASH connected to the QSPIPSU interface.
* The FLASH contains a 256 byte write buffer which can be filled and then a
* write is automatically performed by the device.  All the data put into the
* buffer must be in the same page of the device with page boundaries being on
* 256 byte boundaries.
*
* @param	InstancePtr is a pointer to the XIsf component to use.
* @param	Address contains the address to write data to in the FLASH.
* @param	ByteCount contains the number of bytes to write.
* @param	Command is the command used to write data to the flash.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
int FlashWrite(XIsf *InstancePtr, u32 Address, u32 ByteCount,
								u8 Command)
{
	XIsf_WriteParam WriteParam;

	int Status = XST_FAILURE;

	WriteParam.Address = Address;
	WriteParam.NumBytes = ByteCount;
	WriteParam.WritePtr = WriteBuffer;

	/*
	 * Perform the Write operation.
	 */
	Status = XIsf_Write(&Isf, Command, (void*) &WriteParam);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function reads from the  serial FLASH connected to the
* QSPIPSU interface.
*
* @param	InstancePtr is a pointer to the XIsf component to use.
* @param	Address contains the address to read data from in the FLASH.
* @param	ByteCount contains the number of bytes to read.
* @param	Command is the command used to read data from the flash.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
int FlashRead(XIsf *InstancePtr, u32 Address, u32 ByteCount, u8 Command)
{
	XIsf_ReadParam ReadParam;
	int Status = XST_FAILURE;

	/*
	 * Set the
	 * - Address in the Serial Flash where the data is to be read from.
	 * - Number of bytes ReadParam.NumDummyBytesto be read from the Serial Flash.
	 * - Read Buffer to which the data is to be read.
	 */
	ReadParam.Address = Address;
	ReadParam.NumBytes = ByteCount;
	ReadParam.ReadPtr = ReadBuffer;
	if(Command != XISF_READ){
		ReadParam.NumDummyBytes = 1;
	}

	/*
	 * Perform the Read operation.
	 */
	Status = XIsf_Read(&Isf, Command, (void*) &ReadParam);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function erases the sectors in the  serial FLASH connected to the
* QSPIPSU interface.
*
* @param	InstancePtr is a pointer to the XIsf component to use.
* @param	Address contains the address of the first sector which needs to
*			be erased.
* @param	ByteCount contains the total size to be erased.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
int FlashErase(XIsf *InstancePtr, u32 Address, u32 ByteCount)
{
	int Status = XST_FAILURE;
	int Sector;
	u32 NumSect;
	u32 SectorSize;
	u32 NumSectors;
	u32 Sector_Mask;
	/*
	 * Get the value of Sector Size and Number of Sectors for the flash
	 */
	SectorSize = Isf.SectorSize;
	NumSectors = Isf.NumSectors;

	/* Get the sector mask value */
	Sector_Mask = SectorMask(SectorSize);

	/*
	 * If erase size is same as the total size of the flash, use bulk erase
	 * command
	 */
	if (ByteCount == (NumSectors * SectorSize)) {

		if(InstancePtr->SpiInstPtr->Config.ConnectionMode == XQSPIPSU_CONNECTION_MODE_STACKED){
			XQspiPsu_SelectFlash(InstancePtr->SpiInstPtr,
				XQSPIPSU_SELECT_FLASH_CS_LOWER, XQSPIPSU_SELECT_FLASH_BUS_LOWER);
		}

		/*
		 * Call Bulk erase
		 */
		Status = XIsf_Erase(InstancePtr,
					XISF_BULK_ERASE, Address);
		if(Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		if(InstancePtr->SpiInstPtr->Config.ConnectionMode == XQSPIPSU_CONNECTION_MODE_STACKED){

			XQspiPsu_SelectFlash(InstancePtr->SpiInstPtr,
				XQSPIPSU_SELECT_FLASH_CS_UPPER, XQSPIPSU_SELECT_FLASH_BUS_LOWER);

			/*
			 * Call Bulk erase
			 */
			Status = XIsf_Erase(InstancePtr,
						XISF_BULK_ERASE, Address);
			if(Status != XST_SUCCESS) {
				return XST_FAILURE;
			}
		}
		return Status;
	}

	/*
	 * Calculate no. of sectors to erase based on byte count
	 */
	NumSect = ByteCount/SectorSize + 1;

	/*
	 * If ByteCount to k sectors,
	 * but the address range spans from N to N+k+1 sectors, then
	 * increment no. of sectors to be erased
	 */

	if( ((Address + ByteCount) & Sector_Mask) ==
			((Address + (NumSect * SectorSize)) &
					Sector_Mask) ) {
		NumSect++;
	}

	/*
	 * If the erase size is less than the total size of the flash, use
	 * sector erase command
	 */
	for (Sector = 0; Sector < NumSect; Sector++) {

		/*
		 * Perform the Sector Erase operation.
		 */
		Status = XIsf_Erase(InstancePtr, XISF_SECTOR_ERASE, Address);
		if(Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Address += SectorSize;
	}
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function calculates the sector mask based upon the sector size value
*
*
* @param	SectorSize is the size of the sector of the flash
* 			available on the board.
*
* @return	will return the sector mask after calculation.
*
* @note		None.
*
******************************************************************************/
u32 SectorMask(u32 SectorSize){

	u32 Mask;

	switch(SectorSize){
	case 0x10000:
				Mask = 0xFFFF0000;
				break;

	case 0x20000:
				Mask = 0xFFFE0000;
				break;
	case 0x40000:
				Mask = 0xFFFC0000;
				break;

	case 0x80000:
				Mask = 0xFFF80000;
				break;

	default:
			break;
	}

	return Mask;
}
