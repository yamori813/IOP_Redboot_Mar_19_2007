// lcd_menu function from main diag menu....///

#include <pkgconf/hal.h>
#include <pkgconf/system.h>
#include CYGBLD_HAL_PLATFORM_H

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_arch.h>           // Register state info
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_intr.h>           // Interrupt names
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_ioc80314.h>          // Hardware definitions
#include <cyg/hal/iq80315.h>            // Platform specifics

#include <cyg/infra/diag.h>             // diag_printf
#include <redboot.h>
#include <cyg/io/pci.h>
#include "test_menu.h"

extern CYG_ADDRWORD decIn(void);


#define LINE1 0x80
#define LINE2 0xC0
#define LINE3 0x94
#define LINE4 0xD4
#define LCD_IR 0x4e830000
#define LCD_DR 0x4e838000
#define CLEAR_SCREEN 1

void init_lcd (void)  // Initializes the LCD.
{
	char * dataport;
	char * instrport;
	dataport = (char*)LCD_DR;
	instrport = (char*)LCD_IR;

	* instrport = 0x38;  //38 or 3C -- Set to 8-bit mode, 1/16 duty cycle, -- [38 for 5x8], [3C for 5x11] font
	CYGACC_CALL_IF_DELAY_US(4100);        
	* instrport = 0x38;
	CYGACC_CALL_IF_DELAY_US(4100);
	* instrport = 0x38;
	CYGACC_CALL_IF_DELAY_US(4100);
	* instrport = 0x38;
	CYGACC_CALL_IF_DELAY_US(40);
	* instrport = 0x08;  // 08h	-- Display, Cursor,and Blink OFF 
	CYGACC_CALL_IF_DELAY_US(40);
	* instrport = 0x0f;  // 0Fh	-- Display, Cursor,and Blink ON
	CYGACC_CALL_IF_DELAY_US(1640);
	* instrport = 0x06;  // 06h	-- Cursor moves right, no shift
	CYGACC_CALL_IF_DELAY_US(40);
	* instrport = 0x06;  // 01h -- Clears display and returns cursor to home postion
	CYGACC_CALL_IF_DELAY_US(1640);
}


/* Writes a message to the LCD.  Expects a pointer to  a string of charactors.  
The line number is the DD RAM address to start writing the charactors. */
 
void write_lcd (char line_num, int mes_len, char *message)
{
	char *dataport;
	char *instrport;
	dataport = (char*)LCD_DR;
	instrport = (char*)LCD_IR;
	int j;

	*instrport = line_num;
	CYGACC_CALL_IF_DELAY_US(40);

	for (j=0; j< mes_len; j++)
	{
		*dataport = *(message+j);
		CYGACC_CALL_IF_DELAY_US(40);
	}
}


void lcd_menu (MENU_ARG arg)
{
	cyg_uint32 choice;
	static unsigned char mes_data[] = "0123456789ABCDEFGHIJ";
	char *dataport;
	char *instrport;
	dataport = (char*)LCD_DR;
	instrport = (char*)LCD_IR;

	if (strcmp(HAL_PLATFORM_BOARD, "IQ80315")) {
		diag_printf ("No LCD on this board\n");
		return;
	}

	do {
		init_lcd ();
		diag_printf ("\n\n");
		diag_printf ("+======================================+\n");
		diag_printf ("|        LCD Menu for IQ80315          |\n");
		diag_printf ("+======================================+\n");
		diag_printf ("| 1) Fill 1x16 Display                 |\n");
		diag_printf ("| 2) Fill 2x16 Display                 |\n");
		diag_printf ("| 3) Fill 1x20 Display                 |\n");
		diag_printf ("| 4) Fill 2x20 Display                 |\n");
		diag_printf ("| 5) Fill 4x20 Display                 |\n");
		diag_printf ("| 6) Fill 1x40 Display                 |\n");
		diag_printf ("| 7) Fill 2x40 Display                 |\n");
		diag_printf ("| 8) Clear Display                     |\n");
		diag_printf ("+--------------------------------------+\n");
		choice = decIn();
		switch (choice) {
		case 1:
			write_lcd (LINE1, 8, &mes_data[0]);
			write_lcd (LINE2, 8, &mes_data[8]);

			break;
		case 2:
			write_lcd (LINE1, 16, &mes_data[0]);
			write_lcd (LINE2, 16, &mes_data[0]);

			break;
		case 3:
			write_lcd (LINE1, 20, &mes_data[0]);

			break;
		case 4:
			write_lcd (LINE1, 20, &mes_data[0]);
			write_lcd (LINE2, 20, &mes_data[0]);
			break;		
		case 5:
			write_lcd (LINE1, 20, &mes_data[0]);
			write_lcd (LINE2, 20, &mes_data[0]);
			write_lcd (LINE3, 20, &mes_data[0]);
			write_lcd (LINE4, 20, &mes_data[0]);
			break;
		case 6:
			write_lcd (LINE1, 20, &mes_data[0]);
			write_lcd (LINE2, 20, &mes_data[0]);

			break;
		case 7:
			write_lcd (LINE1, 20, &mes_data[0]);
			write_lcd (LINE3, 20, &mes_data[0]); //40 charactor line 1 is the same as 20 charactor lines 1 & 3.
			write_lcd (LINE2, 20, &mes_data[0]);
			write_lcd (LINE4, 20, &mes_data[0]); //40 charactor line 2 is the same as 20 charactor lines 2 & 4.
			break;
		case 8:
			*instrport = CLEAR_SCREEN;
			CYGACC_CALL_IF_DELAY_US(40);
			break;
		default:
			diag_printf ("Invalid selection\n");
		}
	} while (choice);
}

