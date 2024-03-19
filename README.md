USR-CANET
===========

[USR-CANET200](https://www.pusr.com/products/can-to-ethernet-converters-usr-canet200.html) is an Ethernet to CANbus and RS485 converter manufactured by PUSR. This device features 2 CANbus ports with TCP/UDP server or client to relay CANbus over Ethernet. The interface uses TCP server mode and need to be configured on the web management portal.

The device's official documentation and transparent transmission protocol information can be found `here <https://www.pusr.com/download/CANET/USR-CANET200-User-Manual_V1.0.4.01.pdf>`

## python-can-usr_canet

This module is a plugin that lets you use [USR-CANET200 adapters](https://www.pusr.com/products/can-to-ethernet-converters-usr-canet200.html) in python-can.


### Installation

Install using pip:

    $ pip install git+https://github.com/AgoraRobotics/python-can-usr_canet

### Usage

Overall, using this plugin is quite similar to the main Python-CAN library, with the interface named `usr_canet`. For most scenarios, incorporating a USR-CANET200 interface is as easy as modifying Python-CAN examples with the lines provided below:

Create python-can bus with the CANET200 ETH interface:

    import can
    from usr_canet import UsrCanetBus

    bus = can.Bus(interface="usr_canet", host="192.168.0.7", bitrate=250000)

### Supported platform

Windows, Linux and Mac.

### Limitations

This interface has a simplified implementation of timeout and may not work well in high capacity multi-threaded applications.

### Credits

The code was submited by Tianshu Wei as a [pull request](https://github.com/hardbyte/python-can/pull/1347) to python-can, but now converted to plugin interface

