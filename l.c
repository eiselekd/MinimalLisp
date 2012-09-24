/* Konrad Eisele <eiselekd@gmail.com, eiselekd@web.de>
 * Based on Ian Piumarta's lysp.c. 
 * Released under BSD-license */

#include "l1.c"

/*******/
/* main, setup and parsing */

int
main(int argc, char **argv) 
{
    ctx cctx; 
    FILE *f; char *m = 0, *lft; long l, val; int c, ad = 1, i; 
    v *cl = mkCons(0,0), *v, *n, *r;
    memset(&cctx, 0, sizeof(cctx));
    
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
    v = scan(&cctx, m);
    
    print(&cctx, stderr, v ); printf("\n");

    l_init(&cctx) ;
    
    evlist(&cctx, v, cctx.env);
    
    return 0;
}

/*
Local Variables:
c-basic-offset:4
indent-tabs-mode:nil
End:
*/
