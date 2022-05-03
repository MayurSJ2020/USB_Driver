/*
 * usbd_driver.c
 *
 *  Created on: 27-Feb-2022
 *      Author: 007ma
 */
#include "usbd_driver.h"
#include "string.h"
#include "Helpers/logger.h"
/*
 * AFR for usb d+ and d- are PB15 and PB14 in alterante function 12
 */
static void Initialize_usb_pins()
{
	// Enables the clock for GPIOB.
	SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOBEN);

	// Configures USB pins (in GPIOB) to work in alternate function mode.
	MODIFY_REG(GPIOB->MODER,
		GPIO_MODER_MODER14 | GPIO_MODER_MODER15,
		_VAL2FLD(GPIO_MODER_MODER14, 2) | _VAL2FLD(GPIO_MODER_MODER15, 2)
	);
	// Sets alternate function 12 for: PB14 (-), and PB15 (+).
	MODIFY_REG(GPIOB->AFR[1],
		GPIO_AFRH_AFSEL14 | GPIO_AFRH_AFSEL15,
		_VAL2FLD(GPIO_AFRH_AFSEL14, 0xC) | _VAL2FLD(GPIO_AFRH_AFSEL15, 0xC)
	);

}

static void Initialize_core()
{
	// Enables the clock for USB core.
	SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_OTGHSEN);

	// Configures the USB core to run in device mode, and to use the embedded full-speed PHY.
	MODIFY_REG(USB_OTG_HS->GUSBCFG,
		USB_OTG_GUSBCFG_FDMOD | USB_OTG_GUSBCFG_PHYSEL | USB_OTG_GUSBCFG_TRDT,
		USB_OTG_GUSBCFG_FDMOD | USB_OTG_GUSBCFG_PHYSEL | _VAL2FLD(USB_OTG_GUSBCFG_TRDT, 0x09)
	);

	// Configures the device to run in full speed mode.
	MODIFY_REG(USB_OTG_HS_DEVICE->DCFG,
		USB_OTG_DCFG_DSPD,
		_VAL2FLD(USB_OTG_DCFG_DSPD, 0x03)
	);

	// Enables VBUS sensing device.
	SET_BIT(USB_OTG_HS->GCCFG, USB_OTG_GCCFG_VBUSBSEN);


	// Unmasks the main USB core interrupts.
	SET_BIT(USB_OTG_HS->GINTMSK,
		USB_OTG_GINTMSK_USBRST | USB_OTG_GINTMSK_ENUMDNEM | USB_OTG_GINTMSK_SOFM |
		USB_OTG_GINTMSK_USBSUSPM | USB_OTG_GINTMSK_WUIM | USB_OTG_GINTMSK_IEPINT |
		USB_OTG_GINTSTS_OEPINT | USB_OTG_GINTMSK_RXFLVLM
	);

	// Clears all pending core interrupts.
	WRITE_REG(USB_OTG_HS->GINTSTS, 0xFFFFFFFF);

	// Unmasks USB global interrupt.
	SET_BIT(USB_OTG_HS->GAHBCFG, USB_OTG_GAHBCFG_GINT);

	// Unmasks transfer completed interrupts for all endpoints.
	SET_BIT(USB_OTG_HS_DEVICE->DOEPMSK, USB_OTG_DOEPMSK_XFRCM);
	SET_BIT(USB_OTG_HS_DEVICE->DIEPMSK, USB_OTG_DIEPMSK_XFRCM);
}
static void set_device_address(uint8_t address)
{
	MODIFY_REG(USB_OTG_HS_DEVICE->DCFG,
			USB_OTG_DCFG_DAD,
			_VAL2FLD(USB_OTG_DCFG_DAD,address));
}
//This is to configure the start address of the FIFOs start addresses
static void  refresh_fifo_start_addresses()
{
	//the start address of rx fifo that is OUT endpoint will be always fixed
	// calculating the size the OUT endpoint so we can get it last address
	uint16_t start_address = _FLD2VAL(USB_OTG_GRXFSIZ_RXFD, USB_OTG_HS->GRXFSIZ) * 4; //*4 because we store length of fifo in 32bit ie 1 length = 32bot to convert it to bit we should multiply by 4

	//Assigning the address calculated to the start address of IN0 endpoint
	MODIFY_REG(USB_OTG_HS->DIEPTXF0_HNPTXFSIZ,
			USB_OTG_TX0FSA,
			_VAL2FLD(USB_OTG_TX0FSA,start_address));

	//calculating the size of IN0 so we can add and get the consumed stack
	start_address += _FLD2VAL(USB_OTG_TX0FD, USB_OTG_HS->DIEPTXF0_HNPTXFSIZ) * 4;

	for(uint8_t txfifo_no = 0; txfifo_no < ENDPOINT_COUNT - 1; txfifo_no++)
	{
		MODIFY_REG(USB_OTG_HS->DIEPTXF[txfifo_no],
				USB_OTG_NPTXFSA,
				_VAL2FLD(USB_OTG_NPTXFSA,start_address));

		start_address += _FLD2VAL(USB_OTG_NPTXFD, USB_OTG_HS->DIEPTXF[txfifo_no]) * 4;
	}
}


