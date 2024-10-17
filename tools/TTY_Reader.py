import serial
import time

class TTY_Reader:
    cmd_sleep = 0.5

    def __init__(self, tty_name):
        self.ser = serial.Serial()
        self.ser.port = tty_name
        # If it breaks try the below
        #self.serConf() # Uncomment lines here till it works

        self.ser.open()
        self.ser.flushInput()
        self.ser.flushOutput()

        self.addr = None
        self.setAddress(0)

    def cmd(self, cmd_str):
        self.ser.write(cmd_str + "\n")
        time.sleep(self.cmd_sleep)
        return self.ser.readline()

    def serConf(self):
        self.ser.baudrate = 9600
        self.ser.bytesize = serial.EIGHTBITS
        self.ser.parity = serial.PARITY_NONE
        self.ser.stopbits = serial.STOPBITS_ONE
        self.ser.timeout = 0 # Non-Block reading
        self.ser.xonxoff = False # Disable Software Flow Control
        self.ser.rtscts = False # Disable (RTS/CTS) flow Control
        self.ser.dsrdtr = False # Disable (DSR/DTR) flow Control
        self.ser.writeTimeout = 2

    def close(self):
        self.ser.close()

    def abort(self):
        self.cmd('A\n')