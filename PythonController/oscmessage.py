## Santi Gutierrez
class OSCMessage:

    def __init__(self, msg):
        self.rawMessage = msg

        msgParts = msg.split()
        value = 0
        self.address = ""

        if len(msgParts) > 0:
            address = msgParts[0]
            self.address = address

        if len(msgParts) > 1:
            value = msgParts[1]

        self.value = value
        self.type = ""
        self.identifier = ""

        addressParts = self.address.split("/")
        for part in addressParts:
            if len(part) <= 0:
                continue

            if len(self.type) <= 0:
                self.type = part
            elif len(self.identifier) <= 0:
                self.identifier = part

    def __str__(self):
        return self.rawMessage
