
require 'dl'
require 'dl/import'

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
    gas_set_attribute 0PIPIP
    gas_get_attribute IPIPII
    gas_set_id 0PIP
    gas_set_payload 0PIP
    gas_index_of_attribute IPIP
    gas_attribute_value_size IPI
    gas_payload_size IP
    gas_get_payload IPPII
    gas_update 0P
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
    def index_of_attribute (key)
      return gas_call(GAS_INDEX_OF_ATTRIBUTE, @c_obj, key.size, key).first
    end
    def attribute_value_size (index)
      return gas_call(GAS_ATTRIBUTE_VALUE_SIZE, @c_obj, index).first
    end
    def get_attribute (key)
      index = index_of_attribute(key)
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
      return set_attribute(key, val)
    end
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
  end
end
