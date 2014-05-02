#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#include "vrt.h"
#include "cache/cache.h"
#include "vcc_if.h"
#include "config.h"
#include "vsa.h"

#include <maxminddb.h>
#define GI_UNKNOWN_STRING "Unknown"

void freeit(void *data){
	MMDB_close(data);
	free(data);
}


int
open_db(struct vmod_priv *priv){
	priv->priv = (MMDB_s *)malloc(sizeof(MMDB_s));
	priv->free = freeit;
	if(MMDB_open("/home/simon/GeoLite2-City.mmdb", MMDB_MODE_MMAP, priv->priv) != MMDB_SUCCESS)
		return 0;

	return 1;
}

int
lookup_country(MMDB_s *db, const struct suckaddr *ip, MMDB_entry_data_s *entry)
{
	static const char *country_path[] = { "country", "iso_code", NULL };
	int error, r;
	char *result;
	socklen_t sl;
	const struct sockaddr *sa;
	MMDB_lookup_result_s s;


	if (NULL == (sa = VSA_Get_Sockaddr(ip, &sl)))
		return 0;

	s = MMDB_lookup_sockaddr(db, sa, &error);

	if(!s.found_entry)
		return 0;

	r = MMDB_aget_value(&s.entry, entry, country_path);

	if (r != MMDB_SUCCESS || !entry->has_data)
		return 0;

	if(entry->type != MMDB_DATA_TYPE_UTF8_STRING )
		return 0;

	return 1;
}

VCL_STRING
vmod_query(const struct vrt_ctx *ctx, struct vmod_priv *priv, const struct suckaddr *ip)
{
	MMDB_entry_data_s entry;

	if(!priv->priv){
		open_db(priv);
	}


	if(!lookup_country(priv->priv, ip, &entry))
		return WS_Copy(ctx->ws, "-", 2);

	return WS_Copy(ctx->ws, entry.utf8_string, entry.data_size);
}



int
init_function(struct vmod_priv *priv, const struct VCL_conf *conf)
{
	return (0);
}
