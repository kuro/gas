#
# Copyright 2008 Blanton Black
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

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
      byte = io.read(1)
      fail 'failed to read 1 byte' unless byte
      byte = byte[0]
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
      byte = io.read(1)
      fail 'failed to read 1 byte' unless byte
      byte = byte[0]
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
      when IO, StringIO
        parse(arg)
      else
        fail "invalid type: #{arg.class}"
      end
    end
    def parse (io)
      if String === io
        return parse(StringIO.new(io))
      end

      @size = decode_num(io)
      id_size = decode_num(io)
      @id = io.read(id_size)
      unless @id and @id.size == id_size
        fail "failed to read #{'0x%x' % id_size} bytes"
      end
      nb_attributes = decode_num(io)
      @attributes = Hash.new
      nb_attributes.times do
        key_size = decode_num(io)
        key = io.read(key_size)
        unless key and key.size == key_size
          fail "failed to read #{'0x%x' % key_size} bytes" 
        end
        value_size = decode_num(io)
        value = io.read(value_size)
        unless value and value.size == value_size
          fail "failed to read #{'0x%x' % value_size} bytes"
        end
        @attributes[key] = value
      end
      payload_size = decode_num(io)
      @payload = io.read(payload_size)
      unless @payload and @payload.size == payload_size
        fail "failed to read #{'0x%x' % payload_size} bytes"
      end
      nb_children = decode_num(io)
      @children = Array.new
      nb_children.times do
        @children << Chunk.new(io)
        @children.last.parent = self
      end
      self
    end
    def write (io)
      io << encode_num(@size)
      io << encode_num(id.size)
      io << id
      io << encode_num(@attributes.size)
      #@attributes.each do |key, value|
      @attributes.keys.sort.each do |key|
        value = @attributes[key]
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
      self
    end
    def serialize
      sio = StringIO.new
      write sio
      sio.rewind
      return sio.read
    end
    def add_child (chunk)
      chunk.parent = self
      @children << chunk
      self
    end
    def << (chunk)
      case chunk
      when Array
        chunk.each do |x|
          x.parent = self
          @children << x
        end
      when Chunk
        chunk.parent = self
        @children << chunk
      else
        raise GasError, 'invalid type for appending'
      end
      self
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

    def test (depth = 0)
      pi = proc do |data|
        print('    ' * depth)
        puts data
      end
      pi.call '---'
      pi.call "id: #{id}"
      counter = 0
      #@attributes.each do |key, value|
      @attributes.keys.sort.each do |key|
        value = @attributes[key]
        pi.call "#{counter}: #{key.inspect} => #{value.inspect}"
        counter += 1
      end
      tmp = payload#.strip
      unless tmp.empty?
        lines = tmp.split("\n")
        #lines = lines.map {|l|l.strip}
        m = lines.collect{|l|l.size}.max
        m = m > 78 ? 78 : m
        puts "+" << ('-' * m)
        lines.each do |line|
          puts "| " << line
          #puts "| " << line.inspect[1..-2]
        end
        puts "+" << ('-' * m)
      end
      @children.each do |child|
        child.test(depth+1)
      end
    end

  end # class Chunk

end # module Gas

class Fixnum
  include Gas
  def to_gas
    encode_num(self)
  end
end
