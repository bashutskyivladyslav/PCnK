/*******************************************************************************
* File Name: CyBUS_CLK.h
* Version 2.20
*
*  Description:
*   Provides the function and constant definitions for the clock component.
*
*  Note:
*
********************************************************************************
* Copyright 2008-2012, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_CLOCK_CyBUS_CLK_H)
#define CY_CLOCK_CyBUS_CLK_H

#include <cytypes.h>
#include <cyfitter.h>


/***************************************
*        Function Prototypes
***************************************/
#if defined CYREG_PERI_DIV_CMD

void CyBUS_CLK_StartEx(uint32 alignClkDiv);
#define CyBUS_CLK_Start() \
    CyBUS_CLK_StartEx(CyBUS_CLK__PA_DIV_ID)

#else

void CyBUS_CLK_Start(void);

#endif/* CYREG_PERI_DIV_CMD */

void CyBUS_CLK_Stop(void);

void CyBUS_CLK_SetFractionalDividerRegister(uint16 clkDivider, uint8 clkFractional);

uint16 CyBUS_CLK_GetDividerRegister(void);
uint8  CyBUS_CLK_GetFractionalDividerRegister(void);

#define CyBUS_CLK_Enable()                         CyBUS_CLK_Start()
#define CyBUS_CLK_Disable()                        CyBUS_CLK_Stop()
#define CyBUS_CLK_SetDividerRegister(clkDivider, reset)  \
    CyBUS_CLK_SetFractionalDividerRegister((clkDivider), 0u)
#define CyBUS_CLK_SetDivider(clkDivider)           CyBUS_CLK_SetDividerRegister((clkDivider), 1u)
#define CyBUS_CLK_SetDividerValue(clkDivider)      CyBUS_CLK_SetDividerRegister((clkDivider) - 1u, 1u)


/***************************************
*             Registers
***************************************/
#if defined CYREG_PERI_DIV_CMD

#define CyBUS_CLK_DIV_ID     CyBUS_CLK__DIV_ID

#define CyBUS_CLK_CMD_REG    (*(reg32 *)CYREG_PERI_DIV_CMD)
#define CyBUS_CLK_CTRL_REG   (*(reg32 *)CyBUS_CLK__CTRL_REGISTER)
#define CyBUS_CLK_DIV_REG    (*(reg32 *)CyBUS_CLK__DIV_REGISTER)

#define CyBUS_CLK_CMD_DIV_SHIFT          (0u)
#define CyBUS_CLK_CMD_PA_DIV_SHIFT       (8u)
#define CyBUS_CLK_CMD_DISABLE_SHIFT      (30u)
#define CyBUS_CLK_CMD_ENABLE_SHIFT       (31u)

#define CyBUS_CLK_CMD_DISABLE_MASK       ((uint32)((uint32)1u << CyBUS_CLK_CMD_DISABLE_SHIFT))
#define CyBUS_CLK_CMD_ENABLE_MASK        ((uint32)((uint32)1u << CyBUS_CLK_CMD_ENABLE_SHIFT))

#define CyBUS_CLK_DIV_FRAC_MASK  (0x000000F8u)
#define CyBUS_CLK_DIV_FRAC_SHIFT (3u)
#define CyBUS_CLK_DIV_INT_MASK   (0xFFFFFF00u)
#define CyBUS_CLK_DIV_INT_SHIFT  (8u)

#else 

#define CyBUS_CLK_DIV_REG        (*(reg32 *)CyBUS_CLK__REGISTER)
#define CyBUS_CLK_ENABLE_REG     CyBUS_CLK_DIV_REG
#define CyBUS_CLK_DIV_FRAC_MASK  CyBUS_CLK__FRAC_MASK
#define CyBUS_CLK_DIV_FRAC_SHIFT (16u)
#define CyBUS_CLK_DIV_INT_MASK   CyBUS_CLK__DIVIDER_MASK
#define CyBUS_CLK_DIV_INT_SHIFT  (0u)

#endif/* CYREG_PERI_DIV_CMD */

#endif /* !defined(CY_CLOCK_CyBUS_CLK_H) */

/* [] END OF FILE */