/*
 * Configuring the OUT endpoints of device as receive is in the perspective of host
 */
static void Configure_rxfifosize(uint16_t size)
{
	//Size is measure in terms of 32-bit as one length
	size = 10 + (2 * ((size / 4) + 1));

	//Configuring the dept of the fifo
	MODIFY_REG(USB_OTG_HS->GRXFSIZ,
			USB_OTG_GRXFSIZ_RXFD,
			_VAL2FLD(USB_OTG_GRXFSIZ_RXFD, size));

	refresh_fifo_start_addresses();
}

static void Configure_txfifosize(uint8_t endpoint_number,uint16_t size)
{
	//size in terms of 32 bit 1 = 1 32 bit
	size = (size + 3) / 4;

	if(endpoint_number == 0)
	{
		//This is to configure the in endpoint 0 ie for the host to send control packets
		MODIFY_REG(USB_OTG_HS->DIEPTXF0_HNPTXFSIZ,
				USB_OTG_TX0FD,
				_VAL2FLD(USB_OTG_TX0FD,size));
	}
	else
	{
		//This is for the in endpoint for the device to send the data
		MODIFY_REG(USB_OTG_HS->DIEPTXF[endpoint_number - 1],
				USB_OTG_NPTXFD,
				_VAL2FLD(USB_OTG_NPTXFD,size));
	}

	refresh_fifo_start_addresses();
}

/*
 * You can connect and disconnect device from bus using usb registers
 */
static void Connect(void)
{
	//Turning on transreceivers
	SET_BIT(USB_OTG_HS->GCCFG,USB_OTG_GCCFG_PWRDWN);

	//Connecting usb device to the bus clearing software disconnect bit
	CLEAR_BIT(USB_OTG_HS_DEVICE->DCTL,USB_OTG_DCTL_SDIS);
}

static void Disconnect(void)
{
	//Disconnecting usb device from the bus
	SET_BIT(USB_OTG_HS_DEVICE->DCTL,USB_OTG_DCTL_SDIS);

	//Turning off transreceivers
	CLEAR_BIT(USB_OTG_HS->GCCFG,USB_OTG_GCCFG_PWRDWN);
}
/*
 * This function is used to read data from USB out fifo which is sent by host
 */
