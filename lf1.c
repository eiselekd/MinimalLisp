/* Konrad Eisele <eiselekd@gmail.com, eiselekd@web.de>
 * Released under BSD-license */

/* reuse lisp code */
/*#define DBG*/
#include "f1.c"

typedef struct lfctx {
    fctx f;
    v *estk; v *genv;
} lfctx;
#define  EPUSH(v) lfctx->estk = mkCons(v, lfctx->estk)
#define  EPOP     pop(&lfctx->estk)
#define  TOSE     car(lfctx->estk)

v *lf_eval(lfctx *lfctx) ;
v *lf_apply_p(lfctx *lfctx) ;

#undef  CODE
#define CODE(in,n,ia0,a0,ia1,a1,f,c)              \
    v *                                           \
    fop_##f(lfctx *lfctx) {                       \
        fctx *cctx = (fctx*) lfctx;               \
        c;                                        \
        dbgprint(cctx,#f);                        \
        return NEXT(cctx);                        \
    };                                            \
    v *g_##f;                                     \
    
#include "lf.h"

void
lf_init(lfctx *cctx)
{
    v *c;
    
    ctx *lctx = &cctx->f.lctx; fctx *fcctx = &cctx->f;
    f_init(&cctx->f);

#undef CODE
#define ARITH(n,a0,a1,f,c) CODE(0,n,0,a0,0,a1,f,c)
#define CODE(in,n,ia0,a0,ia1,a1,f,c) {                                  \
        struct v *e, *ei;                                               \
        g_##f = mkCons(mkFop((fopfn)fop_##f), 0);                       \
        CONS_WORD_VAL(g_##f,1);                                         \
        if (a0) { fcctx->env = mkCons(ei=mkCons(unique(&fcctx->lctx, a0, strlen(a0)), e = mkCons(mkFop((fopfn)fop_##f), 0)),fcctx->env); FOP_IMMEDIATE_VAL(ei, ia0); CONS_WORD_VAL(e,1); } \
        if (a1) { fcctx->env = mkCons(ei=mkCons(unique(&fcctx->lctx, a1, strlen(a1)), e = mkCons(mkFop((fopfn)fop_##f), 0)),fcctx->env); FOP_IMMEDIATE_VAL(ei, ia1); CONS_WORD_VAL(e,1); } \
        fcctx->env = mkCons(ei=mkCons(unique(&fcctx->lctx, n,  strlen(n)),  g_##f),fcctx->env); FOP_IMMEDIATE_VAL(ei, in); \
    }
    
#include "lf.h"

    for (c = lctx->env; c; c=cdr(c)) {
        v *a = car(c); v *e;
        v *o = cdr(a), *n = car(a);
        if (o->tag == OP) {
            if (OP_ARGEVALP(o)) {
                rplacd(a, e = mkCons(car(g_ELISP), mkCons(o,0)));
                printf("Remap %s = 0x%p..\n",str(n),e);
                CONS_WORD_VAL(e,1);
                FN_ARGEVAL_SET(e);
            } else {
            }
        }
    }
}

struct lf_maps { char *ln, *fn; } m[] = {
    { "let", "llet" },
    { "setq", "lsetq" },
    { "define", "ldefine" },
    { "lambda", "llambda" },
    { "if", "lif"},
    { "while", "lwhile"},
    { 0, 0 }
};

void
lf_remap(lfctx *lfctx)
{
    struct ctx *lctx = (struct ctx *) lfctx;
    struct fctx *fctx = (struct fctx *)lfctx;
    v *le = 0, *fe; struct lf_maps *c = m;
    while(c->ln && c->fn) {
        if ((le = assq(unique(lctx,c->ln, strlen(c->ln)), lctx->env)) &&
            (fe = assq(unique(lctx,c->fn, strlen(c->fn)), fctx->env))) {
            rplacd(le,cdr(fe));
            printf("Map %s to %s (0x%p)\n", c->ln, c->fn, cdr(fe));
        } else {
            printf("Cannot map %s to %s\n", c->ln, c->fn);
        }
        c++;
    }
}

v *
lf_eval(lfctx *lfctx) 
{
    fctx *cctx = (fctx *)lfctx;
    v *r = 0, *f; v *e = POP;
    switch(e->tag) {
    case INT: case STR:
        PUSH(e);
        PUSH(mkInt(0));
        break;
    case SYM:
        if ((r = assq(e, TOSE))) {
            r = cdr(r);
        } else {
            fprintf(stderr, "Undefined symbol \"%s\"\n", str(e));
        }
        PUSH(r);
        PUSH(mkInt(0));
        break;
    case FUNC:
        break;
    case CONS:
        PUSH(cdr(e));
        PUSH(car(e));
        PUSH(mkInt(1));
        break;
    }
    return r;
}

v *
lf_apply_p(lfctx *lfctx) 
{
    fctx *cctx = (fctx *)lfctx; v *env = 0;
    v *fn = POP; v *args = POP; v *r = 0, *benv; int a = 0, isop = 0;
    if( (isop = ((fn->tag == FUNC) ? 0 : 1)) || fwordp(fn)) {
        benv = TOSE;
        if (FN_FUNCENVP(fn))
            benv = funcenv(fn);
        /* if (FN_ARGBINDP(fn)) */
        /*     benv = evbind(car(func(fn)), args, benv); /\* error, todo: bind eval'ed args  *\/ */
        env = benv;
        a = (FN_ARGEVALP(fn)) ? 1 : 0;
    } else {
        fprintf(stderr, "Cannot apply to "); print((ctx*)cctx, stderr, fn); fprintf(stderr, "\n"); 
    }
    PUSH(env);
    PUSH(args);
    PUSH(fn);
    PUSH(mkInt(isop));
    PUSH(mkInt(a));
    return r;

}


/*
Local Variables:
c-basic-offset:4
indent-tabs-mode:nil
End:
*/
