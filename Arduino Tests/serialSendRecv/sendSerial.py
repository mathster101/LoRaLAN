import time
import serial


text = "Not only is the Beagle an excellent hunting dog and loyal companion, it is also a happy-go-lucky and funny doggy\n"

def foo():
    print("sent")
    seraph= serial.Serial('/dev/ttyUSB0', 921600, timeout=10)
    mlem = text.encode()
    for i in range(10):
        #print('wrote', mlem)
        seraph.write(mlem)
        start = time.time()
        line = seraph.readline()
        end = time.time()
        print(f"{i}--->{line.decode()}")
        print(end - start)
    seraph.close()

def moo():
    print("sent")
    seraph= serial.Serial('/dev/ttyUSB0', 115200, timeout=10)
    mlem = text.encode()
    counter = 0
    while True:    
        line = seraph.readline()
        print(f"{counter}-->{line.decode()}")
        counter += 1
    seraph.close()	

moo()
