libvmod_maxminddb
=================

Varnish 4 vmod for loading maxminddb (geoip2)

I have no clue what I'm doing, will most likely be insecure.


Build:
  ./autogen.sh
  ./configure  VMOD_DIR=/usr/lib/varnish/vmods/
  make
  make install
