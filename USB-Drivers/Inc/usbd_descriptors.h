/*
 * usbd_descriptors.h
 *
 *  Created on: 29-Mar-2022
 *      Author: 007ma
 */
#include "usb_standards.h"
#include "HID/Usb_hid_standards.h"

#ifndef USBD_DESCRIPTORS_H_
#define USBD_DESCRIPTORS_H_

const UsbDeviceDescriptor device_descriptor = {
		.bLength 		 = sizeof(UsbDeviceDescriptor),
		.bDescriptorType = USB_DESCRIPTOR_TYPE_DEVICE,
		.bcdUSB			 = 0x0200, //Usb type ours is 2.0
		.bDeviceClass	 = USB_CLASS_PER_INTERFACE,
		.bDeviceSubClass = USB_SUBCLASS_NONE,
		.bDeviceProtocol = USB_PROTOCOL_NONE,
		.bMaxPacketSize0 = 8,
		.idVendor		 = 0x666,
		.idProduct		 = 0x13AA,
		.bcdDevice		 = 0x0100, //device version
		.iManufacturer 	 = 0,//strings if present
		.iProduct		 = 0,
		.iSerialNumber	 = 0,
		.bNumConfigurations	= 1 //number of configuration in the device
};

//const uint8_t hid_report_descriptor[]={};
const uint8_t hid_report_descriptor[]= {
	HID_USAGE_PAGE(HID_PAGE_DESKTOP),
	HID_USAGE(HID_DESKTOP_MOUSE),
	HID_COLLECTION(HID_APPLICATION_COLLECTION),
	 HID_USAGE(HID_DESKTOP_POINTER),
	 HID_COLLECTION(HID_PHYSICAL_COLLECTION),
		HID_USAGE(HID_DESKTOP_X),
		HID_USAGE(HID_DESKTOP_Y),
		HID_USAGE_MINIMUM(-127),
		HID_USAGE_MAXIMUM(127),
		HID_REPORT_SIZE(8),
		HID_REPORT_COUNT(2),
		HID_INPUT(HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_RELATIVE),

		HID_USAGE_PAGE(HID_PAGE_BUTTON),
		HID_USAGE_MINIMUM(1),
		HID_USAGE_MAXIMUM(3),
		HID_LOGICAL_MINIMUM(0),
		HID_LOGICAL_MAXIMUM(1),
		HID_REPORT_SIZE(1),
		HID_REPORT_COUNT(3),
		HID_INPUT(HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),
		HID_REPORT_SIZE(1),
		HID_REPORT_COUNT(5),
		HID_INPUT(HID_IOF_CONSTANT),
	 HID_END_COLLECTION,
	HID_END_COLLECTION
};

typedef struct
{
	UsbConfigurationDescriptor usb_configuration_descriptor;
	UsbInterfaceDescriptor usb_interface_descriptor;
	UsbHidDescriptor usb_mouse_hid_descriptor;
	UsbEndpointDescriptor usb_mouse_endpoint_descriptor;
}UsbConfigurationDescriptorCombination;

const UsbConfigurationDescriptorCombination configuration_descriptor_combinaton={
		.usb_configuration_descriptor = {
				.bLength = sizeof(UsbConfigurationDescriptor),
				.bDescriptortype = USB_DESCRIPTOR_TYPE_CONFIGURATION,
				.wTotalLength = sizeof(UsbConfigurationDescriptorCombination),
				.bNumInterfaces = 1,
				.bConfiguration = 1,
				.iConfiguration = 0,
				.bmAttributes = 0x80 | 0x40,
				.bMaxpower = 25
		},
		.usb_interface_descriptor = {
				.bLength =  sizeof(UsbInterfaceDescriptor),
				.bDescriptorType = USB_DESCRIPTOR_TYPE_INTERFACE,
				.bInterfaceNumber = 1,
				.bAlternateSetting = 0,
				.bNumEndpoints    = 1,
				.bInterfaceClass  = USB_CLASS_HID,
				.bInterfaceSubClass = USB_PROTOCOL_NONE,
				.bInterfaceProtocol = USB_PROTOCOL_NONE,
				.iInterface = 0
		},
		.usb_mouse_endpoint_descriptor = {
				.bLength = sizeof(UsbEndpointDescriptor),
				.bDescriptorType = USB_DESCRIPTOR_TYPE_ENDPOINT,
				.bEndpointAddress = 0x83, // 8 means IN endpont and 3 means thrid end point
				.bmAttributes = USB_ENDPOINT_TYPE_INTERRUPT,
				.wMaxPacketSize = 64,
				.bInterval = 50
		},
		.usb_mouse_hid_descriptor = {
				.bLength = sizeof(UsbHidDescriptor),
				.bDescriptorType = USB_DESCRIPTOR_TYPE_HID,
				.bcdHID = 0x0100,
				.bCountryCode = USB_HID_COUNTRY_NONE,
				.bNumDescriptors = 1,
				.bDescriptorType0 = USB_DESCRIPTOR_TYPE_HID_REPORT,
				.wDescriptorLength0 = sizeof(hid_report_descriptor)
		}
};

typedef struct {
	int8_t x;
	int8_t y;
	uint8_t buttons;
}__attribute__((__packed__))HidReport;



#endif /* USBD_DESCRIPTORS_H_ */
