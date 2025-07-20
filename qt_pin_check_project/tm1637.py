import time
import lgpio

# Segment byte maps for characters and digits
SEGMENTS = {
    ' ': 0x00, '-': 0x40, '0': 0x3F, '1': 0x06, '2': 0x5B, '3': 0x4F,
    '4': 0x66, '5': 0x6D, '6': 0x7D, '7': 0x07, '8': 0x7F, '9': 0x6F,
    'A': 0x77, 'b': 0x7C, 'C': 0x39, 'd': 0x5E, 'E': 0x79, 'F': 0x71,
    'o': 0x5C, 'k': 0x75, 'r': 0x50
}

_SEGMENTS = bytearray(
    b'\x3F\x06\x5B\x4F\x66\x6D\x7D\x07\x7F\x6F\x77\x7C\x39\x5E\x79\x71\x3D\x76\x06\x1E\x76\x38\x55\x54\x3F\x73\x67'
    b'\x50\x6D\x78\x3E\x1C\x2A\x76\x6E\x5B\x00\x40\x63')



class TM1637:
    def __init__(self, clk, dio, brightness=7):
        self.clk = clk
        self.dio = dio
        self.brightness = brightness & 0x07
        self.handle = lgpio.gpiochip_open(0)
        lgpio.gpio_claim_output(self.handle, self.clk)
        lgpio.gpio_claim_output(self.handle, self.dio)
        self.clear()

    def _start(self):
        lgpio.gpio_write(self.handle, self.dio, 1)
        time.sleep(0.00001)
        lgpio.gpio_write(self.handle, self.clk, 1)
        time.sleep(0.00001)
        lgpio.gpio_write(self.handle, self.dio, 0)
        time.sleep(0.00001)
        lgpio.gpio_write(self.handle, self.clk, 0)
        time.sleep(0.00001)

    def _stop(self):
        lgpio.gpio_write(self.handle, self.clk, 0)
        time.sleep(0.00001)
        lgpio.gpio_write(self.handle, self.dio, 0)
        time.sleep(0.00001)
        lgpio.gpio_write(self.handle, self.clk, 1)
        time.sleep(0.00001)
        lgpio.gpio_write(self.handle, self.dio, 1)
        time.sleep(0.00001)

    def _write_byte(self, b):
        for i in range(8):
            lgpio.gpio_write(self.handle, self.dio, (b >> i) & 1)
            time.sleep(0.00001)
            lgpio.gpio_write(self.handle, self.clk, 1)
            time.sleep(0.00001)
            lgpio.gpio_write(self.handle, self.clk, 0)
            time.sleep(0.00001)
        # Skip ACK
        lgpio.gpio_write(self.handle, self.clk, 0)
        time.sleep(0.00001)
        lgpio.gpio_write(self.handle, self.clk, 1)
        time.sleep(0.00001)
        lgpio.gpio_write(self.handle, self.clk, 0)

    def _encode(self, char):
        return SEGMENTS.get(char.upper(), 0x00)
	
    def _encode_char(self, char):
        """Convert a character 0-9, a-z, space, dash or star to a segment."""
        o = ord(char)
        if o == 32:
            return _SEGMENTS[36]  # space
        if o == 42:
            return _SEGMENTS[38]  # star/degrees
        if o == 45:
            return _SEGMENTS[37]  # dash
        if 65 <= o <= 90:
            return _SEGMENTS[o - 55]  # uppercase A-Z
        if 97 <= o <= 122:
            return _SEGMENTS[o - 87]  # lowercase a-z
        if 48 <= o <= 57:
            return _SEGMENTS[o - 48]  # 0-9
        raise ValueError("Character out of range: {:d} '{:s}'".format(o, chr(o)))

    def encode_string(self, string):
        """Convert an up to 4 character length string containing 0-9, a-z,
        space, dash, star to an array of segments, matching the length of the
        source string."""
        segments = bytearray(len(string))
        for i in range(len(string)):
            segments[i] = self.encode_char(string[i])
        return segments

    def show(self, data):
        # Convert to 4-character string
        s = str(data)[:4].rjust(4)
        encoded = [self._encode_char(c) for c in s]

        self._start()
        self._write_byte(0x40)  # Command: set auto-increment mode
        self._stop()

        self._start()
        self._write_byte(0xC0)  # Address command
        for seg in encoded:
            self._write_byte(seg)
        self._stop()

        self._start()
        self._write_byte(0x88 | self.brightness)  # Display control
        self._stop()

    def clear(self):
        self.show("    ")

    def cleanup(self):
        self.clear()
        lgpio.gpiochip_close(self.handle)
