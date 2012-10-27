/* Konrad Eisele <eiselekd@gmail.com, eiselekd@web.de>
 * Released under BSD-license */

#include "flat1.c"

/*******/

int
main(int argc, char **argv) 
{
    flctx _cctx; ctx *cctx = (ctx*)&_cctx; flctx *ctx = &_cctx;
    FILE *f; char *m = 0, *lft; long l, val; int c, ad = 1, i, state = 0; 
    v *cl = mkCons(0,0), *v, *n, *r, *benv, *e, *a;
    memset(&_cctx, 0, sizeof(_cctx));
    
    if (argv[1] && (f = fopen(argv[1],"r"))) {
        fseek(f, 0, SEEK_END); 
        l = ftell(f);
        fseek(f, 0, SEEK_SET); 
        m = (char *)malloc(l+1);
        fread(m, 1, l, f);
        m[l] = 0;
        fclose(f);
        printf("%s", m);
    }
    v = scan(cctx, m);
    
    print(cctx, stderr, v ); printf("\n");

    l_init(cctx) ;
    
    eval_flat(ctx, v);
    return 0;
}

/*
Local Variables:
c-basic-offset:4
indent-tabs-mode:nil
End:
*/
