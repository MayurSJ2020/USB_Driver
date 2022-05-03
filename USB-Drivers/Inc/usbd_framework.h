/*
 * usbd_framework.h
 *
 *  Created on: 27-Feb-2022
 *      Author: 007ma
 */

#ifndef USBD_FRAMEWORK_H_
#define USBD_FRAMEWORK_H_

#include "usbd_driver.h"
#include "usb_device.h"

void usbd_intialize(UsbDevice *usb_device);
void usbd_poll(void);



#endif /* USBD_FRAMEWORK_H_ */
