class SMBus:
    """Implement the basic SMBus commands."""

    def __init__(self, i2c, addr, pec=False):
        if pec:
            raise NotImplementedError("PEC not supported")
        self.i2c = i2c
        self.addr = addr

    def write_quick(self):
        """Perform a QuickCommand write."""
        return self.i2c.write(self.addr, b'')

    def read_byte(self):
        """Perform a Receive Byte."""
        return self.i2c.read(self.addr, 1)[0]

    def write_byte(self, val):
        """Perform a Send Byte."""
        return self.i2c.write(self.addr, val.to_bytes(1))

    def read_byte_data(self, cmd):
        """Perform a Read Byte."""
        self.i2c.write(self.addr, cmd.to_bytes(1), repeat=True)
        return int.from_bytes(self.i2c.read(self.addr, 1))

    def write_byte_data(self, cmd, val):
        """Perform a Write Byte."""
        self.i2c.write(self.addr, cmd.to_bytes(1) + val.to_bytes(1))

    def read_word_data(self, cmd):
        """Perform a Read Word."""
        self.i2c.write(self.addr, cmd.to_bytes(1), repeat=True)
        return int.from_bytes(self.i2c.read(self.addr, 2))

    def write_byte_data(self, cmd, val):
        """Perform a Write Word."""
        self.i2c.write(self.addr, cmd.to_bytes(1) + val.to_bytes(2)))

    def read_block_data(self, cmd):
        """Perform a Block Read."""
        self.i2c.write(self.addr, cmd.to_bytes(1), repeat=True)
        # XXX this should be a single transaction...
        count = int.from_bytes(self.i2c.read(self.addr, 1, repeat=True))
        return self.i2c.read(self.addr, count)

    def write_block_data(self, cmd, vals):
        """Perform a Block Write."""
        count = len(vals)
        if count > 255:
            raise ValueError("Data too long")
        self.i2c.write(self.addr, cmd.to_bytes(1) + count.to_bytes(1) + vals)

    def process_call(self, cmd, val):
        self.i2c.write(self.addr, cmd.to_bytes(1) + val.to_bytes(2)),
                       repeat=True)
        return int.from_bytes(self.i2c.read(self.addr, 2))

    def block_process_call(self, cmd, vals):
        count = len(vals)
        if count > 255:
            raise ValueError("Data too long")
        self.i2c.write(self.addr, cmd.to_bytes(1) + count.to_bytes(1) + vals,
                       repeat=True)
        # XXX this should be a single transaction...
        count = int.from_bytes(self.i2c.read(self.addr, 1, repeat=True))
        return self.i2c.read(self.addr, count)

    def read_i2c_block_data(self, cmd, count=32):
        self.i2c.write(self.addr, cmd.to_bytes(1) + count.to_bytes(1),
                       repeat=True)
        return self.i2c.read(self.addr, count)

    def write_i2c_block_data(self, cmd, vals):
        self.i2c.write(self.addr, cmd.to_bytes(1) + vals)
