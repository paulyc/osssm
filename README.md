# osssm
osssm stuff for OSM. Started off as a README, turned into an attempt at GIS-themed poetry *ahem* :
---
Currently you have to build every project separately.

This mainly started as a way to support the handy renderosm program though.

That should in theory work if you have mapnik 4 and required dependencies installed.

And configure the info in the openstreetmap-carto project.xml carto style file.

Which is harder than it sounds even using distro package managers.

---

Wherefore, those dependencies include but are indeed not limited to,

openstreetmap-carto, carto, transpiled XML style file, land boundary shapefiles, 

And, the real kicker: a PostGIS DB with OSM data.

Which depends on PostgresQL, PostGIS, osm2pgsql, and a raw data dump from the OSM, 

Plus your very precious CPU time and RAM.

---

And wherefore, if the version of any one of these is incompatible with that of any other,

Or you've got an old version of osm2pgsql with extremely serious memory leaks,

That make it impossible to import the planet with less than 32 GB of RAM, 

(At least, I don't even know because I had to upgrade osm2pgsql to import a whole planet at all),

The entire asset flow breaks,

And you have nothing.

---

But when the asset flow works,

You have a whole planet at your fingertips,

Without doing any more work.

---

At least in theory.

And notwithstanding your CPU.
