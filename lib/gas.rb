
require 'dl'
require 'dl/import'

module Gas
  LIB = case RUBY_PLATFORM
        when /linux/
          DL::dlopen('libgas.so')
        when /mswin/
          #ENV['PATH'] = "#{ENV['PATH']};src"
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
    gas_get_id_s SP
    gas_set_id_s SPS
    gas_nb_children IP
    gas_get_child_at PPI
    gas_get_attribute_ss SPS
    gas_get_payload_s SP
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
    def id
      return gas_call(GAS_GET_ID_S, @c_obj).first
    end
    def id= (str)
      gas_call(GAS_SET_ID_S, @c_obj, str)
      self
    end
    def nb_children
      return gas_call(GAS_NB_CHILDREN, @c_obj).first
    end
    def child_at (index)
      return Chunk.new(gas_call(GAS_GET_CHILD_AT, @c_obj, index).first)
    end
    def get_attribute (key)
      return gas_call(GAS_GET_ATTRIBUTE_SS, @c_obj, key).first
    end
    def payload
      return gas_call(GAS_GET_PAYLOAD_AS_STRING, @c_obj).first
    end
  end
end
