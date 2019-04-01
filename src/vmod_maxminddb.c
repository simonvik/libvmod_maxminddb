#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <sys/socket.h>

#include "cache/cache.h"
#include "vcc_if.h"
#include "config.h"
#include "vsa.h"
#include "vcl.h"

#include <maxminddb.h>

void
freeit(void *data)
{
	MMDB_close(data);
	free(data);
}


int
lookup(MMDB_s *db, const struct suckaddr *ip, MMDB_entry_data_s *entry, const char **path)
{
	int error, r;
	socklen_t sl;
	const struct sockaddr *sa;
	MMDB_lookup_result_s s;


	if (NULL == (sa = VSA_Get_Sockaddr(ip, &sl)))
		return 0;

	s = MMDB_lookup_sockaddr(db, sa, &error);

	if(!s.found_entry)
		return 0;

	r = MMDB_aget_value(&s.entry, entry, path);

	if (r != MMDB_SUCCESS || !entry->has_data)
		return 0;

	if(entry->type != MMDB_DATA_TYPE_UTF8_STRING )
		return 0;

	return 1;
}

VCL_VOID
vmod_init_db(const struct vrt_ctx *ctx, struct vmod_priv *priv, const char *filename)
{
	priv->priv = (MMDB_s *)calloc(1, sizeof(MMDB_s));
	if (priv->priv == NULL)
		return;

	if(MMDB_open(filename, MMDB_MODE_MMAP, priv->priv) != MMDB_SUCCESS){
		free(priv->priv);
		return;
	}
	priv->free = freeit;
}

VCL_STRING
vmod_query_common(const struct vrt_ctx *ctx, struct vmod_priv *priv, const struct suckaddr *ip, const char **path)
{
	MMDB_entry_data_s entry;

	if(!priv->priv || !lookup(priv->priv, ip, &entry, path))
		return WS_Copy(ctx->ws, "-", 2);

	char temp[entry.data_size + 1];
	memcpy(temp, entry.utf8_string, entry.data_size);
	temp[entry.data_size] = '\0';
	return WS_Copy(ctx->ws, temp, entry.data_size + 1);

}

VCL_STRING
vmod_query_continent(const struct vrt_ctx *ctx, struct vmod_priv *priv, const struct suckaddr *ip)
{
	static const char *continent_path[] = { "continent", "code", NULL };
	return vmod_query_common(ctx, priv, ip, continent_path);
}

VCL_STRING
vmod_query_country(const struct vrt_ctx *ctx, struct vmod_priv *priv, const struct suckaddr *ip)
{
	static const char *country_path[] = { "country", "iso_code", NULL };
	return vmod_query_common(ctx, priv, ip, country_path);
}

VCL_STRING
vmod_query_state(const struct vrt_ctx *ctx, struct vmod_priv *priv, const struct suckaddr *ip)
{
	static const char *state_path[] = { "subdivisions", "0", "iso_code", NULL };
	return vmod_query_common(ctx, priv, ip, state_path);
}

VCL_STRING
vmod_query_city(const struct vrt_ctx *ctx, struct vmod_priv *priv, const struct suckaddr *ip)
{
	static const char *city_path[] = { "city", "names", "en", NULL };
	return vmod_query_common(ctx, priv, ip, city_path);
}

VCL_STRING
vmod_query_postalcode(const struct vrt_ctx *ctx, struct vmod_priv *priv, const struct suckaddr *ip)
{
	static const char *postalcode_path[] = { "postal", "code", NULL };
	return vmod_query_common(ctx, priv, ip, postalcode_path);
}

// keep function vmod_query() for compatibility
VCL_STRING
vmod_query(const struct vrt_ctx *ctx, struct vmod_priv *priv, const struct suckaddr *ip)
{
	return vmod_query_country(ctx, priv, ip);
}

int
event_function(VRT_CTX, struct vmod_priv *priv, enum vcl_event_e e)
{
	return (0);
}

