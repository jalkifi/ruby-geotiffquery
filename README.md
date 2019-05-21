# Ruby-geotiffquery

Query geotiff files in ruby.

## Usage

First, load the library:

```ruby
require "ruby_geotiffquery"
```

Get data from GeoTIFF file like this:

```ruby
RubyGeotiffquery.rastervalue("M4213.tif", 23.8575, 61.4322)
# => 115.21099853515625
```

In this particular example return data is the elevation in meters of requested location,

If requested coordinates are out of range of given GeoTIFF file, an exception is thrown:

```ruby
RubyGeotiffquery.rastervalue("M4213.tif", 23.8318, 61.4232)
ERROR 5: M4213.tif, band 1: Access window out of range in RasterIO().  Requested
(-103,796) of size 1x1 on raster of 2400x1200.
```
