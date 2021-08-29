# List of all the board related files.
UIPSRC = ${CHIBIOS}/uip/uip.c \
		 ${CHIBIOS}/uip/uip_arp.c \
		 ${CHIBIOS}/uip/uiplib.c \
		 ${CHIBIOS}/uip/psock.c \
#		 ${CHIBIOS}/uip/timer.c \
		 ${CHIBIOS}/uip/uip-neighbor.c \
		 ${CHIBIOS}/uip/uip-fw.c \
		 ${CHIBIOS}/uip/uip-split.c 

# Required include directories
UIPINC = ${CHIBIOS}/uip
