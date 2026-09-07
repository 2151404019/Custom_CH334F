def read_hex_file(filename):
    data = []

    with open(filename, "r") as f:
        text = f.read()

    items = text.replace(
        "\n",
        " "
    ).split()

    for item in items:
        data.append(int(item, 16))

    return data