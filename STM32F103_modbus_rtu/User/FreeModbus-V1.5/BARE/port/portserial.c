/*
 * FreeModbus Libary: BARE Port
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id: portserial.c,v 1.1 2006/08/22 21:35:13 wolti Exp $
 */

#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

/* ----------------------- static functions ---------------------------------*/
static void prvvUARTTxReadyISR( void );
static void prvvUARTRxISR( void );

/* ----------------------- Start implementation -----------------------------*/
void
vMBPortSerialEnable( BOOL xRxEnable, BOOL xTxEnable )
{

        /* If xRXEnable enable serial receive interrupts. If xTxENable enable
         * transmitter empty interrupts.
         */

        /*GXP,ADD,2016年11月9日15:13:15*/

        if(xRxEnable) {
                //使能接收和接收中断
                USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);

                //SP485EE操作 低电平为接收模式
                GPIO_ResetBits(GPIOE,GPIO_Pin_3);
        } else {
                USART_ITConfig(USART3, USART_IT_RXNE, DISABLE);
                //SP485EE操作 高电平为发送模式
                GPIO_SetBits(GPIOE,GPIO_Pin_3);
        }

        if(xTxEnable) {
                //使能发送完成中断
                USART_ITConfig(USART3, USART_IT_TXE, ENABLE);


        } else {
                //禁止发送完成中断
                USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
        }

}

BOOL
xMBPortSerialInit( UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity )
{
        /*GXP,ADD,2016年11月9日15:13:15*/


        GPIO_InitTypeDef  GPIO_InitStructure;
        USART_InitTypeDef USART_InitStructure;
        NVIC_InitTypeDef  NVIC_InitStructure;

        /*释放掉参数*/

        (void)ucPORT; //不修改串口
        (void)ucDataBits; //不修改数据位长度
        (void)eParity; //不修改校验格式


        /* 第1步： 开启GPIO和UART时钟 */
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

        /* 第2步：将USART Tx的GPIO配置为推挽复用模式 */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOB, &GPIO_InitStructure);

        /* 第3步：将USART Rx的GPIO配置为浮空输入模式
        	由于CPU复位后，GPIO缺省都是浮空输入模式，因此下面这个步骤不是必须的
        	但是，我还是建议加上便于阅读，并且防止其它地方修改了这个口线的设置参数
        */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(GPIOB, &GPIO_InitStructure);
        /*  第3步已经做了，因此这步可以不做
        	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        */
        GPIO_Init(GPIOB, &GPIO_InitStructure);

        /* 第4步： 配置串口硬件参数 */
        USART_InitStructure.USART_BaudRate = 9600;	/* 波特率 */
        USART_InitStructure.USART_WordLength = USART_WordLength_8b;
        USART_InitStructure.USART_StopBits = USART_StopBits_1;
        USART_InitStructure.USART_Parity = USART_Parity_No ;
        USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
        USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
        USART_Init(USART3, &USART_InitStructure);

        //设定USART1 中断优先级
        NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;/*FreeRTOS采用分组4条件下这个没有用*/
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);

        //最后配置SP485EE发送和接收模式

        /* 配置 PE3为推挽输出，用于切换 RS485芯片的收发状态 */
        {
                RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);

                GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
                GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
                GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
                GPIO_Init(GPIOE, &GPIO_InitStructure);
        }

        /*
        	USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
        	注意: 不要在此处打开发送中断
        	发送中断使能在SendUart()函数打开
        */
        USART_Cmd(USART3, ENABLE);		/* 使能串口 */

        /* CPU的小缺陷：串口配置好，如果直接Send，则第1个字节发送不出去
        	如下语句解决第1个字节无法正确发送出去的问题 */
        USART_ClearFlag(USART3, USART_FLAG_TC);     /* 清发送完成标志，Transmission Complete flag */

        return TRUE;

}

BOOL
xMBPortSerialPutByte( CHAR ucByte )
{
        /* Put a byte in the UARTs transmit buffer. This function is called
         * by the protocol stack if pxMBFrameCBTransmitterEmpty( ) has been
         * called. */

        /*GXP,ADD,2016年11月9日15:13:15*/

        USART_SendData(USART3, ucByte);		//发送数据

        return TRUE;
}

BOOL
xMBPortSerialGetByte( CHAR * pucByte )
{
        /* Return the byte in the UARTs receive buffer. This function is called
         * by the protocol stack after pxMBFrameCBByteReceived( ) has been called.
         */

        /*GXP,ADD,2016年11月9日15:13:15*/

        *pucByte = USART_ReceiveData(USART3);//接收数据

        return TRUE;
}

/* Create an interrupt handler for the transmit buffer empty interrupt
 * (or an equivalent) for your target processor. This function should then
 * call pxMBFrameCBTransmitterEmpty( ) which tells the protocol stack that
 * a new character can be sent. The protocol stack will then call
 * xMBPortSerialPutByte( ) to send the character.
 */
static void prvvUARTTxReadyISR( void )
{
        pxMBFrameCBTransmitterEmpty(  );
}

/* Create an interrupt handler for the receive interrupt for your target
 * processor. This function should then call pxMBFrameCBByteReceived( ). The
 * protocol stack will then call xMBPortSerialGetByte( ) to retrieve the
 * character.
 */
static void prvvUARTRxISR( void )
{
        pxMBFrameCBByteReceived(  );
}

void USART3_IRQHandler(void)
{
        //发生接收中断
        if(USART_GetITStatus(USART3, USART_IT_RXNE) == SET) {
                prvvUARTRxISR();
                //清除中断标志位
                USART_ClearITPendingBit(USART3, USART_IT_RXNE);
        }

        //发生完成中断
        if(USART_GetITStatus(USART3, USART_IT_TXE) == SET) {
                prvvUARTTxReadyISR();
                //清除中断标志
                USART_ClearITPendingBit(USART3, USART_IT_TXE);
        }
}

