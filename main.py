import base64
import select
import serial
import struct
from fcntl import ioctl

def openTun(tunName):
    tun = open("/dev/net/tun", "r+b", buffering=0)
    LINUX_IFF_TUN = 0x0001
    LINUX_IFF_NO_PI = 0x1000
    LINUX_TUNSETIFF = 0x400454CA
    flags = LINUX_IFF_TUN | LINUX_IFF_NO_PI
    ifs = struct.pack("16sH22s", tunName, flags, b"")
    ioctl(tun, LINUX_TUNSETIFF, ifs)
    return tun

def readBytes(serialfd):
    try:
        p64 = serialfd.readline()
        #print(len(p64), "Bytes recvd")
        if len(p64) <= 2:
            return b''
        packet = base64.b64decode(p64)
        #print(f"received {p64} --> {packet}")
        return packet
    except:
        return b''

def sendBytes(data, serialfd):
    p64 = base64.b64encode(data)
    p64 = p64.decode('ascii').encode()
    serialfd.write(p64)

if __name__ == "__main__":
    tun = openTun(b"tun0")
    serialfd= serial.Serial('/dev/ttyUSB0', 500000, timeout=10)
    while True:
        inputs = [tun, serialfd]
        outputs = []
        inputs,outputs,errs = select.select(inputs, outputs, inputs)
        for fd in inputs:
            if fd == tun:
                packet = tun.read(2000)
                sendBytes(packet, serialfd)
                print("________________________")
                print("FROM TUN")
                print(packet)
                
            if fd == serialfd:
                packet = readBytes(serialfd)
                if packet == b'':
                    continue
                tun.write(packet)
                print("________________________")
                print("FROM SERIAL")

                
