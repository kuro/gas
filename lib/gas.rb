
module Gas

  def encode_num (value)
    buf = String.new

    coded_length = 1
    loop do
      break if value < ((1 << (7*coded_length))-1)
      #break if ((coded_length * 7) > (4 * 8))
      coded_length += 1
    end

    zero_count = coded_length - 1
    zero_bytes = zero_count / 8
    zero_bits = zero_count % 8

    zero_bytes.times do
      buf << "\0"
    end

    mask = 0x80
    mask >>= zero_bits

    # first masked byte
  #  byte = if coded_length <= 4
  #           mask | ((value >> ((coded_length - zero_bytes - 1) * 8)) & 0xff)
  #         else
  #           mask
  #         end
    byte = mask | ((value >> ((coded_length - zero_bytes - 1) * 8)) & 0xff)
    buf << byte

    # remaining bytes
    si = coded_length - 2 - zero_bytes
    while si >= 0
      byte = ((value >> (si * 8)) & 0xff)
      buf << byte
      si -= 1
    end

    return buf
  end

  def decode_num (io)
    byte = nil

    zero_byte_count = 0
    loop do
      byte = io.readchar
      break if byte != 0x00
      zero_byte_count += 1
    end

    first_bit_set = nil
    7.downto(0) do |first_bit_set|
      break unless byte[first_bit_set].zero?
    end

    mask = 0x0
    0.upto(first_bit_set - 1) do |i|
      mask |= (1 << i)
    end

    additional_bytes_to_read = (7 - first_bit_set) + (7 * zero_byte_count)

    retval = mask & byte
    additional_bytes_to_read.times do
      byte = io.readchar
      retval = (retval << 8) | byte
    end
    return retval
  end

  class Chunk
    include Gas

    attr_accessor :parent, :size, :id, :attributes, :payload, :children

    def initialize (arg = nil)
      @parent = nil
      @size = 0
      @id = String.new
      @attributes = Hash.new
      @payload = String.new
      @children = Array.new

      case arg
      when nil
        # do nothing
      when Hash
        arg.each do |key, val|
          skey = (Fixnum === key and (0..255) === key) ? key.chr : key.to_s
          sval = (Fixnum === val and (0..255) === val) ? val.chr : val.to_s
          case skey
          when 'id'
            @id = sval
          when 'payload'
            @payload = sval
          else
            @attributes[skey] = sval
          end
        end
      when IO
        read(arg)
      else
        fail 'invalid type'
      end
    end
    def read (io)
      @size = decode_num(io)
      id_size = decode_num(io)
      @id = io.read(id_size)
      nb_attributes = decode_num(io)
      @attributes = Hash.new
      nb_attributes.times do
        key_size = decode_num(io)
        key = io.read(key_size)
        value_size = decode_num(io)
        value = io.read(value_size)
        @attributes[key] = value
      end
      payload_size = decode_num(io)
      @payload = io.read(payload_size)
      nb_children = decode_num(io)
      @children = Array.new
      nb_children.times do
        @children << Chunk.new(io)
        @children.last.parent = self
      end
    end
    def write (io)
      io << encode_num(@size)
      io << encode_num(id.size)
      io << id
      io << encode_num(@attributes.size)
      @attributes.each do |key, value|
        io << encode_num(key.size)
        io << key
        io << encode_num(value.size)
        io << value
      end
      io << encode_num(@payload.size)
      io << @payload
      io << encode_num(@children.size)
      @children.each do |child|
        child.write(io)
      end
      io
    end
    def total_size
      encode_num(@size).size + @size
    end
    def update
      @size = 0
      @size += encode_num(@id.size).size
      @size += @id.size
      @size += encode_num(@attributes.size).size
      @attributes.each do |key, value|
        @size += encode_num(key.size).size
        @size += key.size
        @size += encode_num(value.size).size
        @size += value.size
      end
      @size += encode_num(@payload.size).size
      @size += @payload.size
      @size += encode_num(@children.size).size
      @children.each do |child|
        child.update
        @size += encode_num(child.size).size
        @size += child.size
      end
    end
    def << (chunk)
      @children << chunk
    end
    def [] (key)
      skey = (Fixnum === key and (0..255) === key) ? key.chr : key.to_s

      case skey
      when 'id'
        id
      when 'payload'
        payload
      else
        @attributes[skey]
      end
    end
    def []= (key, val)
      skey = (Fixnum === key and (0..255) === key) ? key.chr : key.to_s
      sval = (Fixnum === val and (0..255) === val) ? val.chr : val.to_s
      case skey
      when 'id'
        id = sval
      when 'payload'
        payload = sval
      else
        @attributes[skey] = sval
      end
    end
    def method_missing (meth, *args)
      case meth.to_s
      when /=\Z/
        key = meth.to_s[0..-2]
        raise GasError, 'invalid arg count' unless args.size == 1
        self[key] = args.first
      else
        return self[meth]
      end
      self
    end

  end # class Chunk

end # module Gas

class Fixnum
  include Gas
  def to_gas
    encode_num(self)
  end
end
