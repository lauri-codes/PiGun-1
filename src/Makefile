# Makefile for Raspberry Pi

# use (cross)compiler for Raspi
CC = arm-linux-gnueabihf-gcc

# location of the btstack main folder
BTSTACK_ROOT ?= ../btstack

#--- DO NOT EDIT BEYOND THIS POINT --------#
#--- unless you know what you are doing ---#

CORE += \
	btstack_chipset_bcm.c \
	btstack_chipset_bcm_download_firmware.c \
	btstack_control_raspi.c \
	btstack_link_key_db_tlv.c \
	btstack_run_loop_posix.c \
	btstack_tlv_posix.c \
	btstack_uart_posix.c \
	btstack_slip.c \
	hci_dump_posix_fs.c \
	hci_transport_h4.c \
	hci_transport_h5.c \
	le_device_db_tlv.c \
	wav_util.c 					\
	btstack_stdin_posix.c \
	raspi_get_model.c \
	rijndael.c



VPATH += ${BTSTACK_ROOT}/src
VPATH += ${BTSTACK_ROOT}/src/ble
VPATH += ${BTSTACK_ROOT}/src/ble/gatt-service
VPATH += ${BTSTACK_ROOT}/src/classic
VPATH += ${BTSTACK_ROOT}/src/mesh
VPATH += ${BTSTACK_ROOT}/example
VPATH += ${BTSTACK_ROOT}/3rd-party/bluedroid/decoder/srce
VPATH += ${BTSTACK_ROOT}/3rd-party/bluedroid/encoder/srce
VPATH += ${BTSTACK_ROOT}/3rd-party/hxcmod-player
VPATH += ${BTSTACK_ROOT}/3rd-party/hxcmod-player
VPATH += ${BTSTACK_ROOT}/3rd-party/hxcmod-player/mods
VPATH += ${BTSTACK_ROOT}/3rd-party/lwip/core/src/core/
VPATH += ${BTSTACK_ROOT}/3rd-party/lwip/core/src/core/ipv4
VPATH += ${BTSTACK_ROOT}/3rd-party/lwip/core/src/core/ipv6
VPATH += ${BTSTACK_ROOT}/3rd-party/lwip/core/src/netif
VPATH += ${BTSTACK_ROOT}/3rd-party/lwip/core/src/apps/http
VPATH += ${BTSTACK_ROOT}/3rd-party/lwip/dhcp-server
VPATH += ${BTSTACK_ROOT}/3rd-party/md5
VPATH += ${BTSTACK_ROOT}/3rd-party/micro-ecc
VPATH += ${BTSTACK_ROOT}/3rd-party/yxml
VPATH += ${BTSTACK_ROOT}/platform/lwip
VPATH += ${BTSTACK_ROOT}/platform/lwip/port

CFLAGS += -I.
CFLAGS += -I${BTSTACK_ROOT}/src/ble
CFLAGS += -I${BTSTACK_ROOT}/src/classic
CFLAGS += -I${BTSTACK_ROOT}/src/mesh
CFLAGS += -I${BTSTACK_ROOT}/src
CFLAGS += -I${BTSTACK_ROOT}/3rd-party/bluedroid/decoder/include
CFLAGS += -I${BTSTACK_ROOT}/3rd-party/bluedroid/encoder/include
CFLAGS += -I${BTSTACK_ROOT}/3rd-party/hxcmod-player
CFLAGS += -I${BTSTACK_ROOT}/3rd-party/lwip/core/src/include/
CFLAGS += -I${BTSTACK_ROOT}/3rd-party/lwip/dhcp-server
CFLAGS += -I${BTSTACK_ROOT}/3rd-party/md5
CFLAGS += -I${BTSTACK_ROOT}/3rd-party/micro-ecc
CFLAGS += -I${BTSTACK_ROOT}/3rd-party/yxml
CFLAGS += -I${BTSTACK_ROOT}/platform/lwip
CFLAGS += -I${BTSTACK_ROOT}/platform/lwip/port

# for CVSD/SBC PLC
LDFLAGS += -lm

CORE += \
	btstack_memory.c            \
	btstack_linked_list.c	    \
	btstack_memory_pool.c       \
	btstack_run_loop.c		    \
	btstack_util.c 	            \

COMMON += \
	ad_parser.c                 \
	hci.c			            \
	hci_cmd.c		            \
	hci_dump.c		            \
	l2cap.c			            \
	l2cap_signaling.c	        \
	btstack_audio.c             \
	btstack_tlv.c               \
	btstack_crypto.c            \
	uECC.c                      \
	sm.c                        \

CLASSIC ?= \
	sdp_util.c	                \
	gatt_sdp.c                  \
	spp_server.c  				\
	rfcomm.c	                \
	bnep.c	                    \
	sdp_server.c			    \
	device_id_server.c          \

