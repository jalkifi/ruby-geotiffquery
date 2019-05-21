require "mkmf"

$LOCAL_LIBS << '-lgdal'

#abort "missing malloc()" unless have_func "malloc"
#abort "missing free()"   unless have_func "free"

create_makefile "ruby_geotiffquery/ruby_geotiffquery"
