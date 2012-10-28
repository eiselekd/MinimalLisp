/* Konrad Eisele <eiselekd@gmail.com, eiselekd@web.de>
 * Inspired by Ian Piumarta's lysp.c. 
 * Released under BSD-license */

#ifndef FLAT__H
#define FLAT__H

#include "l1.c"

typedef struct flctx {
    ctx lctx;
    v *rstk, *estk;
} flctx;

#define RSTK     ctx->rstk
#define ESTK     ctx->estk
#define RSTKRET  caar(RSTK)
#define RSTKSCP0 cdar(RSTK)
#define RSTKSCP1 cdar(cdr(RSTK))

#define RCALL(l,a)           RSTK = mkCons(mkCons(mkInt(l),mkCons(0,mkCons(0,a))),RSTK);
#define RRET                 state = num(RSTKRET); rplaca(_nth(RSTKSCP1,1,1), S(0)); RSTK = cdr(RSTK)
#define RCALL_EVAL(l,v)      RCALL(l,mkCons(v,0)); state = EVAL_0;
#define RCALL_EVLIST(l,v)    RCALL(l,mkCons(v,0)); state = EVLIST_0;
#define RCALL_EVARGS(l,v)    RCALL(l,mkCons(v,0)); state = EVARGS_0;
#define RCALL_APPLY(l,f,a)   RCALL(l,mkCons(f,mkCons(a,0))); state = APPLY_0;

#define S(i)        car(nth(RSTKSCP0,i))
#define S_SET(i,v)  rplaca(_nth(RSTKSCP0,i,1),v)
#define PUSH_ENV(e) ESTK = mkCons(e,ESTK)
#define POP_ENV()   ESTK = cdr(ESTK)
#define ENV         car(ESTK)