SDP_CLIENT_SRC += \
	sdp_client.c		        \
	sdp_client_rfcomm.c		    \


SDP_CLIENT += \
	sdp_client.o		        \
	sdp_client_rfcomm.o		    \

# .o for .c
CORE_OBJ    = $(CORE:.c=.o)
COMMON_OBJ  = $(COMMON:.c=.o)
CLASSIC_OBJ  = $(CLASSIC:.c=.o)

CFLAGS  += -g -Wall -Werror -O3 -Wno-restrict \
	-Wno-maybe-uninitialized -Wno-cpp -Wno-comment -Wno-format \
	-Wno-unused-function -Wno-unused-variable -Wno-unused-but-set-variable \
	-I$(BTSTACK_ROOT)/platform/embedded \
	-I$(BTSTACK_ROOT)/platform/posix \
	-I$(BTSTACK_ROOT)/chipset/bcm \
	-I${BTSTACK_ROOT}/3rd-party/tinydir \
    -I${BTSTACK_ROOT}/3rd-party/rijndael

# add 'real time' lib for clock_gettime
LDFLAGS += -lrt -lpthread

VPATH += ${BTSTACK_ROOT}/3rd-party/rijndael
VPATH += ${BTSTACK_ROOT}/platform/posix
VPATH += ${BTSTACK_ROOT}/platform/embedded
VPATH += ${BTSTACK_ROOT}/chipset/bcm

# THESE WILL BE REMOVED IN THIS VERSION
# pigun flags

# PIGUN_FOUR_LEDS enables the four led detection mode
# PIGUN_DEBUG enables some debug output
PIGUNFLAGS = -DPIGUN_FOUR_LEDS


# extra libs no longer used cos they slo AF: -lopencv_shape -lopencv_stitching -lopencv_superres -lopencv_videostab -lopencv_aruco -lopencv_bgsegm -lopencv_bioinspired -lopencv_ccalib -lopencv_datasets -lopencv_dpm -lopencv_face -lopencv_freetype -lopencv_fuzzy -lopencv_hdf -lopencv_line_descriptor -lopencv_optflow -lopencv_video -lopencv_plot -lopencv_reg -lopencv_saliency -lopencv_stereo -lopencv_structured_light -lopencv_phase_unwrapping -lopencv_rgbd -lopencv_viz -lopencv_surface_matching -lopencv_text -lopencv_ximgproc -lopencv_calib3d -lopencv_features2d -lopencv_flann -lopencv_xobjdetect -lopencv_objdetect -lopencv_ml -lopencv_xphoto -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_photo -lopencv_imgproc -lopencv_core
# extra incs for the slo bois:  -I/usr/include/opencv

.PHONY: clean bluetooth-core bluetooth-common bluetooth-classic bluetooth-sdpclient bluetooth-others pigun all

all: bluetooth pigun


bluetooth-core:
	$(info Compiling CORE)

	${CC} -c ${CFLAGS} ${BTSTACK_ROOT}/chipset/bcm/btstack_chipset_bcm.c
	${CC} -c ${CFLAGS} ${BTSTACK_ROOT}/chipset/bcm/btstack_chipset_bcm_download_firmware.c

	${CC} -c ${CFLAGS} ${BTSTACK_ROOT}/port/raspi/btstack_control_raspi.c
	${CC} -c ${CFLAGS} ${BTSTACK_ROOT}/port/raspi/raspi_get_model.c

	${CC} -c ${CFLAGS} ${BTSTACK_ROOT}/src/btstack_slip.c
	${CC} -c ${CFLAGS} ${BTSTACK_ROOT}/src/hci_transport_h4.c
	${CC} -c ${CFLAGS} ${BTSTACK_ROOT}/src/hci_transport_h5.c

	${CC} -c ${CFLAGS} ${BTSTACK_ROOT}/src/classic/btstack_link_key_db_tlv.c
	${CC} -c ${CFLAGS} ${BTSTACK_ROOT}/src/ble/le_device_db_tlv.c

	${CC} -c ${CFLAGS} ${BTSTACK_ROOT}/platform/posix/btstack_run_loop_posix.c
	${CC} -c ${CFLAGS} ${BTSTACK_ROOT}/platform/posix/btstack_tlv_posix.c
	${CC} -c ${CFLAGS} ${BTSTACK_ROOT}/platform/posix/btstack_uart_posix.c
	${CC} -c ${CFLAGS} ${BTSTACK_ROOT}/platform/posix/hci_dump_posix_fs.c
	${CC} -c ${CFLAGS} ${BTSTACK_ROOT}/platform/posix/wav_util.c
	${CC} -c ${CFLAGS} ${BTSTACK_ROOT}/platform/posix/btstack_stdin_posix.c
	
	${CC} -c ${CFLAGS} ${BTSTACK_ROOT}/3rd-party/rijndael/rijndael.c
	
	${CC} -c ${CFLAGS} ${BTSTACK_ROOT}/src/btstack_memory.c
	${CC} -c ${CFLAGS} ${BTSTACK_ROOT}/src/btstack_linked_list.c
	${CC} -c ${CFLAGS} ${BTSTACK_ROOT}/src/btstack_memory_pool.c
	${CC} -c ${CFLAGS} ${BTSTACK_ROOT}/src/btstack_run_loop.c
	${CC} -c ${CFLAGS} ${BTSTACK_ROOT}/src/btstack_util.c

