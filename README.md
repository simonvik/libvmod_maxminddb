libvmod_maxminddb
=================

Varnish 4 vmod for loading maxminddb (geoip2)

**I have no clue what I'm doing, will most likely be insecure.**



Build:
=================
You need libmaxminddb in order to build this.

```
 ./autogen.sh
 ./configure  VMOD_DIR=/usr/lib/varnish/vmods/
 make
 make install
```

Usage:
=================
```
import maxminddb;

sub vcl_recv {
        set req.http.countrycode = maxminddb.query(client.ip);
        return (synth(200, req.http.countrycode));
}

```
