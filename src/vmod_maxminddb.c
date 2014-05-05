#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#include "vrt.h"
#include "cache/cache.h"
#include "vcc_if.h"
#include "config.h"
#include "vsa.h"

#include <maxminddb.h>

void
freeit(void *data)
{
	MMDB_close(data);
	free(data);
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
vmod_query(const struct vrt_ctx *ctx, struct vmod_priv *priv, const struct suckaddr *ip)
{
	MMDB_entry_data_s entry;

	if(!priv->priv || !lookup_country(priv->priv, ip, &entry))
		return WS_Copy(ctx->ws, "-", 2);

	return WS_Copy(ctx->ws, entry.utf8_string, entry.data_size);
}

int
init_function(struct vmod_priv *priv, const struct VCL_conf *conf)
{
	return (0);
}
