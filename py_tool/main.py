from uart import UART
from protocol import make_frame
from file import read_hex_file
from path_complete import input_path

uart = UART()

ports = uart.scan_ports()

if len(ports) == 0:
    print("没有搜索到串口")

    exit()

print("=========================")

for i,p in enumerate(ports):
    print(i + 1, ".", p["name"], p["desc"])

print("=========================")

while True:
    index = input(
                "选择串口编号："
            )
    try:
        index = int(index)

        if index>=1 and index<=len(ports):

            break
    except Exception as e:
        pass

port = ports[index-1]["name"]

baud_list=[
    9600,
    19200,
    57600,
    115200,
    460800,
    921600
]

print(
"""
选择波特率:
1  9600
2  19200
3  57600
4  115200
5  460800
6  921600
"""
)

while True:
    try:
        baud_index = int(input("选择："))

        if 1 <= baud_index <= len(baud_list):
            break
    except:
        pass

baudrate=baud_list[baud_index-1]

if uart.open(
    port,
    baudrate
):
    print("串口打开成功：",
          port,
          baudrate)
else:
    exit()

while True:
    print(
"""
========================
1.发送文件
2.发送空命令
3.退出
========================
""")
    cmd = input("选择：")

    if cmd == "1":
        cmd_hex = input("CMD(hex):")
        cmd_value = int(cmd_hex, 16)
        filename = input_path("数据文件：")
        data = read_hex_file(filename)
        frame = make_frame(cmd_value, data)

        print("发送帧：")
        print(frame.hex(" "))
        uart.send(frame)
        # uart.receive()
    elif cmd == "2":
        frame = make_frame(0x01, [])

        uart.send(frame)
    elif cmd == "3":
        uart.close()

        break