int
eval_flat(flctx *ctx, struct v *v) 
{
    FILE *f; char *m = 0, *lft; long l, val; int c, ad = 1, i, state = 0; 
    struct v *n, *r, *benv, *e, *a; opfn opf;

    enum {
        EVLIST_0,
        EVLIST_1,
        EVARGS_0, 
        EVARGS_1, 
        EVARGS_2, 
        EVAL_0,
        EVAL_1,
        EVAL_2,
        APPLY_0,
        APPLY_1,
        APPLY_2,
        APPLY_3,
        IF_0,
        IF_1,
        IF_2,
        DEFINE_0,
        DEFINE_1,
        WHILE_0,
        WHILE_1,
        WHILE_2,
        LET_0,
        LET_1,
        LET_2,
        SETQ_0,
        SETQ_1,
        QUIT_0,
    };

    struct {
        char *n; int state;
    } opm[] = {
        /* recursing functions */
        { "let", LET_0 },
        { "setq", SETQ_0 },
        { "define", DEFINE_0 },
        { "if", IF_0 },
        { "while", WHILE_0 },
        { 0, 0 }
    };

    PUSH_ENV(ctx->lctx.env);
    RCALL_EVLIST(QUIT_0, v);
    
    for (i = 0; i < ((sizeof(opm) / sizeof(opm[0])) - 1); i++) {
        struct v *p, *n = unique(&ctx->lctx, opm[i].n, strlen(opm[i].n));
        if ((v = assq(n, ctx->lctx.env))) {
            p = cdr(v);
            if ((p->tag == OP && 
                 !OP_ARGEVALP(p))) {
                op(p) = (opfn) (long)opm[i].state;
            } else {
                printf("!Cannot remap: %s\n", opm[i].n);
            }
        }
    }
    
    while (1) {
        switch(state) {
            
            /*
evlist(ctx *cctx, v *l, v *env)
{
    struct v *v = l, *r = 0;
    for (; v; v = cdr(v)) {
        r = eval(cctx, car(v),env);
    }
    return r;
}
local scope: 0(ret): return arg
             1(call-ret): return arg from called func
             2(arg-l): v iteration var
             (env on env stack)
            */

        case EVLIST_0:
            S_SET(0, 0);                  /* EVLIST_0: r = 0; */
        evlist_loop:
            if (!(v = S(2)))              /* for (; <v> ; v = cdr(v)) */
                goto ret;
            RCALL_EVAL(EVLIST_1, car(v));  /* r = < eval(cctx, car(v),env); > */
            break;
        case EVLIST_1:
            S_SET(0, S(1));               /* EVLIST_1: < r = > ... */
            S_SET(2,cdr(S(2)));           /* for (; v ; < v = cdr(v) > ) */
            goto evlist_loop;
            
            
            /* 
v *
evargs(ctx *cctx, v *l, v *env)
{
    struct v *r = 0;
    if (l) {
        r = eval(cctx, car(l),env);
        r =  mkCons(r,evargs(cctx, cdr(l),env));
    }
    return r;
}
local scope: 0(ret): return arg
             1(call-ret): return arg from called func
             2(arg-l): 
             3: tmp return of eval
            */

        case EVARGS_0:
            S_SET(0,0);                         /* EVARGS_0: r = 0; */ 
            if (!(v=S(2)))                      /* if (l) */
                goto ret;
            RCALL_EVAL(EVARGS_1, car(v));        /* r = < eval(cctx, car(l),env); > */
            break;
        case EVARGS_1:
            S_SET(3,S(1));                      /* EVARGS_1: < r = ... > */
            RCALL_EVARGS(EVARGS_2, cdr(S(2)));   /*  r =  mkCons(r, < evargs(cctx, cdr(l),env) > ); */
            break;
        case EVARGS_2:
            S_SET(0,mkCons(S(3),S(1)));         /* EVARGS_2: < r =  mkCons(r,  evargs(cctx, cdr(l),env)  ); > */
            goto ret;
            
            /*
v *
eval(ctx *cctx, v *e, v *env) 
{
    v *r = 0, *f;
    switch(e->tag) {
    case INT: case STR:
        r = e;
        break;
    case SYM:
        if ((r = assq(e, env))) {
            r = cdr(r);
        } else {
            fprintf(stderr, "Undefined symbol \"%s\"\n", str(e));
        }
        break;
    case FUNC:
        break;
    case CONS:
        f = eval(cctx, car(e), env);
        r = apply(cctx, f, cdr(e), env);
        break;
    }
    return r;
}
local scope: 0(ret): return arg
             1(call-ret): return arg from called func
             2(arg-e): 
             3: tmp return of eval
            */

        case EVAL_0:
            S_SET(0,0);                           /* EVAL_0: r = 0 */
            v = S(2);
            switch(v->tag) {
            case INT: case STR:
                S_SET(0, v);                      /* r = e */
                goto ret;
            case SYM:
                if ((e = assq(v, ENV))) {
                    e = cdr(e);
                } else {
                    fprintf(stderr, "Undefined symbol \"%s\"\n", str(v));
                }
                S_SET(0, e);
                goto ret;
            case FUNC: default:
                goto ret;
            case CONS:
                RCALL_EVAL(EVAL_1, car(S(2)));     /* f = <eval(cctx, car(e), env)>; */
                break;
            }
            break;
        case EVAL_1:
            v = S(1);
            RCALL_APPLY(EVAL_2, v, cdr(S(2)));  /* EVAL_1: r = <apply(cctx, f, cdr(e), env)>; */
            break;
        case EVAL_2:
            S_SET(0, S(1));                       /* EVAL_2: <r = ...>  */
            goto ret;

            /*
v *
apply(ctx *cctx, v *fn, v *args, v *env) 
{
    v *r = 0, *benv;
    if (!fn)
        return 0;
    switch(fn->tag) {
    case OP:
        if (OP_ARGEVALP(fn))
            args = evargs(cctx,args,env);
        r = op(fn)(cctx, args, env);
        break;
    case FUNC:
        benv = env;
        if (FN_ARGEVALP(fn))
            args = evargs(cctx,args,env);
        if (FN_FUNCENVP(fn))
            benv = funcenv(fn);
        if (FN_ARGBINDP(fn))
            benv = evbind(car(func(fn)), args, benv);
        r = evlist(cctx, cdr(func(fn)),benv);
        break;
    default:
        fprintf(stderr, "Cannot apply to "); print(cctx, stderr, fn); fprintf(stderr, "\n"); 
        break;
    }
    return r;
}
local scope: 0(ret): return arg
             1(call-ret): return arg from called func
             2(arg-fn): func
             3(arg-args): args iteration var
             (env on env stack)

            */

        case APPLY_0:
            S_SET(0,0);                                  /* APPLY_0: */
            if (!(v=S(2)))                               /* if (!fn) */
                goto ret;
            S_SET(1, S(3));
            if (v->tag == OP && OP_ARGEVALP(v) ||
                v->tag == FUNC && FN_ARGEVALP(v)) {
                RCALL_EVARGS(APPLY_1, S(3));
                break;
            }
        case APPLY_1:
            a = S(1);                                    /* APPLY_1: args = ... */
            v = S(2);
            switch(v->tag) {
            case OP:
                opf = op(v);
                if (((long)opf) >= 0 && ((long)opf) <= QUIT_0) {
                    RCALL(APPLY_2,mkCons(a,0)); 
                    state = (long)opf;
                } else {
                    r = opf(&ctx->lctx, a, ENV);
                    S_SET(0,r);
                    goto ret;
                }
                break;
            case FUNC:
                benv = ENV;
                if (FN_FUNCENVP(v))
                    benv = funcenv(v);
                if (FN_ARGBINDP(v))
                    benv = evbind(car(func(v)), a, benv);
                PUSH_ENV(benv);
                RCALL_EVLIST(APPLY_3, cdr(func(v)));
                break;
            default:
                fprintf(stderr, "Cannot apply to "); print((struct ctx*)ctx, stderr, v); fprintf(stderr, "\n"); 
                goto ret;
            }
            break;

        case APPLY_3:
            POP_ENV(); /* fall through */
        case APPLY_2:
            S_SET(0, S(1));                              /* APPLY_2: r = ... */
            goto ret;

            /*
v*
ifFunc(ctx *cctx, v *a, v *env ) 
{
    v *r = 0, *n, *l;
    r = eval(cctx, car(a), env) ? eval(cctx,cadr(a),env) : eval(cctx,caddr(a), env);
    return r;
}
local scope: 0(ret): return arg
             1(call-ret): return arg from called func
             2(arg-a): v iteration var
             (env on env stack)

            */

        case IF_0:
            S_SET(0,0);                          /* IF_0 */
            RCALL_EVAL(IF_1, car(S(2))) ;
            break;
        case IF_1:
            v = tonum(S(1)) ? cadr(S(2)) : caddr(S(2)); /* IF_1 */
            RCALL_EVAL(IF_2, v);
            break;
        case IF_2:    
            S_SET(0,S(1));     /* IF_2: r = ... */
            goto ret;
           
            /*
v*
whileFunc(ctx *cctx, v *a, v *env ) 
{
    v *r = 0, *n, *l;
    while(eval(cctx,car(a),env)) r = evlist(cctx, cdr(a),env);
    return r;
}
local scope: 0(ret): return arg
             1(call-ret): return arg from called func
             2(arg-a): iteration var
             3(n): local n
             4(n): local env
             (env on env stack)
            */
                
        case WHILE_0:
            S_SET(0,0);                          /* WHILE_0 */
            
        whileloop:
            RCALL_EVAL(WHILE_1, car(S(2))) ;     /* eval(cctx,car(a),env) */
            break;
        case WHILE_1:
            if (!tonum(S(1)))                    /* WHILE_1:  while(eval(cctx,car(a),env)) */
                goto ret;
            RCALL_EVLIST(WHILE_2, cdr(S(2))) ;   /* evlist(cctx, cdr(a),env) */
            break;
        case WHILE_2:
            S_SET(0,S(1));                       /* WHILE_2: r = evlist(cctx, cdr(a),env) */
            goto whileloop;

            /*
 
v*
defineFunc(ctx *cctx, v *a, v *env ) 
{
    v *r = 0, *n, *c;
    if (a) {
        r = mkCons(car(a), 0);
        rplacd(cctx->env, mkCons(r, cdr(cctx->env)));
        r = rplacd(r, eval(cctx,cadr(a),env));
    }
    return r;
}

local scope: 0(ret): return arg
             1(call-ret): return arg from called func
             2(arg-a): v iteration var
             3(n): local n
             4(n): local env
             (env on env stack)

            */

        case DEFINE_0:
            S_SET(0,0);                          /* DEFINE_0 */
            if (!(v = S(2)))
                goto ret;
            v = mkCons(car(S(2)), 0);            /* r = mkCons(car(a), 0); */
            S_SET(3,v);
            rplacd(ctx->lctx.env, mkCons(v, cdr(ctx->lctx.env))); /*  rplacd(cctx->env, mkCons(r, cdr(cctx->env))); */
            RCALL_EVAL(DEFINE_1, cadr(S(2))) ;
            break;
        case DEFINE_1:
            v = rplacd(S(3), S(1));              /*  r = rplacd(r, eval(cctx,cadr(a),env)); */
            S_SET(0, v);
            goto ret;
            
            /*
v*
letFunc(ctx *cctx, v *a, v *env ) {
    v *r = 0, *n, *l;
    for (n = car(a); n; n = cdr(n)) {
        l = eval(cctx,cadar(n),env);
        l = mkCons(caar(n), l);
        env = mkCons(l, env);
    }
    return evlist(cctx, cdr(a), env);
}

local scope: 0(ret): return arg
             1(call-ret): return arg from called func
             2(arg-a): v iteration var
             3(n): local n
             4(n): local env
             (env on env stack)

            */            

        LET_0:
            S_SET(0,0);                      /* LET_0: r = 0 */
            S_SET(4,ENV);                    
            v = car(S(2));                   /* for (< n = car(a) >; n; n = cdr(n)) */
        let_loop:
            if (!v)                          /* for (n = car(a); < n > ; n = cdr(n)) */
                goto let_ret;
            S_SET(3, v);
            RCALL_EVAL(LET_1,cadar(v));       /*  eval(cctx,cadar(n),env); */
            break;
        LET_1:
            v = mkCons(caar(S(3)), S(1));    /*  LET_1: l = mkCons(caar(n), l); */
            v = mkCons(v, S(4));             /*  env = mkCons(l, env); */
            S_SET(4, v);
            v = cdr(S(3));                   /* for (n = car(a); n; < n = cdr(n) > ) */
            goto let_loop;
        let_ret:
            PUSH_ENV(S(4));
            RCALL_EVLIST(LET_2, cdr(S(2)));   /*  evlist(cctx, cdr(a), env); */
            break;
        LET_2:
            POP_ENV();                       /* LET_2 */
            S_SET(0,S(1));                   /* r = ... */
            goto ret;
            
            /*
v*
setqFunc(ctx *cctx, v *a, v *env ) 
{
    v *r = 0, *k, *l, *c;
    k = car(a);
    if (SymP(k)) {
        r = eval(cctx,cadr(a), env);
        if ((c = assq(k, env))) {
            rplacd(c, r);
        }
    }
    return r;
}
local scope: 0(ret): return arg
             1(call-ret): return arg from called func
             2(arg-a): v iteration var
             3(k): local k
             (env on env stack)

            */
                 
        case SETQ_0:
            S_SET(0,0);      /* SETQ_0 */
            v = car(S(2));
            if (!SymP(v))
                goto ret;
            S_SET(3, v);
            RCALL_EVAL(SETQ_1,cadr(S(2)));  /* r = eval(cctx,cadr(a), env); */
            break;
        case SETQ_1:
            S_SET(0, S(1));                /* SETQ_1 */
            if ((v = assq(S(3), ENV))) {   /* r = ... */
                rplacd(v, S(1));
            }
            goto ret;
        case QUIT_0:
            goto ex;
        ret: 
            RRET;
        }
    }
ex:    
    return 0;
}




#endif

/*
Local Variables:
c-basic-offset:4
indent-tabs-mode:nil
End:
*/
