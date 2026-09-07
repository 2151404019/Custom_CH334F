HEAD = [
    0x4D,
    0x58,
    0x51
]

def crc8(data):
    crc = 0

    for byte in data:
        crc ^= byte

        for i in range(8):
            if crc & 0x80:
                crc = ((crc << 1) ^ 0x07) & 0xff
            else:
                crc = (crc << 1) & 0xff

    return crc

def make_frame(cmd, data):
    frame = []

    frame.extend(HEAD)

    length = len(data) + 1      #data + cmd

    frame.append(length)
    frame.append(cmd)
    frame.extend(data)

    #crc
    crc_data = []

    crc_data.append(cmd)
    crc_data.extend(data)

    crc = crc8(crc_data)

    frame.append(crc);

    return bytes(frame)