bluetooth-common:
	$(info Compiling COMMON)
	${CC} -c ${CFLAGS} ${BTSTACK_ROOT}/src/ad_parser.c
	${CC} -c ${CFLAGS} ${BTSTACK_ROOT}/src/hci.c
	${CC} -c ${CFLAGS} ${BTSTACK_ROOT}/src/hci_cmd.c
	${CC} -c ${CFLAGS} ${BTSTACK_ROOT}/src/hci_dump.c
	${CC} -c ${CFLAGS} ${BTSTACK_ROOT}/src/l2cap.c
	${CC} -c ${CFLAGS} ${BTSTACK_ROOT}/src/l2cap_signaling.c
	${CC} -c ${CFLAGS} ${BTSTACK_ROOT}/src/btstack_audio.c
	${CC} -c ${CFLAGS} ${BTSTACK_ROOT}/src/btstack_tlv.c
	${CC} -c ${CFLAGS} ${BTSTACK_ROOT}/src/btstack_crypto.c
	${CC} -c ${CFLAGS} ${BTSTACK_ROOT}/3rd-party/micro-ecc/uECC.c
	${CC} -c ${CFLAGS} ${BTSTACK_ROOT}/src/ble/sm.c

bluetooth-classic:
	$(info Compiling CLASSIC)
	${CC} -c ${CFLAGS} ${BTSTACK_ROOT}/src/classic/sdp_util.c
	${CC} -c ${CFLAGS} ${BTSTACK_ROOT}/src/classic/gatt_sdp.c
	${CC} -c ${CFLAGS} ${BTSTACK_ROOT}/src/classic/spp_server.c
	${CC} -c ${CFLAGS} ${BTSTACK_ROOT}/src/classic/rfcomm.c
	${CC} -c ${CFLAGS} ${BTSTACK_ROOT}/src/classic/bnep.c
	${CC} -c ${CFLAGS} ${BTSTACK_ROOT}/src/classic/sdp_server.c
	${CC} -c ${CFLAGS} ${BTSTACK_ROOT}/src/classic/device_id_server.c

bluetooth-sdpclient:
	$(info Compiling SDP_CLIENT)
	${CC} -c ${CFLAGS} ${BTSTACK_ROOT}/src/classic/sdp_client.c
	${CC} -c ${CFLAGS} ${BTSTACK_ROOT}/src/classic/sdp_client_rfcomm.c

bluetooth-others:
	$(info Compiling other bluetooth shit)
	${CC} -c ${CFLAGS} ${BTSTACK_ROOT}/src/btstack_ring_buffer.c
	${CC} -c ${CFLAGS} ${BTSTACK_ROOT}/src/classic/hid_device.c
	${CC} -c ${CFLAGS} ${BTSTACK_ROOT}/src/btstack_hid_parser.c


# this one compiles all the BTStack only parts
bluetooth: bluetooth-core bluetooth-common bluetooth-classic bluetooth-sdpclient bluetooth-others

DEPS = $(wildcard *.h)
PIGUN_SRC := pigun-hid.c pigun-libcamera.c pigun-detector.c pigun-aimer.c pigun-gpio.c pigun-helpers.c pigun.c main.c
PIGUN_OBJ := $(patsubst %.c,%.o,$(PIGUN_SRC))

PKGCONFIG         = pkg-config
LIBCAMERA_CFLAGS    = $(shell $(PKGCONFIG) --cflags libcamera)
LIBCAMERA_LIBS      = $(shell $(PKGCONFIG) --libs libcamera)

CXX = arm-linux-gnueabihf-g++
# %.o: %.cpp
#     $(CXX) -c $(CFLAGS) -o $@ $<

%.o: %.c $(DEPS)
	${CXX} -c -Wno-narrowing ${CFLAGS} ${PIGUNFLAGS} -o $@ $<

pigun: $(PIGUN_OBJ)
	${CXX} -O3 ${LIBCAMERA_LIBS} *.o -o pigun.exe ${LIBCAMERA_CFLAGS} -lstdc++


clean:
	rm -f *.o pigun.exe