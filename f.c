/* Konrad Eisele <eiselekd@gmail.com, eiselekd@web.de>
 * Inspired by Richard W.M. Jones minmal Forth tutorial and MinForth by Andreas Kochenburger
 * Released under BSD-license */

/* reuse lisp code */
/*#define DBG*/
#include "f1.c"

void sigint(int sig) { exit(0); }
void colreset() { fprintf(FDBG, "[0m"); /* col def */ }
int
main(int argc, char **argv) 
{
    fctx cctx; int c, i; FILE *f; char v[32];
    struct v *fx = 0, *r;
    memset(&cctx,0,sizeof(cctx));

    atexit(colreset);
    signal(SIGINT, sigint);
    
    /* input */
    cctx.f[cctx.fc = 0] = stdin;
    for (i = argc-1; i >= 1; i--) {
        if (f = fopen(argv[i],"r")) {
            cctx.f[++cctx.fc] = f;
        }
    }
    
    f_init(&cctx) ;
    
    r = mkCons(g_QUIT, 0);
    cctx.ip = r;
    fx = NEXT(&cctx);
    
    /* main loop */
    while(fx) {
        fx = fop(fx)(&cctx);
    }

    fprintf(FDBG, "[39m"); /* col def */
    
    return 0;
};

/*
Local Variables:
c-basic-offset:4
indent-tabs-mode:nil
End:
*/
