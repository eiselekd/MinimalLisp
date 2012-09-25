/* Konrad Eisele <eiselekd@gmail.com, eiselekd@web.de>
 * Released under BSD-license */

/* reuse lisp code */
#define NOFOUT
/*#define DBG*/
#include "lf1.c"

void sigint(int sig) { exit(0); }
void colreset() { fprintf(FDBG, "[0m"); /* col def */ }

int
main(int argc, char **argv) 
{
    lfctx _cctx; struct lfctx *lfctx = &_cctx; ctx *lctx = (ctx *)&_cctx; 
    fctx *cctx = (fctx*)&_cctx; int c, i, l; FILE *f; char v[32]; struct v *list;
    struct v *fx = 0, *r; char *m = 0; int ml = 0, o;
    memset(&_cctx,0,sizeof(struct lfctx));
    
    atexit(colreset);
    signal(SIGINT, sigint);
    
    /* input */
    cctx->fc = -1;
    for (i = argc-1; i >= 1; i--) {
        l = strlen(argv[i]);
        if (argv[i][l-1] == 'f') {
            if (f = fopen(argv[i],"r")) {
                cctx->f[++cctx->fc] = f;
            }
        } else if (argv[i][l-1] == 'l') {
            FILE *f;
            if (argv[i] && (f = fopen(argv[i],"r"))) {
                o = ml;
                fseek(f, 0, SEEK_END); 
                ml += (l = ftell(f));
                fseek(f, 0, SEEK_SET);
                if (m) {
                    m = (char *)realloc(m,ml+1);
                } else {
                    m = (char *)malloc(ml+1);
                }
                fread(&m[o], 1, l, f);
                m[ml] = 0;
                fclose(f);
            }
        }
    }
    
    lf_init(&_cctx) ;
    
    r = mkCons(g_QUIT, 0);
    cctx->ip = r;
    fx = NEXT(cctx);
    
    FPRINTF(FDBG, "Compile Forth interpreter\n");
    while(fx) {
        fx = fop(fx)(cctx);
    }
    
    lf_remap(&_cctx);
    
    /* scan lisp program */
    if (!m)
        return 0;
    
    list = scan((ctx *)cctx, m);
    
    PUSH(list);
    
    printh ((ctx*)cctx, stdout, list);
    
    if (!(r = find(cctx, unique((ctx *)cctx, "evlist", 6)))) {
        return 0;
    }
    
    cctx->ip = mkCons(cdr(r), mkCons(g_END,0));
    fx = NEXT(cctx);
    EPUSH(lctx->env);
    
    FPRINTF(FDBG, "Execute Forth Lisp interpreter\n");
    /* execute lisp interpreter */
    while(fx) {
        fx = fop(fx)(cctx);
    }
    
    return 0;
};

/*
Local Variables:
c-basic-offset:4
indent-tabs-mode:nil
End:
*/
