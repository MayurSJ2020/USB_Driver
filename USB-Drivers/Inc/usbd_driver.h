/*
 * usbd_driver.h
 *
 *  Created on: 27-Feb-2022
 *      Author: 007ma
 */

#ifndef USBD_DRIVER_H_
#define USBD_DRIVER_H_

#include "stm32f4xx.h"
#include "usb_standards.h"




#define USB_OTG_HS_GLOBAL ((USB_OTG_GlobalTypeDef*) (USB_OTG_HS_PERIPH_BASE + USB_OTG_GLOBAL_BASE))
#define USB_OTG_HS_DEVICE ((USB_OTG_DeviceTypeDef*)(USB_OTG_HS_PERIPH_BASE + USB_OTG_DEVICE_BASE))
#define USB_OTG_HS_PCGCCTL ((uint32_t*)(USB_OTG_HS_PERIPH_BASE + USB_OTG_PCGCCTL_BASE))

/*
 * This function returns the address for IN_ENDPOINT register
 * param : the will receive endpoint number as a parameter
 */
inline static USB_OTG_INEndpointTypeDef * IN_ENDPOINT(uint8_t endpoint_number)
{
	return (USB_OTG_INEndpointTypeDef *)(USB_OTG_HS_PERIPH_BASE + USB_OTG_IN_ENDPOINT_BASE + (endpoint_number*0x20));
}

/*
 * This is for outendpoints
 */
inline static USB_OTG_OUTEndpointTypeDef * OUT_ENDPOINT(uint8_t endpoint_number)
{
	return (USB_OTG_OUTEndpointTypeDef *)(USB_OTG_HS_PERIPH_BASE + USB_OTG_OUT_ENDPOINT_BASE + (endpoint_number*0x20));
}

inline static __IO uint32_t *FIFO(uint8_t endpoint_number)
{
	return (__IO uint32_t*)(USB_OTG_HS_PERIPH_BASE + USB_OTG_FIFO_BASE + (endpoint_number * 0x1000));
}

#define ENDPOINT_COUNT 6

typedef struct
{
	void (*initialize_core)();
	void (*initialize_gpio_pins)();
	void (*connect)();
	void (*disconnect)();
	void (*flush_rxfifo)();
	void (*flush_txfifo)(uint8_t endpoint_number);
	void (*configure_in_endpoint)(uint8_t endpoint_number,enum UsbEndpointType endpoint_type, uint16_t endpoint_size);
	void (*read_packet)(void const *buffer, uint16_t size);
	void (*write_packet)(uint8_t endpoint_number, void const *buffer, uint16_t size);
	void (*poll)();
	void (*set_device_address)(uint8_t address);
}UsbDriver;

extern const UsbDriver usb_driver;
extern const UsbEvents usb_events;

#endif /* USBD_DRIVER_H_ */
