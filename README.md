## python-can-usr_canet



This module is a plugin that lets you use [USR-CANET200 adapters](https://www.pusr.com/products/can-to-ethernet-converters-usr-canet200.html) in python-can.


### Installation

Install using pip:

    $ pip install python-can-usr_canet


### Usage

Overall, using this plugin is quite similar to the main Python-CAN library, with the interface named `usr_canet`. For most scenarios, incorporating a USR-CANET200 interface is as easy as modifying Python-CAN examples with the lines provided below:

Create python-can bus with the CANET200 ETH interface:

    import can
    from usr_canet import UsrCanetBus

    bus = can.Bus(interface="usr_canet", host="192.168.0.7", bitrate=250000)
