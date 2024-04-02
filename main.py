import base64
import select
import time
import serial
import struct
from fcntl import ioctl
import select

def openTun(tunName):
    tun = open("/dev/net/tun", "r+b", buffering=0)
    LINUX_IFF_TUN = 0x0001
    LINUX_IFF_NO_PI = 0x1000
    LINUX_TUNSETIFF = 0x400454CA
    flags = LINUX_IFF_TUN | LINUX_IFF_NO_PI
    ifs = struct.pack("16sH22s", tunName, flags, b"")
    ioctl(tun, LINUX_TUNSETIFF, ifs)
    return tun

if __name__ == "__main__":
    tun = openTun(b"tun0")
    serialfd= serial.Serial('/dev/ttyUSB0', 115200, timeout=10)
    while True:
        inputs = [tun, serialfd]
        outputs = []
        inputs,outputs,errs = select.select(inputs, outputs, inputs)
        for fd in inputs:
            if fd == tun:
                packet = tun.read(2000)
                packetb64 = base64.b64encode(packet).decode('ascii') + '\n'
                packetb64 = packetb64.encode()
                serialfd.write(packetb64)
                print(f"converted packet to {len(packetb64) - 1}+1 bytes")
                print(packetb64.decode())
                
            if fd == serialfd:
                pass
                # packetb64 = serialfd.readline().encode('ascii')
                # packet = base64.b64decode(packetb64)

                