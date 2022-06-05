/*****< audiocfg.h >**********************************************************/
/*      Copyright 2013 Stonestreet One.                                      */
/*      All Rights Reserved.                                                 */
/*                                                                            */
/*      Copyright 2015 Texas Instruments Incorporated.                        */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  AUDIOCFG - I2C and I2S interface configuration for audio CODEC.          */
/*                                                                           */
/*  Author:  Marcus Funk                                                     */
/*                                                                           */
/*** MODIFICATION HISTORY ****************************************************/
/*                                                                           */
/*   mm/dd/yy  F. Lastname    Description of Modification                    */
/*   --------  -----------    -----------------------------------------------*/
/*   10/01/13  M. Funk        Initial creation.                              */
/*****************************************************************************/
#ifndef AUDIOCFG_H_
#define AUDIOCFG_H_

#include "stm32f4xx.h"           /* STM32F register definitions.              */
#include "stm32f4xx_gpio.h"      /* STM32F GPIO control functions.            */
#include "stm32f4xx_rcc.h"       /* STM32F Clock control functions.           */
#include "stm32f4xx_i2c.h"       /* STM32F I2C control functions.             */
#include "stm32f4xx_spi.h"       /* STM32F SPI and I2S control functions.     */


#define AUDIO_I2C                   1

#define AUDIO_I2C_SDA_PORT          B
#define AUDIO_I2C_SDA_PIN           9

#define AUDIO_I2C_SCL_PORT          B
#define AUDIO_I2C_SCL_PIN           6

#define AUDIO_I2S                   2

#define AUDIO_I2S_WS_PORT           I
#define AUDIO_I2S_WS_PIN            0

#define AUDIO_I2S_SCK_PORT          I
#define AUDIO_I2S_SCK_PIN           1

#define AUDIO_I2S_SDO_PORT          I
#define AUDIO_I2S_SDO_PIN           3

#define AUDIO_I2S_MCK_PORT          C
#define AUDIO_I2S_MCK_PIN           6

#define AUDIO_I2S_DMA_TX_NUMBER     1
#define AUDIO_I2S_DMA_TX_STREAM     4
#define AUDIO_I2S_DMA_TX_CHANNEL    0

/************************************************************************/
/* !!!DO NOT MODIFY PAST THIS POINT!!!                                  */
/************************************************************************/

   /* The following section builds the macros that can be used with the */
   /* STM32F standard peripheral libraries based on the above           */
   /* configuration.                                                    */

/* Standard C style concatenation macros                             */
#define DEF_CONCAT2(_x_, _y_)          __DEF_CONCAT2__(_x_, _y_)
#define __DEF_CONCAT2__(_x_, _y_)      _x_ ## _y_

#define DEF_CONCAT3(_x_, _y_, _z_)     __DEF_CONCAT3__(_x_, _y_, _z_)
#define __DEF_CONCAT3__(_x_, _y_, _z_) _x_ ## _y_ ## _z_

   /* I2C peripheral mapping.                                           */
#define AUDIO_I2C_BASE                   (DEF_CONCAT2(I2C, AUDIO_I2C))
#define AUDIO_I2C_RCC_PERIPH_CLK_BIT     (DEF_CONCAT2(RCC_APB1Periph_I2C, AUDIO_I2C))
#define AUDIO_I2C_GPIO_AF                (DEF_CONCAT2(GPIO_AF_I2C, AUDIO_I2C))

   /* I2S peripheral mapping.                                           */
#define AUDIO_I2S_BASE                   (DEF_CONCAT2(SPI, AUDIO_I2S))
#define AUDIO_I2S_IRQ                    (DEF_CONCAT3(SPI, AUDIO_I2S, _IRQn))
#define AUDIO_I2S_IRQ_HANDLER            (DEF_CONCAT3(SPI, AUDIO_I2S, _IRQHandler))
#define AUDIO_I2S_RCC_PERIPH_CLK_BIT     (DEF_CONCAT2(RCC_APB1Periph_SPI, AUDIO_I2S))
#define AUDIO_I2S_GPIO_AF                (DEF_CONCAT2(GPIO_AF_SPI, AUDIO_I2S))

   /* GPIO mapping.                                                     */
#define AUDIO_I2C_SDA_GPIO_PORT          (DEF_CONCAT2(GPIO, AUDIO_I2C_SDA_PORT))
#define AUDIO_I2C_SCL_GPIO_PORT          (DEF_CONCAT2(GPIO, AUDIO_I2C_SCL_PORT))

