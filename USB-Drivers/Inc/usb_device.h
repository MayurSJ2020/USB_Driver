/*
 * usb_device.h
 *
 *  Created on: 21-Mar-2022
 *      Author: 007ma
 */

#ifndef USB_DEVICE_H_
#define USB_DEVICE_H_

#include "usb_standards.h"

typedef struct
{
	// The current Usb device state
	UsbDeviceState device_state;

	// Control transfer stage (for endpoint 0)
	UsbControlTranferStage control_transfer_stage;

	//The selected Usb config
	uint8_t configuration_value;

	//Usb device out in buffer pointers
	void const  *ptr_out_buffer;
	uint32_t out_data_size;

	void const  *ptr_in_buffer;
	uint32_t in_data_size;
}UsbDevice;

#endif /* USB_DEVICE_H_ */
