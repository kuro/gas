#!/usr/bin/env ruby

Dir.chdir '..' do
  file = if ARGV.empty?
           "."
         else
           if ARGV[0].include?('.git')
             exit
           else
             ARGV[0].sub(Dir.pwd + '/', '')
           end
         end
  info = `git whatchanged -n1 --pretty=oneline -- "#{file}"`
  info = info.split(' ').first
  puts info
end
