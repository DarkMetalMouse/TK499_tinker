#ifndef __MT_SPI_H
#define __MT_SPI_H

#include "HAL_conf.h"
#include "string.h"
#include "qspi_fun.h"


#define   HSKE_SPI1_TX   0x2
#define   HSKE_SPI1_RX   0x3
#define   HSKE_SPI2_TX   0x4
#define   HSKE_SPI2_RX   0x5

// 4001_3000 ~ 4001_33FF      // APB2

#define QSPI_TXREG        *(__IO uint32_t*)(QSPI_BASE)
#define QSPI_RXREG        *(__IO uint32_t*)(QSPI_BASE+0x4)
#define QSPI_CSTAT        *(__IO uint32_t*)(QSPI_BASE+0x8)
#define QSPI_INTSTAT      *(__IO uint32_t*)(QSPI_BASE+0xc)
#define QSPI_INTEN        *(__IO uint32_t*)(QSPI_BASE+0x10)
#define QSPI_INTCLR       *(__IO uint32_t*)(QSPI_BASE+0x14)
#define QSPI_GCTL         *(__IO uint32_t*)(QSPI_BASE+0x18)
#define QSPI_CCTL         *(__IO uint32_t*)(QSPI_BASE+0x1c)
#define QSPI_SPBRG        *(__IO uint32_t*)(QSPI_BASE+0x20)
#define QSPI_RXDNR        *(__IO uint32_t*)(QSPI_BASE+0x24)
#define QSPI_SCSR         *(__IO uint32_t*)(QSPI_BASE+0x28)
#define QSPI_SCSR         *(__IO uint32_t*)(QSPI_BASE+0x28)
#define QSPI_MODE         *(__IO uint32_t*)(QSPI_BASE+0x2c)

// 4000_3800 ~ 4000_3BFF      // APB1

#define SPI2_TXREG        *(__IO uint32_t*)(SPI2_BASE)
#define SPI2_RXREG        *(__IO uint32_t*)(SPI2_BASE+0x4)
#define SPI2_CSTAT        *(__IO uint32_t*)(SPI2_BASE+0x8)
#define SPI2_INTSTAT      *(__IO uint32_t*)(SPI2_BASE+0xc)
#define SPI2_INTEN        *(__IO uint32_t*)(SPI2_BASE+0x10)
#define SPI2_INTCLR       *(__IO uint32_t*)(SPI2_BASE+0x14)
#define SPI2_GCTL         *(__IO uint32_t*)(SPI2_BASE+0x18)
#define SPI2_CCTL         *(__IO uint32_t*)(SPI2_BASE+0x1c)
#define SPI2_SPBRG        *(__IO uint32_t*)(SPI2_BASE+0x20)
#define SPI2_RXDNR        *(__IO uint32_t*)(SPI2_BASE+0x24)
#define SPI2_SCSR         *(__IO uint32_t*)(SPI2_BASE+0x28)
#define SPI2_EXTLEN       *(__IO uint32_t*)(SPI2_BASE+0x2c)

//Read
#define RD_Com		0x03
#define RD_Adr		0x03
#define FastRD_Com	0x0b
#define FastRD_Dual_Com	0x3b
#define FastRD_Quad_Com	0x6b
#define FastRD_DualIo_Com	0xbb
#define FastRD_QuadIo_Com	0xeb
#define WordRD_QuadIo_Com	0xeb
#define FastRD_Adr	0x03
#define RDID_Com	0x9f
#define RDMANUIDQIO_Com	0x94
#define RDID_Adr	0x0

//Write Control
#define WREN_Com	0x06
#define WREN_Adr	0x00
#define WRDI_Com	0x04
#define WRDI_Adr	0x00

//Erase
#define SE_Com		0x20
#define SE_Adr		0x03
#define BE32K_Com	0x52
#define BE64K_Com	0xd8
#define BE_Adr		0x00
#define CE_Com		0x60

//Program
#define	PP_Com		0x02
#define	PP_Com_Quad	0x32
#define PP_Adr		0x03


//Status Register
#define RDSR_Com	0x05
#define RDSR_Adr	0x00
#define WRSR1_Com	0x01
#define WRSR2_Com	0x31
#define WRSR3_Com	0x11
#define WRSR_Adr	0x00

//Identification
#define ManuID		0xef
#define MemType		0x40
#define MemCap		0x16
#define W25Q32JVID 0x15

#define   APB2CLK   72000000


//INTSTAT
#define     RXMatch_Clr     0x0f
#define     RXOERR_Clr      0x17
#define     UnderRun_Clr    0x1b
#define     RXINT_Clr       0x1d
#define     TXINT_Clr       0x1e

//INTEN
#define     RXMatchEN       0x10
#define     RXOERREN        0x08
#define     UnderRunEN      0x4
#define     RXIEN           0x2
#define     TXIEN           0x1


//GCTL
//#define     Data_8bit       0xcff
//#define     Data_16bit      0x100
//#define     Data_32bit      0x200
#define     Data_8bit       0x7ff
#define     Data_32bit      0x800

//#define     CS_Low		0xf7f
//#define     CS_High		0x80
#define     CS_Low		0xFE
#define     CS_High		0x01

#define     CSC_SEL     0x40
//#define     DMA_EN     0x20
#define     DMA_EN     0x200
#define     DMA_DIS    0xdff
#define     RXEN        0x10
#define     TXEN        0x8
//#define     TXDIS           0xf17
#define     TXDIS           0xff7
#define     RXDIS           0xfef


#define     RXDMA_EN    0x800
#define     TXDMA_EN    0x400
#define     TXDMA_Dis   0xfbff
#define     RXDMA_Dis   0xf7ff
#define     MM          0x4
#define     INTEN      0x2
#define     QSPIEN       0x1



//cctl
#define   CKPL_Low    0x0
#define   CKPL_High   0x2
#define   CKPH_Sam1Tran2_Edge 0x1
#define   CKPH_Sam2Tran1_Edge  0x0

#define   Len_8bit      0x8
#define   Len_7bit      0x7

#define   MSB_First     0xb
#define   LSB_First     0x4

#define   Txtlf         0x80
#define   Rxtlf         0x20
#define   Dis_Txtlf     0xFFFFFF3F
#define   Dis_Rxtlf     0xFFFFFF9F

#define   SRAM_BASE2    SRAM_BASE
#define   SRAM_BASE1    SRAM_BASE
#define   SRAM_BASE0    SRAM_BASE

int QSPItest(void);
void QSPI_InitasMode0(void);
void QspiFlashProgram(u32 qSpiFlashAdd, u8 *inData, u32 len);
void QspiFlashRead(u32 qspiAdd, u8 *puladdress, u32 len);
unsigned long  testchipEraseFunction(void);
void QspiFlashSectorErase(u32 add); 
void QSPIReadGroup(unsigned long address,u8* buf,uint32_t datanum);
void QSPI_Block64Erase(unsigned long address);
void qspiRecDMA_Config(u32 adress,u32 len);
#endif 

