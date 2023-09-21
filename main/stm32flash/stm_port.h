/*
  stm32flash - Open Source ST STM32 flash program for *nix
  Copyright (C) 2014 Antonio Borneo <borneo.antonio@gmail.com>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


#ifndef _H_STM_PORT
#define _H_STM_PORT
#include <stdint.h>
#include <stddef.h> 


typedef struct serial serial_t;

typedef enum {
	SERIAL_PARITY_NONE,
	SERIAL_PARITY_EVEN,
	SERIAL_PARITY_ODD,

	SERIAL_PARITY_INVALID
} serial_parity_t;

typedef enum {
	SERIAL_BITS_5,
	SERIAL_BITS_6,
	SERIAL_BITS_7,
	SERIAL_BITS_8,

	SERIAL_BITS_INVALID
} serial_bits_t;

typedef enum {
	SERIAL_BAUD_1200,
	SERIAL_BAUD_1800,
	SERIAL_BAUD_2400,
	SERIAL_BAUD_4800,
	SERIAL_BAUD_9600,
	SERIAL_BAUD_14400,
	SERIAL_BAUD_19200,
	SERIAL_BAUD_38400,
	SERIAL_BAUD_56000,
	SERIAL_BAUD_57600,
	SERIAL_BAUD_115200,
	SERIAL_BAUD_128000,
	SERIAL_BAUD_230400,
	SERIAL_BAUD_256000,
	SERIAL_BAUD_460800,
	SERIAL_BAUD_500000,
	SERIAL_BAUD_576000,
	SERIAL_BAUD_921600,
	SERIAL_BAUD_1000000,
	SERIAL_BAUD_1152000,
	SERIAL_BAUD_1500000,
	SERIAL_BAUD_2000000,
	SERIAL_BAUD_2500000,
	SERIAL_BAUD_3000000,
	SERIAL_BAUD_3500000,
	SERIAL_BAUD_4000000,

	SERIAL_BAUD_INVALID,
	SERIAL_BAUD_KEEP
} serial_baud_t;

typedef enum {
	SERIAL_STOPBIT_1,
	SERIAL_STOPBIT_2,

	SERIAL_STOPBIT_INVALID
} serial_stopbit_t;


typedef enum {
	PORT_ERR_OK = 0,
	PORT_ERR_NODEV,		/* No such device */
	PORT_ERR_TIMEDOUT,	/* Operation timed out */
	PORT_ERR_BAUD,		/* Unsupported baud rate */
	PORT_ERR_UNKNOWN,
} port_err_t;

/* flags */
#define PORT_BYTE	(1 << 0)	/* byte (not frame) oriented */
#define PORT_GVR_ETX	(1 << 1)	/* cmd GVR returns protection status */
#define PORT_CMD_INIT	(1 << 2)	/* use INIT cmd to autodetect speed */
#define PORT_RETRY	(1 << 3)	/* allowed read() retry after timeout */
#define PORT_STRETCH_W	(1 << 4)	/* warning for no-stretching commands */
#define PORT_NPAG_CSUM	(1 << 5)	/* checksum after number of pages to erase */

/* all options and flags used to open and configure an interface */
struct port_options {
	const char *device;
	serial_baud_t baudRate;
	const char *serial_mode;
	int bus_addr;
	int rx_frame_max;
	int tx_frame_max;
};

/*
 * Specify the length of reply for command GET
 * This is helpful for frame-oriented protocols, e.g. i2c, to avoid time
 * consuming try-fail-timeout-retry operation.
 * On byte-oriented protocols, i.e. UART, this information would be skipped
 * after read the first byte, so not needed.
 */
struct varlen_cmd {
	uint8_t version;
	uint8_t length;
};

typedef struct stm_port_interface {
	const char *name;
	unsigned flags;
	port_err_t (*open)(struct stm_port_interface *port, struct port_options *ops);
	port_err_t (*close)(struct stm_port_interface *port);
	port_err_t (*flush)(struct stm_port_interface *port);
	port_err_t (*read)(struct stm_port_interface *port, void *buf, size_t nbyte);
	port_err_t (*write)(struct stm_port_interface *port, void *buf, size_t nbyte);
	//port_err_t (*gpio)(struct stm_port_interface *port, serial_gpio_t n, int level);
	const char *(*get_cfg_str)(struct stm_port_interface *port);
	struct varlen_cmd *cmd_get_reply;
	void *private;
} stm_port_interface_t;

port_err_t port_open(struct port_options *ops, struct port_interface **outport);

#endif
