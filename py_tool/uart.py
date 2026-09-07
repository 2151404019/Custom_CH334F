import serial
import serial.tools.list_ports
import threading
import time

class UART:
    def __init__(self):
        self.ser = None
        self.running = False
        self.rx_thread = None

    def scan_ports(self):
        ports = []

        devices = serial.tools.list_ports.comports()

        for port in devices:
            ports.append({
                "name": port.device,
                "desc": port.description
            })

        return ports

    def open(self, 
             port, 
             baudrate):
        try:
            self.ser = serial.Serial(port=port,
                                     baudrate=baudrate,
                                     bytesize=serial.EIGHTBITS,
                                     parity=serial.PARITY_NONE,
                                     stopbits=serial.STOPBITS_ONE,
                                     timeout=1)

            self.running = True
            self.rx_thread = threading.Thread(
                target=self.rx_thread,
                daemon=True
            )

            self.rx_thread.start()

            return True
        except Exception as e:
            print("串口打开失败：", e)

            return False

    def send(self, data):
        if self.ser:
            self.ser.write(data)

    # def receive(self):
    #     if self.ser:
    #         count = self.ser.in_waiting

    #         if count:
    #             data = self.ser.read(count)
    #             print(
    #                 "RX:",
    #                 data.hex(" ")
    #             )
    #             return data
            
    #     return None
    def rx_process(self):
        while self.running:
            try:
                if self.ser.in_waiting:
                    data = self.ser.read(self.ser.in_waiting)
                    print("\nRx:", data.hex(" "))

                time.sleep(0.01)
            except:
                break


    def close(self):
        self.running = False

        if self.ser:
            self.ser.close()