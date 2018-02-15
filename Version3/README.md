# LightRay: Version 3

[Transimpedance amplifier](https://en.wikipedia.org/wiki/Transimpedance_amplifier)



[Everlight PD333](http://www.everlight.com/file/ProductFile/PD333-3C-H0-L2.pdf)


[Texas Instruments TLV271](http://www.ti.com/lit/ds/symlink/tlv274.pdf)

[SN74HCU04N](http://www.ti.com/lit/ds/symlink/sn74hcu04.pdf)

[Serial Baud Rates, Bit Timing and Error Tolerance](http://www.picaxe.com/docs/baudratetolerance.pdf)

## Raspberry Pi serial configuration

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
