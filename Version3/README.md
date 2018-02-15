# LightRay: Version 3

In [Version 1](../Version1/README.md) and [Version 2](../Version2/README.md) we focussed on controlling the sending and receiving of every single bit in software. This puts some heavy load on the CPU and you need some real-time capabilities in the system which is why we focussed on using microcontrollers without any operating system. The basic idea for this version is to use the UART of an Arduino and an Raspberry Pi to do the heavy lifting.

## Schematic

### Sender

The sender schematic is derived from a minimal schematic for an ATmega328P 8-bit microcontroller, the heart of the *Arduino Uno*.

![Sender schematics](schematic_sender.png)

The laser module is the same as in Version 1, just that the modulation input is connection with the TX pin of the serial output of the microcontroller. The TX pin is high when in idle mode and - that is very convenient - the laser is low active, too, so it is turned off when idle.

There is a 6-pin AVR ISP programming port to program the microcontroller, e.g. with an [USBtinyISP](https://learn.adafruit.com/usbtinyisp/overview).

To have something useful to send, we integrated a [Bosch BME280](https://www.adafruit.com/product/2652) sensor for temperature, pressure and humidity.

The switch S1 is used to switch the sender into a *test mode*: In *normal mode* the sender would send an update maybe every 5 minutes. In *test mode* there is continuous sending of test data which makes the alignment of laser and photo diode easier. Once this is done, the system can be switched back to *normal mode*.

### Receiver

This is the receiver schematic.

![Receiver schematics](schematic_receiver.png)

The underlying concept of this schematic is the so-called [Transimpedance amplifier](https://en.wikipedia.org/wiki/Transimpedance_amplifier) using an operational amplifier to convert the small current of the photo diode into a voltage. We picked a [Texas Instruments TLV271](http://www.ti.com/lit/ds/symlink/tlv274.pdf) as opamp and an [Everlight PD333](http://www.everlight.com/file/ProductFile/PD333-3C-H0-L2.pdf) as the photo diode.

The whole system is powered by the 3.3V of the Raspberry Pi 3 and the output of the opamp is fed into the RX pin of the Raspberry Pi.

Unfortunately is the output of the opamp just the inverse of what we need: Sender TX high, laser off, photo current low, voltage output of opamp low, Receiver RX low. So we use a standard Hex-Inverter chip [SN74HCU04N](http://www.ti.com/lit/ds/symlink/sn74hcu04.pdf) to logically invert the signal.

## Software

### Sender

### Receiver

Disable Linux usage of under **Interfacing options** and **Serial**
```
sudo raspi-config
```
Add line to /boot/config.txt to restore ```/dev/ttyAMA0``` to GPIOs 14 (TX, pin 8) and 15 (RX, pin 10)
```
dtoverlay=pi3-disable-bt
```
Stop Bluetooth services
```
sudo systemctl disable hciuart
```


[Raspberry Pi UARTS](https://www.raspberrypi.org/documentation/configuration/uart.md)


## Performance

![Sender timing](sender_timing.png)

![Receiver timing](receiver_timing.png)

[Serial Baud Rates, Bit Timing and Error Tolerance](http://www.picaxe.com/docs/baudratetolerance.pdf)