#define AUDIO_I2S_WS_GPIO_PORT           (DEF_CONCAT2(GPIO, AUDIO_I2S_WS_PORT))
#define AUDIO_I2S_SCK_GPIO_PORT          (DEF_CONCAT2(GPIO, AUDIO_I2S_SCK_PORT))
#define AUDIO_I2S_SDO_GPIO_PORT          (DEF_CONCAT2(GPIO, AUDIO_I2S_SDO_PORT))
#define AUDIO_I2S_MCK_GPIO_PORT          (DEF_CONCAT2(GPIO, AUDIO_I2S_MCK_PORT))

#define AUDIO_I2C_SDA_GPIO_AHB_BIT       (DEF_CONCAT2(RCC_AHB1Periph_GPIO, AUDIO_I2C_SDA_PORT))
#define AUDIO_I2C_SCL_GPIO_AHB_BIT       (DEF_CONCAT2(RCC_AHB1Periph_GPIO, AUDIO_I2C_SCL_PORT))

#define AUDIO_I2S_WS_GPIO_AHB_BIT        (DEF_CONCAT2(RCC_AHB1Periph_GPIO, AUDIO_I2S_WS_PORT))
#define AUDIO_I2S_SCK_GPIO_AHB_BIT       (DEF_CONCAT2(RCC_AHB1Periph_GPIO, AUDIO_I2S_SCK_PORT))
#define AUDIO_I2S_SDO_GPIO_AHB_BIT       (DEF_CONCAT2(RCC_AHB1Periph_GPIO, AUDIO_I2S_SDO_PORT))
#define AUDIO_I2S_MCK_GPIO_AHB_BIT       (DEF_CONCAT2(RCC_AHB1Periph_GPIO, AUDIO_I2S_MCK_PORT))

   /* DMA Mapping.                                                      */
/*
#define AUDIO_I2S_TX_DMA_AHB_BIT         (DEF_CONCAT2(RCC_AHB1Periph_DMA, AUDIO_I2S_DMA_TX_NUMBER))
#define AUDIO_I2S_TX_DMA_CHANNEL         (DEF_CONCAT2(RCC_AHB1Periph_DMA, AUDIO_I2S_DMA_TX_CHANNEL))
#define AUDIO_I2S_TX_DMA_STREAM          (DEF_CONCAT2(DEF_CONCAT3(DMA, AUDIO_I2S_DMA_TX_NUMBER, _Stream), AUDIO_I2S_DMA_TX_STREAM))

#define AUDIO_I2S_TX_DMA_FLAG_TCIF       (DEF_CONCAT2(DMA_FLAG_TCIF, AUDIO_I2S_DMA_TX_STREAM))
#define AUDIO_I2S_TX_DMA_FLAG_HTIF       (DEF_CONCAT2(DMA_FLAG_HTIF, AUDIO_I2S_DMA_TX_STREAM))
#define AUDIO_I2S_TX_DMA_FLAG_TEIF       (DEF_CONCAT2(DMA_FLAG_TEIF, AUDIO_I2S_DMA_TX_STREAM))
#define AUDIO_I2S_TX_DMA_FLAG_DMEIF      (DEF_CONCAT2(DMA_FLAG_DMEIF, AUDIO_I2S_DMA_TX_STREAM))
#define AUDIO_I2S_TX_DMA_FLAG_FEIF       (DEF_CONCAT2(DMA_FLAG_FEIF, AUDIO_I2S_DMA_TX_STREAM))

#define AUDIO_I2S_TX_IRQ                 (DEF_CONCAT3(DEF_CONCAT3(DMA, AUDIO_I2S_DMA_TX_NUMBER, _Stream), AUDIO_I2S_DMA_TX_STREAM, _IRQn))
#define AUDIO_I2S_TX_IRQHandler          (DEF_CONCAT3(DEF_CONCAT3(DMA, AUDIO_I2S_DMA_TX_NUMBER, _Stream), AUDIO_I2S_DMA_TX_STREAM, _IRQHandler))

#define AUDIO_I2S_DR_REGISTER_ADDRESS    (((unsigned int)(DEF_CONCAT3(SPI, AUDIO_I2S, _BASE))) + 0x0C)
*/
#endif
