/****************************************************************************
 FileName:      usbcom.cpp
 Dependencies:  See INCLUDES section
 Compiler:      Visual Studio Community 2015 for Windows and g++ for Linux
 Company:       Yepkit, Lda.

 Software License Agreement:

 Copyright (c) 2016 Yepkit Lda

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.

*****************************************************************************
 File Description:

    Change History:
        Rev     Date            Description
        -----   -----------     -----------------------------------------
        0.0.1   2016-02-12      Initial development


 ****************************************************************************
 *  Summary:
 *
 *  Functions that communicate through USB with the YKUR board.
 *
*****************************************************************************/



//-------------------------------------------------------------------------
// INCLUDES
//-------------------------------------------------------------------------
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "platformdefs.h"
#include "stdafx.h"
#include "hidapi.h"
#include "usbcom.h"


using namespace std;


//-------------------------------------------------------------------------
// DEFINES
//-------------------------------------------------------------------------
#define VERSION "0.1.0"
#define VENDOR_ID 0x04D8
#define PRODUCT_ID 0xF1CB





/**************************************************************************
 * listDevices() - List YKUR Devices Serial Number
 *-------------------------------------------------------------------------
 *
 * This function prints the iSerial of the
 * attached YKUR devices.
 *
 **************************************************************************/
int listDevices() {

    	// Enumerate and print the YKUR devices on the system
    	struct hid_device_info *devs, *cur_dev;

    	devs = hid_enumerate(0x0, 0x0);
    	if (devs == NULL) {
        	// No HID devices found
        	printf("\nNo HID USB devices connected to this Host\n");
    	}

    	cur_dev = devs;
    	while (cur_dev) {
        	if ((cur_dev->vendor_id == VENDOR_ID) && (cur_dev->product_id == PRODUCT_ID)) {
            		printf("YKUR device found with Serial Number: %ls", cur_dev->serial_number);
            		printf("\n");
            		printf("  Manufacturer: %ls\n", cur_dev->manufacturer_string);
            		printf("  Product:      %ls\n", cur_dev->product_string);
            		printf("\n");
        	}

        	cur_dev = cur_dev->next;
    	}
    	hid_free_enumeration(devs);
    
	return 0;
}






 /*******************************************************************************
  * command(char cmd, char addr) - Sends the command to YKUR board
  * -----------------------------------------------------------------------------
  * 
  * Description:
  *
  * 	The command in the variable cmd is sent to YKUR board. If more than
  * 	one YKUR board is connected to the host it will send the command
  * 	just to the first one that appears in the enumerated list.
  *
  * 	When more than one YKUR board connected to the same host
  * 	the commandBySerial function should be used instead.
  * 	
  * Inputs:
  *
  * 	cmd	-	Command byte to be sent to YKUR.
  *	addr	-	Board port addressing
  *
  * Outputs:
  *
  * 	The function will always return zero.				
  *
  ********************************************************************************/
 int command(char cmd, char addr)
 {

     	int res;
     	unsigned char buf[65];
     	#define MAX_STR 255
     	wchar_t wstr[MAX_STR];
     	hid_device *handle;
     	int i;


     	// Open YKUR device
     	handle = hid_open(VENDOR_ID, PRODUCT_ID, NULL);

     	// Set the hid_read() function to be blocking.
     	hid_set_nonblocking(handle, 0);

     	// Send an Output report with the desired command
     	buf[0] = 0; 	// First byte is report number
     	buf[1] = cmd;	// Command byte
     	buf[2] = addr;	// Address byte
     	res = hid_write(handle, buf, 65);

     	// Read response
     	res = hid_read(handle, buf, 65);
     	if (res < 0) {
         	printf("Unable to read YKUR command confirmation\n");
     	}

     	// Print out the returned buffer.
	
     	for (i = 0; i < res; i++)
         	printf("buf[%d]: %d\n", i, buf[i]);
	

    	return 0;
 }
 



 /*******************************************************************************
  * commandBySerial(char *iSerial, char cmd, char addr) - Sends the command to YKUR board
  * -----------------------------------------------------------------------------
  * 
  * Description:
  *
  * 	The command in the variable cmd is sent to YKUR board. If more than
  * 	one YKUR board is connected to the host it will send the command
  * 	just to the first one that appears in the enumerated list.
  *
  * 	If there are more than one YKUR board connected to the same host
  * 	the commandBySerial function should be used instead.
  * 	
  * Inputs:
  *	
  *	iSerial	-	Serial number of the YKUR board to issue the command.
  * 	cmd	-	Command byte to be sent to YKUR.
  *	addr	-	Board port addressing
  *
  * Outputs:
  *
  * 	The function will always return zero.				
  *
  ********************************************************************************/
 int commandBySerial(char *iSerial, char cmd, char addr)
 {

	int res;
     	unsigned char buf[65];
     	#define MAX_STR 255
     	wchar_t wstr[MAX_STR];
     	hid_device *handle;
     	int i;
	
     
	// Convert to a wchar_t*
	size_t origsize = strlen(iSerial) + 1;
	const size_t newsize = 100;
	size_t convertedChars = 0;
	wchar_t serial[newsize];

#ifndef _LINUX_
	mbstowcs_s(&convertedChars, serial, origsize, iSerial, _TRUNCATE);
#else
	mbstowcs(serial, iSerial, newsize);
#endif

     	// Open the YKUR device 
 	handle = hid_open(VENDOR_ID, PRODUCT_ID, serial);

     	// Set the hid_read() function to be blocking (wait for response from YKUR).
    	hid_set_nonblocking(handle, 0);

     	// Send an Output report with the desired command
     	buf[0] = 0; 	// First byte is report number
     	buf[1] = cmd;	// Command byte
     	buf[2] = addr;	// Address byte
     	res = hid_write(handle, buf, 65);

     	// Read response
     	res = hid_read(handle, buf, 65);
     	if (res < 0) {
         	printf("Unable to read YKUR command response\n");
     	}

 
    	return 0;
 }
 









