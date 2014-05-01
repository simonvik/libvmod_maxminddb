#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#include "vrt.h"
#include "cache/cache.h"
#include "vcc_if.h"
#include "config.h"



int 
body_iterator(struct req *req, void *priv, void *buf, size_t size)
{
	if(size == 0)
		return 0;

	if(strstr((char*) buf, "viagra") != NULL){
		return 1000;
	}
}



VCL_BOOL
vmod_req_ok(const struct vrt_ctx *ctx, struct vmod_priv *priv)
{

	//unsigned char *str;
	//str = WS_Copy(ctx->ws, "hello", 6);

	//HTTP1_IterateReqBody(struct req *req, req_body_iter_f *func, void *priv)
	//typedef int (req_body_iter_f)(struct req *, void *priv, void *ptr, size_t);
	//i = func(req, priv, buf, l);

	if(ctx->req->req_bodybytes <= 1024){
		if(HTTP1_IterateReqBody(ctx->req, body_iterator, priv) >= 1000){
			return 1;
		}
	}

	return 0;
}

int
init_function(struct vmod_priv *priv, const struct VCL_conf *conf)
{
	return (0);
}