static void read_packet(void const *buffer, uint16_t size)
{
	//This is only one Rx fifo so we dont need address for every transaction
	uint32_t volatile *fifo = FIFO(0);

	for(;size>=4; size -=4)
	{
		//Pops one 32 bit of data until there is less than one word less
		uint32_t data = *fifo;
		//store the data in the buffer
		*((uint32_t*)buffer) = data;
		buffer+=4;
	}

	if(size > 0)
	{
		//Pops the remaining bytes which is less than a word which is 4bytes
		uint32_t data= *fifo;

		for(;size>0; size --, data >>= 8) // only reading byte by byte after the length is less than a word
		{
			*((uint8_t *)buffer) = data & 0xFF;
			buffer++;
		}
	}
}
/*
 * This function is used to write packet to the fifo which we are intending to send
 */
static void write_packet (uint8_t endpoint_number, void const *buffer, uint16_t size )
{
	uint32_t volatile *fifo = FIFO(endpoint_number);
	USB_OTG_INEndpointTypeDef *in_endpoint = IN_ENDPOINT(endpoint_number);

	//configuring that we are sending 1 packet of "size" bytes
	MODIFY_REG(in_endpoint->DIEPTSIZ,
			USB_OTG_DIEPTSIZ_PKTCNT | USB_OTG_DIEPTSIZ_XFRSIZ,
			_VAL2FLD(USB_OTG_DIEPTSIZ_PKTCNT,1) | _VAL2FLD(USB_OTG_DIEPTSIZ_XFRSIZ,size) );

	//enables the transmission after clearing both stall and nak of the endpoint
	MODIFY_REG(in_endpoint->DIEPCTL,
			USB_OTG_DIEPCTL_STALL,
			USB_OTG_DIEPCTL_CNAK | USB_OTG_DIEPCTL_EPENA);

	/*
	 * This is to avoid overflow ex we need to send 22bytes for one transaction we send 4bytes ie 1 word
	 * So 22/4 = 5.5 so to send that .5 packet we add +3 so it will become 6 packets to send
	 */
	size =  (size + 3) / 4 ;

	for(;size>0; size -- ,buffer+=4)
	{
		*fifo = *((uint32_t*)buffer) ;;

	}

}

/*
 * Clearing the rx buffer removing the content in the buffer
 */
static void flush_rxfifo(void)
{
	SET_BIT(USB_OTG_HS->GRSTCTL, USB_OTG_GRSTCTL_RXFFLSH);
}

/*
 * Clearing the tx buffer removing the content in the buffer
 */
static void flush_txfifo(uint8_t endpoint_number)
{
	MODIFY_REG(USB_OTG_HS->GRSTCTL,
			USB_OTG_GRSTCTL_TXFNUM,
			_VAL2FLD(USB_OTG_GRSTCTL_TXFNUM, endpoint_number) | USB_OTG_GRSTCTL_TXFFLSH);
}

static void configure_endpoint0(uint8_t endpoint_size)
{
	SET_BIT(USB_OTG_HS_DEVICE->DAINTMSK, 1<<0 | 1<<16); //Unmasking endpoint 0 IN and out Endpoint.

	//Configures the max packet size, activates the endpoint, and NAK the endpoint(cannot send data only)
	MODIFY_REG(IN_ENDPOINT(0)->DIEPCTL,
			USB_OTG_DIEPCTL_MPSIZ,
			USB_OTG_DIEPCTL_USBAEP | _VAL2FLD(USB_OTG_DIEPCTL_MPSIZ , endpoint_size) | USB_OTG_DIEPCTL_SNAK);

	//Clears NAK and enables endpoint for transmission
	SET_BIT(OUT_ENDPOINT(0)->DOEPCTL, USB_OTG_DOEPCTL_EPENA | USB_OTG_DOEPCTL_CNAK);

	//64 bytes is max packet size for FS usb
	Configure_rxfifosize(64);
	Configure_txfifosize(0,endpoint_size);

}

