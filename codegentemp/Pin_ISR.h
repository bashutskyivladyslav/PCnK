/*******************************************************************************
* File Name: Pin_ISR.h  
* Version 2.20
*
* Description:
*  This file contains Pin function prototypes and register defines
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_PINS_Pin_ISR_H) /* Pins Pin_ISR_H */
#define CY_PINS_Pin_ISR_H

#include "cytypes.h"
#include "cyfitter.h"
#include "Pin_ISR_aliases.h"


/***************************************
*     Data Struct Definitions
***************************************/

/**
* \addtogroup group_structures
* @{
*/
    
/* Structure for sleep mode support */
typedef struct
{
    uint32 pcState; /**< State of the port control register */
    uint32 sioState; /**< State of the SIO configuration */
    uint32 usbState; /**< State of the USBIO regulator */
} Pin_ISR_BACKUP_STRUCT;

/** @} structures */


/***************************************
*        Function Prototypes             
***************************************/
/**
* \addtogroup group_general
* @{
*/
uint8   Pin_ISR_Read(void);
void    Pin_ISR_Write(uint8 value);
uint8   Pin_ISR_ReadDataReg(void);
#if defined(Pin_ISR__PC) || (CY_PSOC4_4200L) 
    void    Pin_ISR_SetDriveMode(uint8 mode);
#endif
void    Pin_ISR_SetInterruptMode(uint16 position, uint16 mode);
uint8   Pin_ISR_ClearInterrupt(void);
/** @} general */

/**
* \addtogroup group_power
* @{
*/
void Pin_ISR_Sleep(void); 
void Pin_ISR_Wakeup(void);
/** @} power */


/***************************************
*           API Constants        
***************************************/
#if defined(Pin_ISR__PC) || (CY_PSOC4_4200L) 
    /* Drive Modes */
    #define Pin_ISR_DRIVE_MODE_BITS        (3)
    #define Pin_ISR_DRIVE_MODE_IND_MASK    (0xFFFFFFFFu >> (32 - Pin_ISR_DRIVE_MODE_BITS))

    /**
    * \addtogroup group_constants
    * @{
    */
        /** \addtogroup driveMode Drive mode constants
         * \brief Constants to be passed as "mode" parameter in the Pin_ISR_SetDriveMode() function.
         *  @{
         */
        #define Pin_ISR_DM_ALG_HIZ         (0x00u) /**< \brief High Impedance Analog   */
        #define Pin_ISR_DM_DIG_HIZ         (0x01u) /**< \brief High Impedance Digital  */
        #define Pin_ISR_DM_RES_UP          (0x02u) /**< \brief Resistive Pull Up       */
        #define Pin_ISR_DM_RES_DWN         (0x03u) /**< \brief Resistive Pull Down     */
        #define Pin_ISR_DM_OD_LO           (0x04u) /**< \brief Open Drain, Drives Low  */
        #define Pin_ISR_DM_OD_HI           (0x05u) /**< \brief Open Drain, Drives High */
        #define Pin_ISR_DM_STRONG          (0x06u) /**< \brief Strong Drive            */
        #define Pin_ISR_DM_RES_UPDWN       (0x07u) /**< \brief Resistive Pull Up/Down  */
        /** @} driveMode */
    /** @} group_constants */
#endif

/* Digital Port Constants */
#define Pin_ISR_MASK               Pin_ISR__MASK
#define Pin_ISR_SHIFT              Pin_ISR__SHIFT
#define Pin_ISR_WIDTH              1u

