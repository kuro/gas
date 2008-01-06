#!/usr/bin/env ruby

require 'gas'
include Gas

r = Chunk.new(:id => 'root', :status => 'fun')
foo = Chunk.new(:id=>'foo', :something=>[?a, ?b, ?c].pack('c*'))
bar = Chunk.new('bar')
unnamed = Chunk.new
r.add_child foo
foo << bar << unnamed
foo.parent.print

(v = Chunk.new('vehicle')) << (Chunk.new('car') << [Chunk.new('truck') , Chunk.new('bus')])
#(v = Chunk.new('vehicle')) << [Chunk.new('car') , Chunk.new('truck') << Chunk.new('bus')]
v.print


#open('test/dump.gas') do |io|
  #root = Gas::Chunk.new(io.fileno)
  #root.print
  ##root.destroy
#end
