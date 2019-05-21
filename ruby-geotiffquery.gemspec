require File.expand_path("../lib/ruby_geotiffquery/version", __FILE__)
require "date"

Gem::Specification.new do |s|
	s.name = 'ruby-geotiffquery'
	s.version = RubyGeotiffquery::VERSION
	s.date = Date.today
	s.summary = 'Query geotiff files with in ruby.'
	s.authors = ['Mika Haulo']
	s.email = ['mika@haulo.fi']
	s.licenses = ['MIT']
	s.homepage = 'https://www.github.com/mhaulo/ruby-geotiffquery'
	s.extensions = ['ext/ruby_geotiffquery/extconf.rb']
	s.files = [
		'ext/ruby_geotiffquery/ruby_geotiffquery.c',
		'lib/ruby_geotiffquery.rb',
		'lib/ruby_geotiffquery/version.rb'
	]
	s.require_paths = ['lib']
end