/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in Pin_ISR_SetInterruptMode() function.
     *  @{
     */
        #define Pin_ISR_INTR_NONE      ((uint16)(0x0000u)) /**< \brief Disabled             */
        #define Pin_ISR_INTR_RISING    ((uint16)(0x5555u)) /**< \brief Rising edge trigger  */
        #define Pin_ISR_INTR_FALLING   ((uint16)(0xaaaau)) /**< \brief Falling edge trigger */
        #define Pin_ISR_INTR_BOTH      ((uint16)(0xffffu)) /**< \brief Both edge trigger    */
    /** @} intrMode */
/** @} group_constants */

/* SIO LPM definition */
#if defined(Pin_ISR__SIO)
    #define Pin_ISR_SIO_LPM_MASK       (0x03u)
#endif

/* USBIO definitions */
#if !defined(Pin_ISR__PC) && (CY_PSOC4_4200L)
    #define Pin_ISR_USBIO_ENABLE               ((uint32)0x80000000u)
    #define Pin_ISR_USBIO_DISABLE              ((uint32)(~Pin_ISR_USBIO_ENABLE))
    #define Pin_ISR_USBIO_SUSPEND_SHIFT        CYFLD_USBDEVv2_USB_SUSPEND__OFFSET
    #define Pin_ISR_USBIO_SUSPEND_DEL_SHIFT    CYFLD_USBDEVv2_USB_SUSPEND_DEL__OFFSET
    #define Pin_ISR_USBIO_ENTER_SLEEP          ((uint32)((1u << Pin_ISR_USBIO_SUSPEND_SHIFT) \
                                                        | (1u << Pin_ISR_USBIO_SUSPEND_DEL_SHIFT)))
    #define Pin_ISR_USBIO_EXIT_SLEEP_PH1       ((uint32)~((uint32)(1u << Pin_ISR_USBIO_SUSPEND_SHIFT)))
    #define Pin_ISR_USBIO_EXIT_SLEEP_PH2       ((uint32)~((uint32)(1u << Pin_ISR_USBIO_SUSPEND_DEL_SHIFT)))
    #define Pin_ISR_USBIO_CR1_OFF              ((uint32)0xfffffffeu)
#endif


/***************************************
*             Registers        
***************************************/
/* Main Port Registers */
#if defined(Pin_ISR__PC)
    /* Port Configuration */
    #define Pin_ISR_PC                 (* (reg32 *) Pin_ISR__PC)
#endif
/* Pin State */
#define Pin_ISR_PS                     (* (reg32 *) Pin_ISR__PS)
/* Data Register */
#define Pin_ISR_DR                     (* (reg32 *) Pin_ISR__DR)
/* Input Buffer Disable Override */
#define Pin_ISR_INP_DIS                (* (reg32 *) Pin_ISR__PC2)

/* Interrupt configuration Registers */
#define Pin_ISR_INTCFG                 (* (reg32 *) Pin_ISR__INTCFG)
#define Pin_ISR_INTSTAT                (* (reg32 *) Pin_ISR__INTSTAT)

/* "Interrupt cause" register for Combined Port Interrupt (AllPortInt) in GSRef component */
#if defined (CYREG_GPIO_INTR_CAUSE)
    #define Pin_ISR_INTR_CAUSE         (* (reg32 *) CYREG_GPIO_INTR_CAUSE)
#endif

/* SIO register */
#if defined(Pin_ISR__SIO)
    #define Pin_ISR_SIO_REG            (* (reg32 *) Pin_ISR__SIO)
#endif /* (Pin_ISR__SIO_CFG) */

/* USBIO registers */
#if !defined(Pin_ISR__PC) && (CY_PSOC4_4200L)
    #define Pin_ISR_USB_POWER_REG       (* (reg32 *) CYREG_USBDEVv2_USB_POWER_CTRL)
    #define Pin_ISR_CR1_REG             (* (reg32 *) CYREG_USBDEVv2_CR1)
    #define Pin_ISR_USBIO_CTRL_REG      (* (reg32 *) CYREG_USBDEVv2_USB_USBIO_CTRL)
#endif    
    
    
/***************************************
* The following code is DEPRECATED and 
* must not be used in new designs.
***************************************/
/**
* \addtogroup group_deprecated
* @{
*/
#define Pin_ISR_DRIVE_MODE_SHIFT       (0x00u)
#define Pin_ISR_DRIVE_MODE_MASK        (0x07u << Pin_ISR_DRIVE_MODE_SHIFT)
/** @} deprecated */

#endif /* End Pins Pin_ISR_H */


/* [] END OF FILE */