static void Configure_IN_Endpoint(uint8_t endpoint_number, UsbEndpointType endpoint_type, uint16_t endpoint_size)
{
	//unmasking in endpoint interrupt
	SET_BIT(USB_OTG_HS_DEVICE->DAINTMSK, 1<<endpoint_number);


	 //Activating the endpoint | setting size of the endpoint | setting snak bit to nak the host on sending data from device | setting type of endpoint | assingning tx fifo with data 0

	MODIFY_REG(IN_ENDPOINT(endpoint_number)->DIEPCTL,
			USB_OTG_DIEPCTL_MPSIZ | USB_OTG_DIEPCTL_EPTYP | USB_OTG_DIEPCTL_TXFNUM,
			USB_OTG_DIEPCTL_USBAEP | _VAL2FLD(USB_OTG_DIEPCTL_MPSIZ,endpoint_size) | USB_OTG_DIEPCTL_SNAK |
			_VAL2FLD(USB_OTG_DIEPCTL_EPTYP, endpoint_type) | USB_OTG_DIEPCTL_SD0PID_SEVNFRM |
			_VAL2FLD(USB_OTG_DIEPCTL_TXFNUM, endpoint_number)); // assigning number to the endpoint

	Configure_txfifosize(endpoint_number, endpoint_size);

}

static void Deconfig_endpoints(uint8_t endpoint_number)
{
	USB_OTG_INEndpointTypeDef  * In_endpoint = IN_ENDPOINT(endpoint_number);
	USB_OTG_OUTEndpointTypeDef * Out_endpoint = OUT_ENDPOINT(endpoint_number);

	//Mask all interuppts of the targettd in and out interrupt
	CLEAR_BIT(USB_OTG_HS_DEVICE->DAINTMSK, 1<<endpoint_number | 1<<16<<endpoint_number);

	//clears all the interrupt of the endpoints
	SET_BIT(In_endpoint->DIEPINT, 0x29FF);
	SET_BIT(Out_endpoint->DOEPINT, 0X715F);

	//Disabling endpoint if possible
	if(In_endpoint->DIEPCTL & USB_OTG_DIEPCTL_EPENA)
	{
		SET_BIT(In_endpoint->DIEPCTL, USB_OTG_DIEPCTL_EPDIS);
	}

	//Deactivating endpoint
	CLEAR_BIT(In_endpoint->DIEPCTL, USB_OTG_DIEPCTL_USBAEP);
	//We cannot deconfig out endpoint 0 ie control endpoint it should be active always
	if(endpoint_number != 0)
	{
		if(Out_endpoint->DOEPCTL & USB_OTG_DOEPCTL_EPENA)
		{
			SET_BIT(Out_endpoint->DOEPCTL, USB_OTG_DOEPCTL_EPDIS);
		}

		CLEAR_BIT(Out_endpoint->DOEPCTL, USB_OTG_DOEPCTL_USBAEP);
	}
	//flushing out all the datas in the fifo
	flush_rxfifo();
	flush_txfifo(endpoint_number);
}


static void usbrst_handler()
{
	log_info("USB reset was detected");
	for(uint8_t i=0; i<= ENDPOINT_COUNT; i++)
	{
		Deconfig_endpoints(i);
	}
	usb_events.on_usb_reset_received();
}

static void enumdne_handler(void)
{
	log_info("USB device speed enumaration done");
	configure_endpoint0(8);
}

/*
 * Helper function for rx not empty handler
 */
static void rxflvl_handler()
{
	//popping status info word from the rxfifo
	uint32_t receive_status = USB_OTG_HS_GLOBAL->GRXSTSP;

	//determining in which endpoint data is received from above register
	uint8_t endpoint_number = _FLD2VAL(USB_OTG_GRXSTSP_EPNUM, receive_status);
	//Number of bytes of data received
	uint16_t bcnt = _FLD2VAL(USB_OTG_GRXSTSP_BCNT, receive_status);
	//Status of received packet
	uint16_t pktsts = _FLD2VAL(USB_OTG_GRXSTSP_PKTSTS, receive_status);

	switch(pktsts)
	{
	case 0x06: //Setup packet (includes data)
		usb_events.on_setup_data_received(endpoint_number, bcnt);
		break;
	case 0x02: //Out packet (includes data)
		break;
	case 0x04: //Setup stage is complete
		//Endpoint will be disabled by the core after one transaction so we have to reenable it
		SET_BIT(OUT_ENDPOINT(endpoint_number)->DOEPCTL,
				USB_OTG_DOEPCTL_CNAK | USB_OTG_DOEPCTL_EPENA);
		break;
	case 0x03: //Out transfer has completed
		//Endpoint will be disabled by the core after one transaction so we have to reenable it
		SET_BIT(OUT_ENDPOINT(endpoint_number)->DOEPCTL,
				USB_OTG_DOEPCTL_CNAK | USB_OTG_DOEPCTL_EPENA);
		break;
	}
}

