#!/usr/bin/env python3

import can
from can.listener import RedirectReader
import sys
import time

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

port = 20001    # 20001 for CAN1, 20005 for CAN2
if len(sys.argv) > 1:
    port = int(sys.argv[1])
print('PORT', port)

usr_canet_bus = can.Bus(interface="usr_canet", host="192.168.0.7", port=port, bitrate=250000)

usr_canet_listener = RedirectReader(bus=vbus)
vbus_listener = RedirectReader(bus=usr_canet_bus)
print_listener = can.Printer()

can.Notifier(usr_canet_bus, [usr_canet_listener, print_listener])   # does not print outgoing messages from other senders
can.Notifier(vbus, [vbus_listener, print_listener])

try:
    while True:
        time.sleep(1)
except KeyboardInterrupt:
    pass
finally:
    usr_canet_bus.shutdown()
    vbus.shutdown()
