#include <stdio.h>
#include <ruby.h>
#include <gdal/gdal.h>
#include <gdal/ogr_srs_api.h>
#include <gdal/cpl_string.h>

char* ruby_geotiffquery_SanitizeSRS(const char *pszUserInput) {
	OGRSpatialReferenceH hSRS;
	char *pszResult = NULL;

	CPLErrorReset();

	hSRS = OSRNewSpatialReference(NULL);
	
	if (OSRSetFromUserInput(hSRS, pszUserInput) == OGRERR_NONE) {
		OSRExportToWkt(hSRS, &pszResult);
	}
	else {
		CPLError(CE_Failure, CPLE_AppDefined,
		    "Translating source or target SRS failed:\n%s", pszUserInput);
		pszResult = NULL;
	}

	OSRDestroySpatialReference(hSRS);

	return pszResult;
}

double ruby_geotiffquery_fetch_info(double x, double y, const char *filename, OGRCoordinateTransformationH hCT) {
	// Open source file
	GDALDatasetH hSrcDS = GDALOpen(filename, GA_ReadOnly);
	if (hSrcDS == NULL) {
		rb_raise(rb_eRuntimeError, "Unable to open dataset");
	}
	
	// Next: Turn the location into a pixel and line location.
	
	double dfGeoX = x;
	double dfGeoY = y;
	
	if (hCT) {
		if (!OCTTransform(hCT, 1, &dfGeoX, &dfGeoY, NULL)) {
			rb_raise(rb_eRuntimeError, "Invalid dataset");
		}
	}

	double adfGeoTransform[6];
	double adfInvGeoTransform[6];

	if (GDALGetGeoTransform(hSrcDS, adfGeoTransform) != CE_None) {
		rb_raise(rb_eRuntimeError, "Invalid dataset");
	}

	int unused_return_value = GDALInvGeoTransform(adfGeoTransform, adfInvGeoTransform);

	int iPixel = (int) floor(
		adfInvGeoTransform[0]
		+ adfInvGeoTransform[1] * dfGeoX
		+ adfInvGeoTransform[2] * dfGeoY);
	int iLine = (int) floor(
		adfInvGeoTransform[3]
		+ adfInvGeoTransform[4] * dfGeoX
		+ adfInvGeoTransform[5] * dfGeoY);

	
	
	// We boldly assume GeoTIFF files we are going to process have only one raster band.
	// Indexing of bands starts from 1.
	int bandId = 1;
	GDALRasterBandH hBand = GDALGetRasterBand(hSrcDS, bandId);

	if (hBand == NULL) {
		rb_raise(rb_eRuntimeError, "Invalid raster band in dataset");
	}
	
	// Next: Report the pixel value of this band.
	
	double adfPixel[2];
	double output;
	
	if (GDALRasterIO(hBand, GF_Read, iPixel, iLine, 1, 1,
		adfPixel, 1, 1, GDT_CFloat64, 0, 0) == CE_None) {
		output = adfPixel[0];
	}
	else {
		rb_raise(rb_eRuntimeError, "Access window out of range in RasterIO()");
	}
	

	// Next: Cleanup and return
	
	GDALClose(hSrcDS);
	
	return output;
}

VALUE ruby_geotiffquery_value(VALUE _self, VALUE geotiff_file, VALUE lon, VALUE lat) {
	Check_Type(geotiff_file, T_STRING);
	Check_Type(lon, T_FLOAT);
	Check_Type(lat, T_FLOAT);
	
	double x = NUM2DBL(lon);
	double y = NUM2DBL(lat);
	char *filename = StringValueCStr(geotiff_file);
	
	GDALAllRegister();
	
	// Next: Setup coordinate transformation, if required
	
	OGRSpatialReferenceH sourceSRS = NULL;
	OGRSpatialReferenceH targetSRS = NULL;
	OGRCoordinateTransformationH hCT = NULL;
	
	char *pszSourceSRS = NULL;
	CPLFree(pszSourceSRS);
	pszSourceSRS = ruby_geotiffquery_SanitizeSRS("WGS84");
	
	if (pszSourceSRS != NULL) {
		sourceSRS = OSRNewSpatialReference(pszSourceSRS);
		const char* projRef = "PROJCS[\"ETRS89 / TM35FIN(E,N)\",GEOGCS[\"ETRS89\",DATUM[\"European_Terrestrial_Reference_System_1989\",SPHEROID[\"GRS 1980\",6378137,298.2572221010002,AUTHORITY[\"EPSG\",\"7019\"]],TOWGS84[0,0,0,0,0,0,0],AUTHORITY[\"EPSG\",\"6258\"]],PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0.0174532925199433],AUTHORITY[\"EPSG\",\"4258\"]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0],PARAMETER[\"central_meridian\",27],PARAMETER[\"scale_factor\",0.9996],PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",0],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AUTHORITY[\"EPSG\",\"3067\"]]";
		targetSRS = OSRNewSpatialReference(projRef);
		hCT = OCTNewCoordinateTransformation(sourceSRS, targetSRS);
		
		if (hCT == NULL) {
			rb_raise(rb_eRuntimeError, "Invalid spatial reference system");
		}
	}
	
	double value = ruby_geotiffquery_fetch_info(x, y, filename, hCT);
	
	// Next: Cleanup
	
	if (hCT) {
		OSRDestroySpatialReference(sourceSRS);
		OSRDestroySpatialReference(targetSRS);
		OCTDestroyCoordinateTransformation(hCT);
	}
	
	GDALDestroyDriverManager();
	CPLFree(pszSourceSRS);
	
	return DBL2NUM(value);
}

void Init_ruby_geotiffquery(void) {
	VALUE mod = rb_define_module("RubyGeotiffquery");
	rb_define_module_function(mod, "rastervalue", ruby_geotiffquery_value, 3);
}
