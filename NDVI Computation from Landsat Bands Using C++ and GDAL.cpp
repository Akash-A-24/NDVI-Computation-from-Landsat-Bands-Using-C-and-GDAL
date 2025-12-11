#include <iostream>
#include <gdal_priv.h>
#include <cpl_conv.h> // for CPLMalloc
using namespace std;
int main() {
GDALAllRegister();
const char* redbandpath = "LC08_B4.tif"; // landsat Red band (Band 4)
const char* nirbandpath = "LC08_B5.tif"; // Landsat NIR BAND(Band 5)
GDALDataset* redDataset = (GDALDataset*)GDALOpen(redbandpath, GA_ReadOnly);
GDALDataset* nirDataset = (GDALDataset*)GDALOpen(nirbandpath, GA_ReadOnly);

if(redDataset == NULL || nirDataset == NULL) {
    cerr << "error: could not open input files."  << endl;
    return 1;
}

GDALRasterBand* redBand = redDataset->GetRasterBand(1);
GDALRasterBand* nirBand = nirDataset->GetRasterBand(1);

int width = 100;
int height = 100;

float* redbuffer = (float*)CPLMalloc(sizeof(float) * width * height);
float* nirbuffer = (float*)CPLMalloc(sizeof(float) * width * height);
float* ndvibuffer = (float*)CPLMalloc(sizeof(float) * width * height);

// Read 100*100 pixels from top-left corner(offset 0,0)
redBand->RasterIO(GF_Read, 0, 0, width, height, redbuffer, width, height, GDT_Float32, 0, 0);
nirBand->RasterIO(GF_Read, 0, 0, width, height, nirbuffer, width, height, GDT_Float32, 0, 0);

// compute ndvi = (nir - red) / (nir + red)
for (int i = 0; i<width*height; i++){
    float nir = nirbuffer[i];
     float red = redbuffer[i];
     float denominator = nir + red;
     if (denominator == 0){
        ndvibuffer[i] = 0; // avoid divide by zero
     } else {
     float ndvi = (nir - red )/denominator;
     if (ndvi > 1.0f) ndvi = 1.0f;
     if (ndvi > 1.0f) ndvi = 1.0f;
     ndvibuffer[i] = ndvi;
     }
}
// create output NDVI geotiff
const char* outputPath = "ndvi_200x200. tif";
GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");

if (driver == NULL) {
    cerr << "error : could not get GTiff driver." << endl;
    return 1;
}
GDALDataset* ndviDataset = driver->Create(outputPath, width, height, 1, GDT_Float32, NULL);
ndviDataset->GetRasterBand(1)->RasterIO(GF_Write, 0, 0, width, height, ndvibuffer, width, height, GDT_Float32, 0, 0);

// copy georeferencing info from redband
double geoTransform[6];
if(redDataset->GetGeoTransform(geoTransform) == CE_None){
    ndviDataset->SetGeoTransform(geoTransform);
}
 ndviDataset->SetProjection(redDataset->GetProjectionRef());
 cout << "ndvi calculated and save as ndvi_200x200.tif" << endl;

//cleanup
GDALClose(redDataset);
GDALClose(nirDataset);
GDALClose(ndviDataset);

CPLFree(redbuffer);
CPLFree(nirbuffer);
CPLFree(ndvibuffer);
return 0;
}