static void iepint_handler()
{
	uint8_t endpoint_number = ffs(USB_OTG_HS_DEVICE->DAINT) - 1;

	if(IN_ENDPOINT(endpoint_number)->DIEPINT & USB_OTG_DIEPINT_XFRC)
	{
		//swv_writeline("Transfer completed on IN endpiont %d.",endpoint_number);
		usb_events.on_in_tranfer_completed(endpoint_number);

		SET_BIT(IN_ENDPOINT(endpoint_number)->DIEPINT,USB_OTG_DIEPINT_XFRC);
	}
}

static void oepint_handler()
{
	uint8_t endpoint_number = ffs(USB_OTG_HS_DEVICE->DAINT >> 16) - 1;

	if(OUT_ENDPOINT(endpoint_number)->DOEPINT & USB_OTG_DOEPINT_XFRC)
	{
		//swv_writeline("Transfer completed on IN endpiont %d.",endpoint_number);
		usb_events.on_out_tranfer_completed(endpoint_number);

		SET_BIT(OUT_ENDPOINT(endpoint_number)->DOEPINT,USB_OTG_DOEPINT_XFRC);
	}
}
/*
 * Interrupt handling of USB
 */
static void Ginsts_handler(void)
{
	volatile uint32_t  gintsts = USB_OTG_HS_GLOBAL->GINTSTS;

	if(gintsts & USB_OTG_GINTSTS_USBRST)
	{
		usbrst_handler();
		SET_BIT(USB_OTG_HS_GLOBAL->GINTSTS,USB_OTG_GINTSTS_USBRST); //clearing interrupt flag
	}
	else if(gintsts & USB_OTG_GINTSTS_ENUMDNE)
	{
		enumdne_handler();
		SET_BIT(USB_OTG_HS_GLOBAL->GINTSTS,USB_OTG_GINTSTS_ENUMDNE);
	}
	else if(gintsts & USB_OTG_GINTSTS_RXFLVL)
	{
		rxflvl_handler();
		SET_BIT(USB_OTG_HS_GLOBAL->GINTSTS,USB_OTG_GINTSTS_RXFLVL);
	}
	else if(gintsts & USB_OTG_GINTSTS_IEPINT)
	{
		iepint_handler();
		SET_BIT(USB_OTG_HS_GLOBAL->GINTSTS,USB_OTG_GINTSTS_IEPINT);
	}
	else if(gintsts & USB_OTG_GINTSTS_OEPINT)
	{
		oepint_handler();
		SET_BIT(USB_OTG_HS_GLOBAL->GINTSTS,USB_OTG_GINTSTS_OEPINT);
	}
	usb_events.on_usb_polled();
}

const UsbDriver usb_driver = {
		.initialize_core = &Initialize_core,
		.initialize_gpio_pins = &Initialize_usb_pins,
		.connect = &Connect,
		.disconnect = &Disconnect,
		.flush_rxfifo = &flush_rxfifo,
		.flush_txfifo = &flush_txfifo,
		.configure_in_endpoint = &Configure_IN_Endpoint,
		.read_packet = &read_packet,
		.write_packet = &write_packet,
		.poll = &Ginsts_handler,
		.set_device_address = &set_device_address
};



