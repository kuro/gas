#!/usr/bin/env ruby

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
