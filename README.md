# ST7580-LINUX
 Port of the ST X-CUBE-PLM1 driver to GNU/Linux. Works on any device that can run GNU/Linux, as long as you got usable RTS/DTR serial/UART pins.

## CP210x USB to UART converter wiring to the X-NUCLEO-PLM01A1 board

Beware of the low performance when using such an USB converter, this is due to the way the USB communication is handled, see here: https://www.silabs.com/community/interface/knowledge-base.entry.html/2004/04/20/cp210x_usb_communica-fDS1

| CP210x | X-NUCLEO-PLM01A1 |
|--------|------------------|
| 3V3    | CN6/2            |
| GND    | CN6/7            |
| TXD    | CN9/3            |
| RXD    | CN5/1            |
| DTR    | CN9/8            |
| RTS    | CN5/6            |