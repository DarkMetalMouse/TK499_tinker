/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */
#include "sdio_sdcard.h"
extern void UartSendGroup(u8* buf,u16 len);
extern char prinfBuf[];
//#include "Virtual_Disk.h"

/* Definitions of physical drive number for each drive */
#define SDRAM		0	/* Example: Map Ramdisk to physical drive 0 */
#define DEV_MMC		1	/* Example: Map MMC/SD card to physical drive 1 */
#define DEV_USB		2	/* Example: Map USB MSD to physical drive 2 */

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{ 
    
	return RES_OK;
    
} 



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	unsigned char res;

	switch (pdrv) {
	case 0 :
	return RES_OK;
	case 1 :
	return RES_OK;         
	case 2 :
	return RES_OK;         
	case 3 :
	return RES_OK;
	default:
	return STA_NOINIT;
	} 
	return res;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res;
	int result;

	switch (pdrv) {
		case 0 :
						res = SD_ReadDisk((u8*)buff,sector,count);
						
						return RES_OK;
		case 1 :
						return RES_OK;
		case 2 :
						return RES_OK;
	}
	return res;
	//return res;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res;
	int result;

	switch (pdrv) {
	case 0 :
			res = SD_WriteDisk((u8*)buff,sector,count);
			
      break;
	}

	return res;
}



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
        BYTE drv,                /* Physical drive nmuber (0..) */
        BYTE ctrl,                /* Control code */
        void *buff                /* Buffer to send/receive control data */
)
{
	if (drv==0)
	{   
		switch (ctrl)
		{
				case CTRL_SYNC :
								return RES_OK;
				case GET_SECTOR_COUNT :
					*(DWORD*)buff = 243*1024*1024/512;
				return RES_OK;
				case GET_BLOCK_SIZE :
					*(WORD*)buff = 512;
				return RES_OK;        
				case CTRL_POWER :
								break;
				case CTRL_LOCK :
								break;
				case CTRL_EJECT :
								break;
/* MMC/SDC command */
				case MMC_GET_TYPE :
								break;
				case MMC_GET_CSD :
								break;
				case MMC_GET_CID :
								break;
				case MMC_GET_OCR :
								break;
				case MMC_GET_SDSTAT :
								break;        
		}
		}else if(drv==1){
		switch (ctrl)
		{
				case CTRL_SYNC :
								return RES_OK;
				case GET_SECTOR_COUNT :
				return RES_OK;
				case GET_SECTOR_SIZE :
								return RES_OK;
				case GET_BLOCK_SIZE :
				return RES_OK;        
				case CTRL_POWER :
								break;
				case CTRL_LOCK :
								break;
				case CTRL_EJECT :
								break;
/* MMC/SDC command */
				case MMC_GET_TYPE :
								break;
				case MMC_GET_CSD :
								break;
				case MMC_GET_CID :
								break;
				case MMC_GET_OCR :
								break;
				case MMC_GET_SDSTAT :
								break;        
		}         
	}
	else{                                 
					return RES_PARERR;  
	}
	return RES_PARERR;
}


DWORD get_fattime (void)
{				 
	return 0;
}		
