Gem::Specification.new do |s|
  s.name = 'gas'
  s.date = Date.today
  s.version = File.read(File.dirname(__FILE__) + "/VERSION").chomp
  s.summary = 'extensible binary meta-container'
  s.homepage = 'http://gas.gekkoware.net/'
  s.description = 'extensible binary meta-container'
  s.has_rdoc = true
  s.authors = ['Blanton Black']
  s.files = %w{LICENSE NOTICE README lib/gas.rb}
end
