#include "config.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>

#include <maxminddb.h>

#include "cache/cache.h"

#include "vsa.h"
#include "vcl.h"

#include "vcc_if.h"

void
freeit(void *data)
{
	MMDB_close(data);
	free(data);
}


int
lookup(MMDB_s *db, const struct suckaddr *ip, MMDB_entry_data_s *entry, const char **path, uint32_t type)
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

	if(entry->type != type)
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

	if(!priv->priv || !lookup(priv->priv, ip, &entry, path, MMDB_DATA_TYPE_UTF8_STRING))
		return WS_Copy(ctx->ws, "-", 2);

	char temp[entry.data_size + 1];
	memcpy(temp, entry.utf8_string, entry.data_size);
	temp[entry.data_size] = '\0';
	return WS_Copy(ctx->ws, temp, entry.data_size + 1);

}

VCL_STRING
vmod_query_common_real(const struct vrt_ctx *ctx, struct vmod_priv *priv, const struct suckaddr *ip, const char **path)
{
	MMDB_entry_data_s entry;

	if(!priv->priv || !(lookup(priv->priv, ip, &entry, path, MMDB_DATA_TYPE_DOUBLE) || !lookup(priv->priv, ip, &entry, path, MMDB_DATA_TYPE_FLOAT)))
		return WS_Copy(ctx->ws, "0", 2);

	double value;
	if (entry.type == MMDB_DATA_TYPE_DOUBLE)
		value = entry.double_value;
	else
		value = (double)entry.float_value;

	const int max_len = 9;
	char temp[max_len];
	int len = snprintf(temp, max_len, "%0.4f", value);
	if (len < 0 || len >= max_len)
		return WS_Copy(ctx->ws, "0", 2);

	return WS_Copy(ctx->ws, temp, len + 1);
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

VCL_STRING
vmod_query_latitude(const struct vrt_ctx *ctx, struct vmod_priv *priv, const struct suckaddr *ip)
{
	static const char *latitude_path[] = { "location", "latitude", NULL };
	return vmod_query_common_real(ctx, priv, ip, latitude_path);
}

VCL_STRING
vmod_query_longitude(const struct vrt_ctx *ctx, struct vmod_priv *priv, const struct suckaddr *ip)
{
	static const char *longitude_path[] = { "location", "longitude", NULL };
	return vmod_query_common_real(ctx, priv, ip, longitude_path);
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
