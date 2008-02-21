
require 'dl'
require 'dl/import'
require 'pp'

module Gas

  CHUNK_STRUCT = 'PLLPLPLPLP'
  ATTRIBUTE_STRUCT = 'LPLP'

  class GasError < RuntimeError
  end
  class AttributeNotFoundError < GasError
  end

  LIB = case RUBY_PLATFORM
        when /linux/
          DL::dlopen('libgas.so')
        when /mswin/
          DL::dlopen('libgas.dll')
        else
          raise GasError, 'unsupported platform'
        end

  def gas_call (target, *args)
    ret, rs = target.call(*args)
    return ret
  end

  function_map = %w/
    gas_read_buf PSLP
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
    gas_new PPL
    gas_new_named PS
    gas_set_id 0PPL
    gas_read_fd PL
    gas_print 0P
    gas_id_size LP
    gas_get_id LPPL
    gas_get_parent PP
    gas_nb_children LP
    gas_get_child_at PPL
    gas_add_child 0PP
    gas_set_attribute 0PPLPL
    gas_get_attribute LPLPL
    gas_set_id 0PPL
    gas_set_payload 0PPL
    gas_index_of_attribute LPPL
    gas_attribute_value_size LPL
    gas_payload_size LP
    gas_get_payload LPPL
    gas_update 0P
    gas_write_buf LPP
    gas_total_size LP
    gas_delete_attribute_at LPL
    gas_delete_child_at LPL
    /
    function_map = Hash[*function_map]
    function_map.each do |func, sig|
      self.const_set func.upcase, LIB[func, sig]
    end

    def describe_chunk_struct (ptr_data)
      ptr_data.struct!(
        CHUNK_STRUCT,
        :parent, :size,
        :id_size, :id,
        :nb_attributes, :attributes,
        :payload_size, :payload,
        :nb_children, :children
      )
#      class << ptr_data
#        def struct_size
#          DL.sizeof(CHUNK_STRUCT)
#        end
#      end
    end
    def describe_attribute_chunk (ptr_data)
      ptr_data.struct!(
        ATTRIBUTE_STRUCT,
        :key_size, :key,
        :value_size, :value
      )
