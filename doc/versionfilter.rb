#!/usr/bin/env ruby

require 'pathname'

Dir.chdir '..' do
  file = if ARGV.empty?
           "."
         else
           pathname = Pathname.new(ARGV[0]).realpath
           pathname.relative_path_from(Pathname.getwd).to_s
         end
  info = `git whatchanged -n1 --pretty=oneline -- "#{file}"`
  info = info.split(' ').first
  puts info
end
