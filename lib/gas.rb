
require 'dl'
require 'dl/import'
require 'pp'

module Gas
  LIB = case RUBY_PLATFORM
        when /linux/
          DL::dlopen('libgas.so')
        when /mswin/
          DL::dlopen('libgas.dll')
        else
          fail 'unsupported platform'
        end

  def gas_call (target, *args)
    return target.call(*args)
    #puts "status=#{status.inspect} - rs=#{rs.inspect}"
    #return status
    #return rs
  end

  function_map = %w/
    gas_read_buf PSIP
    /
  function_map = Hash[*function_map]
  function_map.each do |func, sig|
    self.const_set func.upcase, LIB[func, sig]
  end
  def self.parse_buffer (buf)
    o = GAS_READ_BUF.call(buf, buf.size, nil).first
    return Chunk.new(o)
  end

  class Chunk
    include Gas
    function_map = %w/
    gas_new_named PS
    gas_destroy 0P
    gas_read_fd PI
    gas_print 0P
    gas_id_size IP
    gas_get_id IPPII
    gas_nb_children IP
    gas_get_child_at PPI
    gas_add_child 0PP
    gas_set_attribute 0PIPIP
    gas_get_attribute IPIPII
    gas_set_id 0PIP
    gas_set_payload 0PIP
    gas_index_of_attribute IPIP
    gas_attribute_value_size IPI
    gas_payload_size IP
    gas_get_payload IPPII
    gas_update 0P
    gas_write_buf IPP
    gas_total_size IP
    /
    function_map = Hash[*function_map]
    function_map.each do |func, sig|
      self.const_set func.upcase, LIB[func, sig]
    end

    def initialize (arg)
      case arg
      when DL::PtrData
        @c_obj = arg
      when String
        @c_obj = gas_call(GAS_NEW_NAMED, arg).first
      when Integer
        @c_obj = gas_call(GAS_READ_FD, arg).first
      else
        fail 'invalid arg type'
      end

      if block_given?
        yield self
      end
    end
    def destroy
      gas_call(GAS_DESTROY, @c_obj)
      nil
    end
    def print
      gas_call(GAS_PRINT, @c_obj)
      self
    end
    def id_size
      return gas_call(GAS_ID_SIZE, @c_obj).first
    end
    def id
      buf = DL.malloc(id_size)
      bytes_left = gas_call(GAS_GET_ID, @c_obj, buf, 0, id_size).first
      fail unless bytes_left.zero?
      return buf.to_str
    end
    def id= (str)
      gas_call(GAS_SET_ID, @c_obj, str.size, str)
      self
    end
    def nb_children
      return gas_call(GAS_NB_CHILDREN, @c_obj).first
    end
    def child_at (index)
      return Chunk.new(gas_call(GAS_GET_CHILD_AT, @c_obj, index).first)
    end
    def each_child
      nb_children.times do |i|
        yield child_at(i)
      end
    end
    def children
      a = Array.new
      nb_children.times do |i|
        a << child_at(i)
      end
      return a
    end
    def add_child (child)
      gas_call(GAS_ADD_CHILD, @c_obj, child.instance_variable_get(:@c_obj))
      self
    end
    def add_children (child_array)
      child_array.each do |child|
        add_child(child)
      end
      self
    end
    def index_of_attribute (key)
      return gas_call(GAS_INDEX_OF_ATTRIBUTE, @c_obj, key.to_s.size, key.to_s).first
    end
    def has_attribute (key)
      return index_of_attribute(key) >= 0
    end
    def attribute_value_size (index)
      return gas_call(GAS_ATTRIBUTE_VALUE_SIZE, @c_obj, index).first
    end
    def get_attribute (key)
      index = index_of_attribute(key.to_s)
      value_size = attribute_value_size(index)

      buf = DL.malloc(value_size)
      bytes_left = gas_call(GAS_GET_ATTRIBUTE, @c_obj, index, buf, 0, value_size).first
      fail unless bytes_left.zero?
      return buf.to_str
    end
    # TODO this should not crash when the attribute is not found
    def [] (key)
      return get_attribute(key)
    end
    def set_attribute (key, val)
      gas_call(GAS_SET_ATTRIBUTE, @c_obj, key.size, key, val.size, val)
      self
    end
    def []= (key, val)
      return set_attribute(key.to_s, val.to_s)
    end
#    def attributes
#    end
    def payload_size
      return gas_call(GAS_PAYLOAD_SIZE, @c_obj).first
    end
    def payload
      buf = DL.malloc(payload_size)
      bytes_left = gas_call(GAS_GET_PAYLOAD, @c_obj, buf, 0, payload_size).first
      fail unless bytes_left.zero?
      return buf.to_str
    end
    def payload= (data)
      gas_call(GAS_SET_PAYLOAD, @c_obj, data.size, data)
      self
    end
    def update
      gas_call(GAS_UPDATE, @c_obj)
      self
    end
    def total_size
      return gas_call(GAS_TOTAL_SIZE, @c_obj).first
    end
    def serialize
      # TODO automatically update or not?
      update
      buf = DL.malloc(total_size)
      offset = gas_call(GAS_WRITE_BUF, @c_obj, buf).first
      fail "gas_write_buf size != offset" unless total_size == offset
      return buf.to_str
    end
    def write (io)
      buf = serialize
      io.write buf
      # TODO what about the allocated data
    end
    def method_missing (meth, *args)
      case meth.to_s
      when /=\Z/
        key = meth.to_s[0..-2]
        fail 'invalid arg count' unless args.size == 1
        set_attribute(key, args.first)
      else
        if has_attribute(meth)
          return get_attribute(meth)
        else
          fail "attribute '#{meth}' not found"
        end
      end
    end
  end
end
