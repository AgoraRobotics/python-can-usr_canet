#!/usr/bin/env python3

import can
import time
# from usr_canet import UsrCanetBus

__USAGE__ = """
sudo modprobe vcan
sudo ip link add dev vcan0 type vcan
sudo ip link set up vcan0
"""

try:
    vbus = can.Bus(interface="socketcan", channel="vcan0")
except OSError as e:
    print(__USAGE__)
    exit(1)

usr_canet_bus = can.Bus(interface="usr_canet", host="192.168.0.7", bitrate=250000)

class ForwardListener(can.Listener):
    def __init__(self, other_bus):
        super().__init__()
        self.other_bus = other_bus

    def on_message_received(self, msg: can.Message) -> None:
        self.other_bus.send(msg)
        print(msg)

usr_canet_listener = ForwardListener(other_bus=vbus)
vbus_listener = ForwardListener(other_bus=usr_canet_bus)

can.Notifier(usr_canet_bus, [usr_canet_listener])
can.Notifier(vbus, [vbus_listener])

try:
    while True:
        time.sleep(1)
except KeyboardInterrupt:
    pass
finally:
    usr_canet_bus.shutdown()
    vbus.shutdown()
