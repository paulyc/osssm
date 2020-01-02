/*
g++ -o renderosm -I/usr/include/mapnik renderosm.cpp -lmapnik -licuuc
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <iostream>

#include <mapnik/projection.hpp>
#include <mapnik/geometry/box2d.hpp>
#include <mapnik/map.hpp>
#include <mapnik/load_map.hpp>
#include <mapnik/agg_renderer.hpp> 
#include <mapnik/image.hpp> 
#include <mapnik/image_util.hpp> 
#include <mapnik/datasource_cache.hpp>
#include <mapnik/font_engine_freetype.hpp>


void usageexit();
void projforw(const mapnik::projection *p, double x, double y, double *u, double *v);
mapnik::box2d<double> *osmurl2box(double lati, double longi, unsigned zoom, unsigned pixwidth, unsigned pixheight, mapnik::projection *proj);


int main(int argc, char **argv)
{
    mapnik::projection *proj;
    mapnik::box2d<double> *box;
    mapnik::Map *map;
    mapnik::image_rgba8 *outimage;
    mapnik::agg_renderer<mapnik::image_rgba8> *renderer; 
    const char *outfilename= "renderosm.png";
    const char *stylesheet= "/data/openstreetmap/styles/git-old-mapnik-style/osm.xml";
    char *parse, *end;
    double aspect;
    unsigned webwidth= 1600, webheight= 1200, outwidth= 0, outheight= 0;
    int argind;

    if( argc == 1 || !strcmp(argv[1], "-h") || !strcmp(argv[1], "--help") )
        usageexit();

    for( argind= 1; argind < argc && argv[argind][0] == '-' && strlen(argv[argind]) == 2; argind += 2 ) {
        if( argind > argc - 2 ) {
            printf("Missing mandatory argument to `%s' option.\n", argv[argind]);
            usageexit();
        }
        if( argv[argind][1] == 'o' )
            outfilename= argv[argind+1];
        else if( argv[argind][1] == 'w' )
            outwidth= strtoul(argv[argind+1], NULL, 10);
        else if( argv[argind][1] == 'h' )
            outheight= strtoul(argv[argind+1], NULL, 10);
        else if( argv[argind][1] == 's' )
            stylesheet= argv[argind+1];
        else {
            printf("Unknown option `%s'.\n", argv[argind]);
            usageexit();
        }
    }
    if( argind != argc-1 ) {
        printf("Need single map region argument - missing or excess arguments present.\n");
        usageexit();
    }

    mapnik::datasource_cache::instance().register_datasources("/usr/lib/mapnik/input/");
    mapnik::freetype_engine::register_fonts("/usr/lib/mapnik/fonts");
    // Create map object with preliminary parameters:
    map= new mapnik::Map(webwidth, webheight, "+init=epsg:3857");
    mapnik::load_map(*map, stylesheet);

    proj= new mapnik::projection(map->srs().c_str());

    if( !strncmp(argv[argind], "http", 4) ) {
        double lati, longi;
        unsigned zoom;

        parse= strstr(argv[argind], "#map=");
        if( !parse ) {
            fprintf(stderr, "Argument `%s' looks like OSM URL, but could not find string `#map'.\n", argv[argind]);
            return 1;
        }
        parse += 5;
        zoom= strtoul(parse, &end, 10);
        if( end == parse ) {
            fprintf(stderr, "Could not parse zoom level in OSM URL `%s'.\n", argv[argind]);
            return 1;
        }
        for( parse= end; *parse == '/'; ++parse );
        lati= strtod(parse, &end);
        if( end == parse ) {
            fprintf(stderr, "Could not parse latitude in OSM URL `%s'.\n", argv[argind]);
            return 1;
        }
        for( parse= end; *parse == '/'; ++parse );
        longi= strtod(parse, &end);
        if( end == parse ) {
            fprintf(stderr, "Could not parse longitude in OSM URL `%s'.\n", argv[argind]);
            return 1;
        }
        box= osmurl2box(lati, longi, zoom, webwidth, webheight, proj);
        aspect= (double)webwidth / webheight;
    }
    else {
        double lat1, long1, lat2, long2, x[4], y[4], x0, y0, x1, y1;
        int ind;

        lat1= strtod(argv[argind], &end);
        if( parse == argv[argind] ) {
            fprintf(stderr, "Could not parse first latitude in `%s'.\n", argv[argind]);
            return 1;
        }
        for( parse= end; isspace(*parse) || *parse == ','; ++parse );
        long1= strtod(parse, &end);
        if( end == parse ) {
            fprintf(stderr, "Could not parse first longitude in `%s'.\n", argv[argind]);
            return 1;
        }
        for( parse= end; isspace(*parse) || *parse == ','; ++parse );
        lat2= strtod(parse, &end);
        if( end == parse ) {
            fprintf(stderr, "Could not parse second latitude in `%s'.\n", argv[argind]);
            return 1;
        }
        for( parse= end; isspace(*parse) || *parse == ','; ++parse );
        long2= strtod(parse, &end);
        if( end == parse ) {
            fprintf(stderr, "Could not parse second longitude in `%s'.\n", argv[argind]);
            return 1;
        }
        // Compute a rectangle in projected coordinates that contain all
        // corners of the given latitude/longitude rectangle:
        projforw(proj, long1, lat1, &x[0], &y[0]);
        projforw(proj, long2, lat1, &x[1], &y[1]);
        projforw(proj, long2, lat2, &x[2], &y[2]);
        projforw(proj, long1, lat2, &x[3], &y[3]);
        x0= x1= x[0];
        y0= y1= y[0];
        for( ind= 1; ind< 4; ++ind ) {
            if( x[ind] < x0 )   x0= x[ind];
            if( y[ind] < y0 )   y0= y[ind];
            if( x[ind] > x1 )   x1= x[ind];
            if( y[ind] > y1 )   y1= y[ind];
        }
        box= new mapnik::box2d<double>(x0, y0, x1, y1);
        aspect= (x1 - x0) / (y1 - y0);
        if( outwidth && outheight && outheight != (unsigned)(outwidth / aspect + 0.5) ) {
            printf("Warning: output aspect ratio does not match aspect of rectangle exactly.  Adapting height.");
            outheight= (unsigned)(outwidth / aspect);
        }
    }

    if( outwidth == 0 && outheight == 0 ) {
        outwidth= 3000;
        outheight= (unsigned)(outwidth / aspect + 0.5);
    }
    else if( outwidth == 0 )
        outwidth= (unsigned)(outheight * aspect + 0.5);
    else if( outheight == 0 )
        outheight= (unsigned)(outwidth / aspect + 0.5);

    map->resize(outwidth, outheight);
    if( map->width() != outwidth || map->height() != outheight ) {
        fprintf(stderr, "**************************************************\n"
                "The intended map image size (%u x %u) could not be set.  Mapnik\n"
                "corrected this to %u x %u.  This may result from exceeding mapnik's\n"
                "arbitrary internal size limit, which cannot be printed here because it is a\n"
                "private constant of the Map class.  However, you can look up Map::MAX_MAPSIZE\n"
                "in /usr/include/mapnik/map.hpp.  If you want to render a map of this size,\n"
                "change MAX_MAPSIZE in map.hpp in the mapnik source and recompile and reinstall\n"
                "mapnik.\n"
                "**************************************************\n",
                outwidth, outheight, map->width(), map->height());
    }
    map->zoom_to_box(*box);
    outimage= new mapnik::image_rgba8(outwidth, outheight);
    renderer= new mapnik::agg_renderer<mapnik::image_rgba8>(*map, *outimage); 
    renderer->apply(); 
    mapnik::save_to_file(*outimage, outfilename);

    delete renderer;
    delete outimage;
    delete map;
    delete box;
    delete proj;
    return 0;
}


void usageexit()
{
    printf("%s",
"Usage:\n"
"renderosm [options] <OpenStreetMap URL>\n"
"renderosm [options] <lat1,long1,lat2,long2>\n"
"\n"
"Options:\n"
"-o <output.png>     Output file\n"
"-w <width>          Width of output image in pixels (default 3000)\n"
"-h <height>         Height of output image in pixels\n"
"-s <style.xml>      XML style file\n"
"\n"
"The zoom level on which the map appearance depends will be computed by the\n"
"Mapnik library from the output image size.  The zoom level from the\n"
"OpenStreetMap URL is used only for determining the map region.  The map\n"
"rectangle represented by an OpenStreetMap URL is assumed to be the equivalent\n"
"of a 1600 by 1200 browser window or the same area with a different aspect ratio\n"
"if both -w and -h are present.\n");
    exit(1);
}


/*
    De-obfuscated forward projection function with an interface that separates
    input and output parameters.
*/
void projforw(const mapnik::projection *p, double x, double y, double *u, double *v)
{
    p->forward(x, y);
    if( u ) *u= x;
    if( v ) *v= y;
}


/*
    Conversion of OSM URL parameters into approximate rectangle expressed by
    mapnik 2D box.
*/
mapnik::box2d<double> *osmurl2box(double lati, double longi, unsigned zoom, unsigned pixwidth, unsigned pixheight, mapnik::projection *proj)
{
    double halfwidth, halfheight, canvassize= 180.0, dummy= 0.0;
    unsigned tilesize= 256;

    proj->forward(canvassize, dummy);
    canvassize *= 2;
    // half window size in projection coordinates:
    halfwidth= canvassize / (1 << zoom) / tilesize * pixwidth / 2;
    halfheight= canvassize / (1 << zoom) / tilesize * pixheight / 2;
    proj->forward(longi, lati);
    return new mapnik::box2d<double>(longi - halfwidth, lati - halfheight, longi + halfwidth, lati + halfheight);
}