#      class << ptr_data
#        def struct_size
#          DL.sizeof(ATTRIBUTE_STRUCT)
#        end
#      end
    end

    def initialize (arg = nil)
      case arg
      when nil
        @c_obj = gas_call(GAS_NEW, nil, 0)
      when DL::PtrData
        @c_obj = arg
      when String
        @c_obj = gas_call(GAS_NEW_NAMED, arg)
      when IO
        @c_obj = gas_call(GAS_READ_FD, arg.fileno)
        if @c_obj.nil?
          raise GasError, 'error reading from fd, io is probably empty'
        end
      when Integer
        @c_obj = gas_call(GAS_READ_FD, arg)
      when Hash
        @c_obj = gas_call(GAS_NEW, nil, 0)
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
        @c_obj = gas_call(GAS_NEW, 0, nil)
      else
        raise GasError, "invalid arg type: #{arg.type}"
      end

      describe_chunk_struct(@c_obj)

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
          raise GasError, 'destroy called on chunk with a valid parent'
        end
        @c_obj.free = nil
        gas_call(GAS_DESTROY, @c_obj)
        @c_obj = nil
      end
      nil
    end
    def update
      gas_call(GAS_UPDATE, @c_obj)
      self
    end
    def parent
      #o = gas_call(GAS_GET_PARENT, @c_obj)
      o = @c_obj[:parent]
      return o.nil? ? nil : Chunk.new(o)
    end
    def print
      gas_call(GAS_PRINT, @c_obj)
      self
    end
    def size
      return @c_obj[:size]
    end
    def total_size
      return gas_call(GAS_TOTAL_SIZE, @c_obj)
    end

    def id_size
      #return gas_call(GAS_ID_SIZE, @c_obj)
      return @c_obj[:id_size]
    end
    def id
      id = @c_obj[:id]
      id.size = @c_obj[:id_size]
      return id.to_s
    end
    def set_id (id)
      sid = (Fixnum === id and (0..255) === id) ? id.chr : id.to_s
      gas_call(GAS_SET_ID, @c_obj, sid, sid.size)
      self
    end
    def id= (str)
      return set_id(str)
    end

    def index_of_attribute (key)
      skey = (Fixnum === key and (0..255) === key) ? key.chr : key.to_s
      retval = gas_call(GAS_INDEX_OF_ATTRIBUTE, @c_obj, skey, skey.size)
      return retval
    end
    def has_attribute (key)
      return index_of_attribute(key) >= 0
    end
    def attribute_value_size (index)
      return gas_call(GAS_ATTRIBUTE_VALUE_SIZE, @c_obj, index)
    end
    def get_attribute (key)
      skey = (Fixnum === key and (0..255) === key) ? key.chr : key.to_s
      index = index_of_attribute(skey)
      if index < 0
        raise AttributeNotFoundError, "attribute \"#{skey}\" not found"
      end
      value_size = attribute_value_size(index)

      buf = DL.malloc(value_size)
      bytes_read = gas_call(GAS_GET_ATTRIBUTE, @c_obj, index, buf, value_size)

      raise GasError, 'error getting attribute' unless bytes_read == value_size
      return buf.to_str
    end
    def set_attribute (key, val)
      skey = (Fixnum === key and (0..255) === key) ? key.chr : key.to_s
      sval = (Fixnum === val and (0..255) === val) ? val.chr : val.to_s
      gas_call(GAS_SET_ATTRIBUTE, @c_obj, skey, skey.size, sval, sval.size)
      self
    end
    def nb_attributes
      @c_obj[:nb_attributes]
    end
    def each_attribute
      nb_attributes.times do |i|
        a = @c_obj[:attributes] + (i * DL.sizeof(ATTRIBUTE_STRUCT))
        describe_attribute_chunk(a)

        key = a[:key]
        key.size = a[:key_size]
        value = a[:value]
        value.size = a[:value_size]

        yield(key.to_str, value.to_str)
      end
    end
    def delete_attribute_at (index)
      status = gas_call(GAS_DELETE_ATTRIBUTE_AT, @c_obj, index)
      raise GasError, 'invalid parameter' if status != 0
    end
    # NOTE: read only
    def attributes
      @attributes = Hash.new
      each_attribute do |key, value|
        @attributes[key] = value
      end
      @attributes.freeze
      return @attributes
    end

    def payload_size
      #return gas_call(GAS_PAYLOAD_SIZE, @c_obj)
      return @c_obj[:payload_size]
    end
    def payload
      payload = @c_obj[:payload]
      payload.size = @c_obj[:payload_size]
      return payload.to_str
    end
    def set_payload (data)
      sdata = (Fixnum === data and (0..255) === data) ? data.chr : data.to_s
      gas_call(GAS_SET_PAYLOAD, @c_obj, sdata, sdata.size)
      self
    end
    def payload= (data)
      return set_payload(data)
    end

    def nb_children
      #return gas_call(GAS_NB_CHILDREN, @c_obj)
      return @c_obj[:nb_children]
    end
    def child_at (index)
      o = gas_call(GAS_GET_CHILD_AT, @c_obj, index)
      #o = (@c_obj[:children] + (index * DL.sizeof(CHUNK_STRUCT))).ptr
      return Chunk.new(o)
    end
    def each_child
      nb_children.times do |i|
        yield child_at(i)
      end
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
    def delete_child_at (index)
      status = gas_call(GAS_DELETE_CHILD_AT, @c_obj, index)
      raise GasError, 'invalid parameter' if status != 0
    end
    # NOTE: read only
    def children
      a = Array.new
      nb_children.times do |i|
        a << child_at(i)
      end
      a.freeze
      return a
    end

    # TODO this should not crash when the attribute is not found
    def [] (key)
      return get_attribute(key)
    end
    def []= (key, val)
      return set_attribute(key, val)
    end

    def serialize
      # TODO automatically update or not?
      update
      buf = DL.malloc(total_size)
      offset = gas_call(GAS_WRITE_BUF, @c_obj, buf)
      raise GasError, "gas_write_buf size != offset" unless total_size == offset
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
        raise GasError, 'invalid arg count' unless args.size == 1
        set_attribute(key, args.first)
      else
        # exception will propagate
        return get_attribute(meth)
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
        raise GasError, 'invalid arg type'
      end
    end


  end # class Chunk
end # module Gas
