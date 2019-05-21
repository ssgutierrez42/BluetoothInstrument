## Santi Gutierrez


import Adafruit_BluefruitLE
from Adafruit_BluefruitLE.services import UART

import serial
from oscmessage import OSCMessage

from threading import Lock

mutex = Lock()

minPosition = 0
maxPosition = 128

horizontalPosition = -1
verticalPosition = -1

ble = Adafruit_BluefruitLE.get_provider()
ser = None #serial port

try:
    #ser = serial.Serial('/dev/tty.usbmodem2129840', 9600) # bluetooth controller
    ser = serial.Serial('/dev/tty.usbserial-14310', 115200) # drawing machine
    print("[SERIAL] connected")
except:
    print("[ERROR] unable to init serial connection with device.")

def main():
    # Clear any cached data because both bluez and CoreBluetooth have issues with
    # caching data and it going stale.
    ble.clear_cached_data()

    # Get the first available BLE network adapter and make sure it's powered on.
    adapter = ble.get_default_adapter()
    adapter.power_on()
    print('[BLUETOOTH] Using adapter: {0}'.format(adapter.name))

    # Disconnect any currently connected UART devices.  Good for cleaning up and
    # starting from a fresh state.
    print('[BLUETOOTH] Disconnecting any connected UART devices...')
    UART.disconnect_devices()

    # Scan for UART devices.
    print('[BLUETOOTH] Searching for UART device...')
    try:
        adapter.start_scan()
        # Search for the first UART device found (will time out after 60 seconds
        # but you can specify an optional timeout_sec parameter to change it).
        device = UART.find_device()
        if device is None:
            raise RuntimeError('[BLUETOOTH] Failed to find UART device!')
    finally:
        # Make sure scanning is stopped before exiting.
        adapter.stop_scan()

    print('[BLUETOOTH] Connecting to device...')
    device.connect()  # Will time out after 60 seconds, specify timeout_sec parameter
                      # to change the timeout.

    # Wait for service discovery to complete for the UART service.  Will
    # time out after 60 seconds (specify timeout_sec parameter to override).
    print('[BLUETOOTH] Discovering services...')
    UART.discover(device)

    # Once service discovery is complete create an instance of the service
    # and start interacting with it.
    uart = UART(device)

    try:
        print('[BLUETOOTH] listening...')
        print('------------------------\n')

        while device.is_connected:

            # Now wait up to one minute to receive data from the device.
            received = ""
            while "\r\n" not in received:
                received += uart.read()

            if received is not None:
                # Received data, print it out.
                print('[BLUETOOTH] Received: {0}'.format(received))
                result = OSCMessage(received)
                value = result.value

                result = OSCMessage(received)
                if "/range/a" in received:
                    value = result.value
                    updateHorizontal(value)
                elif "/range/b" in received:
                    value = result.value
                    updateVertical(value)
                elif "/button/a" in received:
                    resetMachinePosition()
                #elif "/button/b" in received:
                    #resetMachinePosition()
                elif "/button/c" in received:
                    resetMachinePosition()
                    print("[APP] stopping program...")
                    break #disconnect
    finally:
        device.disconnect()

def updateHorizontal(value):
    mutex.acquire()
    global horizontalPosition

    try:
        updateValue = int(value)

        if horizontalPosition == -1:
            horizontalPosition = updateValue
            return

        delta = updateValue - horizontalPosition
        horizontalPosition = updateValue
        command = 'G21G91G0X{0}Y{0}F10'.format(delta)
        draw(command)
    finally:
        mutex.release()

def updateVertical(value):
    mutex.acquire()
    global verticalPosition

    try:
        updateValue = int(value)

        if verticalPosition == -1:
            verticalPosition = updateValue
            return

        delta = updateValue - verticalPosition
        verticalPosition = updateValue
        command = 'G21G91G0X{0}Y{1}F10'.format(delta, -delta)
        draw(command)
    finally:
        mutex.release()

def resetMachinePosition():
    mutex.acquire()
    try:
        print("[POSITION] reset")
        draw("G90 G0 X0 Y0")
        draw("G90 G0 Z0")
        horizontalPosition = -1
        verticalPosition = -1
    finally:
        mutex.release()

# UP: G21G91G0X-10Y10F10
# DOWN: G21G91G0X10Y-10F10
# LEFT: G21G91G0X10Y10F10
# RIGHT: G21G91G0X-10Y-10F10
def draw(message):
    if ser != None:
        ser.write(message + "\n")
        print("[DRAW] sent: " + message)

def parseOSC(message):
    return OSCMessage(message)

# Initialize the BLE system.  MUST be called before other BLE calls!
ble.initialize()

# Main function implements the program logic so it can run in a background
# thread.  Most platforms require the main thread to handle GUI events and other
# asyncronous events like BLE actions.  All of the threading logic is taken care
# of automatically though and you just need to provide a main function that uses
# the BLE provider.

# Start the mainloop to process BLE events, and run the provided function in
# a background thread.  When the provided main function stops running, returns
# an integer status code, or throws an error the program will exit.
ble.run_mainloop_with(main)
