/*
 * msc_desc.c
 *
 *  Created on: 2014.07.04
 *      Author: Kestutis Bivainis
 */

#include "app_usbd_cfg.h"

/**
 * USB Standard Device Descriptor
 */
ALIGNED(4) const uint8_t USB_DeviceDescriptor[] = {
  USB_DEVICE_DESC_SIZE,       /* bLength */
  USB_DEVICE_DESCRIPTOR_TYPE, /* bDescriptorType */
  WBVAL(0x0200),              /* bcdUSB */
  0x00,                       /* bDeviceClass */
  0x00,                       /* bDeviceSubClass */
  0x00,                       /* bDeviceProtocol */
  USB_MAX_PACKET0,            /* bMaxPacketSize0 */
  WBVAL(0x0D28),              /* idVendor */
  WBVAL(0x0206),              /* idProduct */
  WBVAL(0x0100),              /* bcdDevice */
  0x01,                       /* iManufacturer Index to string descriptor containing manufacturer. */
  0x02,                       /* iProduct Index to string descriptor containing product. */
  0x03,                       /* iSerialNumber Index to string descriptor containing serial number. */
  0x01                        /* bNumConfigurations */
};

/**
 * USB FSConfiguration Descriptor
 * All Descriptors (Configuration, Interface, Endpoint, Class, Vendor)
 */
ALIGNED(4) uint8_t USB_FsConfigDescriptor[] = {
/* Configuration 1 */
  USB_CONFIGURATION_DESC_SIZE,       /* bLength */
  USB_CONFIGURATION_DESCRIPTOR_TYPE, /* bDescriptorType */
  WBVAL(
    USB_CONFIGURATION_DESC_SIZE + 
    USB_INTERFACE_DESC_SIZE + 
    USB_ENDPOINT_DESC_SIZE + 
    USB_ENDPOINT_DESC_SIZE
  ),                                 /* wTotalLength */
  0x01,                              /* bNumInterfaces: Number of interfaces supported by this configuration. */
  0x01,                              /* bConfigurationValue: Value to use as an argument to Set Configuration to select this configuration. */
  0x04,                              /* iConfiguration: Index of string descriptor describing this configuration. */
  USB_CONFIG_SELF_POWERED,           /* bmAttributes  */
  USB_CONFIG_POWER_MA(100),          /* bMaxPower: 100mA */
/* Interface 0, Alternate Setting 0, MSC Class */
  USB_INTERFACE_DESC_SIZE,           /* bLength */
  USB_INTERFACE_DESCRIPTOR_TYPE,     /* bDescriptorType */
  0,                                 /* bInterfaceNumber: Number of Interface */
  0x00,                              /* bAlternateSetting: Alternate setting */
  0x02,                              /* bNumEndpoints: Two endpoints used */
  USB_DEVICE_CLASS_STORAGE,          /* bInterfaceClass: Storage device class */
  MSC_SUBCLASS_SCSI,                 /* bInterfaceSubClass: MSC Subclass */
  MSC_PROTOCOL_BULK_ONLY,            /* bInterfaceProtocol: MSC Protocol */
  0x05,                              /* iInterface: Index to string descriptor containing interface description. */
/* Bulk In Endpoint */
  USB_ENDPOINT_DESC_SIZE,            /* bLength */
  USB_ENDPOINT_DESCRIPTOR_TYPE,      /* bDescriptorType */
  MSC_EP_IN,                         /* bEndpointAddress */
  USB_ENDPOINT_TYPE_BULK,            /* bmAttributes */
  WBVAL(USB_FS_MAX_BULK_PACKET),     /* wMaxPacketSize */
  0x00,                              /* bInterval: ignore for Bulk transfer */  
/* Bulk Out Endpoint */
  USB_ENDPOINT_DESC_SIZE,            /* bLength */
  USB_ENDPOINT_DESCRIPTOR_TYPE,      /* bDescriptorType */
  MSC_EP_OUT,                        /* bEndpointAddress */
  USB_ENDPOINT_TYPE_BULK,            /* bmAttributes */
  WBVAL(USB_FS_MAX_BULK_PACKET),     /* wMaxPacketSize */
  0x00,                              /* bInterval: ignore for Bulk transfer */
/* Terminator */
  0                                  /* bLength */
};

/**
 * USB String Descriptor (optional)
 */
ALIGNED(4) const uint8_t USB_StringDescriptor[] = {
/* Index 0x00: LANGID Codes */
  0x04,                       /* bLength */
  USB_STRING_DESCRIPTOR_TYPE, /* bDescriptorType */
  WBVAL(0x0409),              /* US English */    /* wLANGID */
/* Index 0x01: Manufacturer */
  (3 * 2 + 2),                /* bLength (3 Char + Type + lenght) */
  USB_STRING_DESCRIPTOR_TYPE, /* bDescriptorType */
  'N', 0,
  'X', 0,
  'P', 0,
/* Index 0x02: Product */
  (18 * 2 + 2),               /* bLength */
  USB_STRING_DESCRIPTOR_TYPE, /* bDescriptorType */
  'L', 0,
  'P', 0,
  'C', 0,
  '1', 0,
  '5', 0,
  '4', 0,
  '9', 0,
  ' ', 0,
  'L', 0,
  'P', 0,
  'C', 0,
  'X', 0,
  'p', 0,
  'r', 0,
  'e', 0,
  's', 0,
  's', 0,
  'o', 0,
/* Index 0x03: Serial Number */
  (10 * 2 + 2),               /* bLength (8 Char + Type + lenght) */
  USB_STRING_DESCRIPTOR_TYPE, /* bDescriptorType */
  '0', 0,
  '1', 0,
  '2', 0,
  '3', 0,
  '4', 0,
  '5', 0,
  '6', 0,
  '7', 0,
  '8', 0,
  '9', 0,
/* Index 0x04: Interface 1, Alternate Setting 0 */
  (3 * 2 + 2),                /* bLength (4 Char + Type + lenght) */
  USB_STRING_DESCRIPTOR_TYPE, /* bDescriptorType */
  'M', 0,
  'S', 0,
  'D', 0,
/* Index 0x05: Interface 1, Alternate Setting 0 */
  (3 * 2 + 2),                /* bLength (4 Char + Type + lenght) */
  USB_STRING_DESCRIPTOR_TYPE, /* bDescriptorType */
  'U', 0,
  'S', 0,
  'B', 0,
};

uint8_t InquiryStr[28] = {
    'E','E','P','R','O','M',' ',' ','L','P','C','X','p','r','e','s','s','o',' ','b','o','a','r','d','1','.','0','0'
};
