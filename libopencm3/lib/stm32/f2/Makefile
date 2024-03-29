##
## This file is part of the libopencm3 project.
##
## Copyright (C) 2009 Uwe Hermann <uwe@hermann-uwe.de>
##
## This library is free software: you can redistribute it and/or modify
## it under the terms of the GNU Lesser General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## This library is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU Lesser General Public License for more details.
##
## You should have received a copy of the GNU Lesser General Public License
## along with this library.  If not, see <http://www.gnu.org/licenses/>.
##

LIBNAME		= libopencm3_stm32f2
SRCLIBDIR	?= ../..

CC		= $(PREFIX)gcc
AR		= $(PREFIX)ar
TGT_CFLAGS	= -Os \
		  -Wall -Wextra -Wimplicit-function-declaration \
		  -Wredundant-decls -Wmissing-prototypes -Wstrict-prototypes \
		  -Wundef -Wshadow \
		  -I../../../include -fno-common \
		  -mcpu=cortex-m3 -mthumb $(FP_FLAGS) -Wstrict-prototypes \
		  -ffunction-sections -fdata-sections -MD -DSTM32F2
TGT_CFLAGS      += $(DEBUG_FLAGS)
TGT_CFLAGS	+= $(STANDARD_FLAGS)
# ARFLAGS	= rcsv
ARFLAGS		= rcs

OBJS += crc_common_all.o
OBJS += crypto_common_f24.o
OBJS += dac_common_all.o dac_common_v1.o
OBJS += desig_common_all.o desig_common_v1.o
OBJS += dma_common_f24.o
OBJS += exti_common_all.o
OBJS += flash.o flash_common_all.o flash_common_f.o flash_common_f24.o flash_common_idcache.o
OBJS += gpio_common_all.o gpio_common_f0234.o
OBJS += hash_common_f24.o
OBJS += i2c_common_v1.o
OBJS += iwdg_common_all.o
OBJS += rcc.o rcc_common_all.o
OBJS += rng_common_v1.o
OBJS += rtc_common_l1f024.o
OBJS += spi_common_all.o spi_common_v1.o spi_common_v1_frf.o
OBJS += timer_common_all.o timer_common_f0234.o timer_common_f24.o
OBJS += usart_common_all.o usart_common_f124.o

OBJS += usb.o usb_standard.o usb_control.o usb_msc.o
OBJS += usb_hid.o usb_bos.o usb_microsoft.o
OBJS += usb_audio.o usb_cdc.o usb_midi.o
OBJS += usb_dwc_common.o usb_f107.o usb_f207.o

VPATH += ../../usb:../:../../cm3:../common

include ../../Makefile.include
