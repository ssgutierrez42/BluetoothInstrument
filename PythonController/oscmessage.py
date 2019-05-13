## Santi Gutierrez
class OSCMessage:

    def __init__(self, msg):
        self.rawMessage = msg

        msgParts = msg.split()
        address = msgParts[0]
        value = 0
        if len(msgParts) > 1:
            value = msgParts[1]

        self.address = address
        self.value = value
        self.type = ""
        self.identifier = ""

        addressParts = address.split("/")
        for part in addressParts:
            if len(part) <= 0:
                continue

            if len(self.type) <= 0:
                self.type = part
            elif len(self.identifier) <= 0:
                self.identifier = part

    def __str__(self):
        return self.rawMessage
