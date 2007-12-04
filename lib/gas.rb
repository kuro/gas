
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
    gas_destroy 0P
    /
  function_map = Hash[*function_map]
  function_map.each do |func, sig|
    self.const_set func.upcase, LIB[func, sig]
  end
  def self.parse_buffer (buf)
    ptr = GAS_READ_BUF.call(buf, buf.size, nil).first
    # ptr is the root, initialize does not set destroy for pointers, so set here
    ptr.free = GAS_DESTROY
    return Chunk.new(ptr)
  end

  class Chunk
    include Gas
    function_map = %w/
    gas_new PIP
    gas_new_named PS
    gas_set_id 0PIP
    gas_read_fd PI
    gas_print 0P
    gas_id_size IP
    gas_get_id IPPII
    gas_get_parent PP
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

    def initialize (arg = nil)
      case arg
      when DL::PtrData
        @c_obj = arg
      when String
        @c_obj = gas_call(GAS_NEW_NAMED, arg).first
      when Integer
        @c_obj = gas_call(GAS_READ_FD, arg).first
      when Hash
        @c_obj = gas_call(GAS_NEW, 0, nil).first
        arg.each do |key, val|
          skey = (Fixnum === key and (0..255) === key) ? key.chr : key.to_s
          sval = (Fixnum === val and (0..255) === val) ? val.chr : val.to_s
          case skey
          when 'id'
            set_id sval
          when 'payload'
            set_payload sval
          else
            self[skey] = sval
          end
        end
      when nil
        @c_obj = gas_call(GAS_NEW, 0, nil).first
      else
        fail 'invalid arg type'
      end

      # pointers are created internally, and do not need to be destroyed, i
      # think
      @c_obj.free = GAS_DESTROY unless DL::PtrData === arg

      if block_given?
        yield self
      end
    end
    # do not call the manual destory on elements belonging to a tree other than
    # the root node
    def destroy
      unless @c_obj.nil?
        if not parent.nil?
          @c_obj.free = nil
          fail 'destroy called on chunk with a valid parent'
        end
        @c_obj.free = nil
        gas_call(GAS_DESTROY, @c_obj)
        @c_obj = nil
      end
      nil
    end
    def parent
      o = gas_call(GAS_GET_PARENT, @c_obj).first
      return o.nil? ? nil : Chunk.new(o)
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
    def set_id (id)
      sid = (Fixnum === id and (0..255) === id) ? id.chr : id.to_s
      gas_call(GAS_SET_ID, @c_obj, sid.size, sid)
      self
    end
    def id= (str)
      return set_id(str)
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
      other = child.instance_variable_get(:@c_obj)
      gas_call(GAS_ADD_CHILD, @c_obj, other)
      other.free = nil
      self
    end
    def add_children (child_array)
      child_array.each do |child|
        add_child(child)
      end
      self
    end
    def index_of_attribute (key)
      skey = (Fixnum === key and (0..255) === key) ? key.chr : key.to_s
      return gas_call(GAS_INDEX_OF_ATTRIBUTE, @c_obj, skey.size, skey).first
    end
    def has_attribute (key)
      return index_of_attribute(key) >= 0
    end
    def attribute_value_size (index)
      return gas_call(GAS_ATTRIBUTE_VALUE_SIZE, @c_obj, index).first
    end
    def get_attribute (key)
      skey = (Fixnum === key and (0..255) === key) ? key.chr : key.to_s
      index = index_of_attribute(skey)
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
      skey = (Fixnum === key and (0..255) === key) ? key.chr : key.to_s
      sval = (Fixnum === val and (0..255) === val) ? val.chr : val.to_s
      gas_call(GAS_SET_ATTRIBUTE, @c_obj, skey.size, skey, sval.size, sval)
      self
    end
    def []= (key, val)
      return set_attribute(key, val)
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
    def set_payload (data)
      sdata = (Fixnum === data and (0..255) === data) ? data.chr : data.to_s
      gas_call(GAS_SET_PAYLOAD, @c_obj, sdata.size, sdata)
      self
    end
    def payload= (data)
      return set_payload(data)
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
    def << (arg)
      case arg
      when Array
        add_children arg
        return self
      when Chunk
        add_child(arg)
        #return arg
        return self
      else
        fail 'invalid arg type'
      end
    end
  end
end
