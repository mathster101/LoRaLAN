// from https://blog.mbedded.ninja/programming/
// operating-systems/linux/linux-serial-ports-using-c-cpp/

#include <stdio.h>
#include <string.h>
#include <fcntl.h>   // Contains file controls like O_RDWR
#include <errno.h>   // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h>  // write(), read(), close()

#define device "/dev/ttyUSB1"

int setupSerial()
{
    int serial_port = open(device, O_RDWR| O_NOCTTY);

    // Check for errors
    if (serial_port < 0)
    {
        printf("Error %i from open: %s\n", errno, strerror(errno));
    }

    struct termios tty;
    if (tcgetattr(serial_port, &tty) != 0)
    {
        printf("Error %i from tcgetattr %s\n", errno, strerror(errno));
    }
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~CRTSCTS;
    tty.c_cflag |= CREAD | CLOCAL;
    //tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO;
    tty.c_lflag &= ~ECHOE;
    tty.c_lflag &= ~ECHONL;
    tty.c_lflag &= ~ISIG;
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);
    tty.c_oflag &= ~OPOST;
    tty.c_oflag &= ~ONLCR;

    tty.c_cc[VTIME] = 10; // Wait for up to 1s (10 deciseconds)
    tty.c_cc[VMIN] = 0;

    cfsetispeed(&tty, B115200);
    cfsetospeed(&tty, B115200);

    if (tcsetattr(serial_port, TCSANOW, &tty) != 0)
    {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
    }
    return serial_port;
}

void readLine(int serial_port, char* recvBuffer)
{
    char buffer[2000];
    int doneReading = 0;
    int index = 0;
    while(doneReading == 0)
    {
        char recvChar;
        int isPresent = read(serial_port, &recvChar, sizeof(recvChar));
        if(isPresent == 0)
            continue;
        buffer[index++] = recvChar;
        if(recvChar == '\n')
        {
            buffer[index] = '\0';
            doneReading = 1;
        }
    }
    strcpy(recvBuffer, buffer);
}


int main()
{

    int serial_port = setupSerial();
    while(1)
    {
        char buffer[2000];
        readLine(serial_port, buffer);
        printf("%s\n", buffer);
    }

    return 0;
}