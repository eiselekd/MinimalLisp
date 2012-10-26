/* Konrad Eisele <eiselekd@gmail.com, eiselekd@web.de>
 * Based on Ian Piumarta's lysp.c. 
 * Released under BSD-license */

#include "flat1.c"

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
        IF_0,
        IF_1,
        IF_2,
    };

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
            S_SET(0, 0);                  /* r = 0; */
        evlist_loop:
            if (!(v = S(2)))              /* for (; <v> ; v = cdr(v)) */
                goto ret;
            CALL_EVAL(EVLIST_1, car(v));  /* r = < eval(cctx, car(v),env); > */
            break;
        case EVLIST_1:
            S_SET(0, S(1));               /* < r = > ... */
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
            S_SET(0,0);                         /* r = 0; */ 
            if (!(v=S(2)))                      /* if (l) */
                goto ret;
            CALL_EVAL(EVARGS_1, car(v));        /* r = < eval(cctx, car(l),env); > */
            break;
        case EVARGS_1:
            S_SET(3,S(1));                      /* < r = ... > */
            CALL_EVARGS(EVARGS_2, cdr(S(2)));   /*  r =  mkCons(r, < evargs(cctx, cdr(l),env) > ); */
            break;
        case EVARGS_2:
            S_SET(0,mkCons(S(3),S(1)));         /*  < r =  mkCons(r,  evargs(cctx, cdr(l),env)  ); > */
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
            S_SET(0,0);                           /* r = 0 */
            v = S(2);
            switch(v->tag) {
            case INT: case STR:
                S_SET(0, v);                      /* r = e */
                goto ret;
            case SYM:
                if ((v = assq(v, env))) {
                    v = cdr(v);
                } else {
                    fprintf(stderr, "Undefined symbol \"%s\"\n", str(e));
                }
                S_SET(0, v);
                goto ret;
            case FUNC: default:
                goto ret;
            case CONS:
                CALL_EVAL(EVAL_1, car(S(2)));     /* f = <eval(cctx, car(e), env)>; */
                break;
            }
            break;
        case EVAL_1:
            CALL_APPLY(EVAL_2, S(1), cdr(S(2)));  /* r = <apply(cctx, f, cdr(e), env)>; */
            break;
        case EVAL_2:
            S_SET(0, S(1));                       /* <r = ...>  */
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
            S_SET(0,0);
            if (!(v=S(2)))                               /* if (!fn) */
                goto ret;
            S_SET(1, S(3));
            if (v->tag == OP && OP_ARGEVALP(v) ||
                v->tag == FUNC && FN_ARGEVALP(v)) {
                CALL_EVARGS(APPLY_1, S(3));
                break;
            }
        case APPLY_1:
            a = S(1);                                    /* args = ... */
            v = S(2);
            switch(v->tag) {
            case OP:
                ...
                r = op(fn)(cctx, args, env);
                break;
            case FUNC:
                benv = ENV;
                if (FN_FUNCENVP(v))
                    benv = funcenv(v);
                if (FN_ARGBINDP(v))
                    benv = evbind(car(func(v)), args, benv);
                PUSH_ENV(benv);
                CALL_EVLIST(APPLY_2, cdr(func(fn)));
                break;
            default:
                fprintf(stderr, "Cannot apply to "); print(cctx, stderr, fn); fprintf(stderr, "\n"); 
                goto ret;
            }
            break;
            
        case APPLY_2:
            S_SET(0, S(1));                              /* r = ... */
            POP_ENV();
            break;
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
            S_SET(0,0);
            r = CALL_EVAL(IF_1, car(S(2))) ;
            break;
        case IF_1:
            v = S(1) ? cadr(S(2)) : caddr(S(2));
            CALL_EVAL(IF_2, v);
            break;
        case IF_2:
            S_SET(0,S(1));     /* r = ... */
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
            S_SET(0,0);                      /* r = 0 */
            S_SET(4,env);                    
            v = car(S(2));                   /* for (< n = car(a) >; n; n = cdr(n)) */
        let_loop:
            if (!v)                          /* for (n = car(a); < n > ; n = cdr(n)) */
                goto let_ret;
            S_SET(3, v);
            CALL_EVAL(LET_1,cadar(v));       /*  eval(cctx,cadar(n),env); */
            break;
        LET_1:
            v = mkCons(caar(S(3)), S(1));    /*  l = mkCons(caar(n), l); */
            v = mkCons(v, S(4));             /*  env = mkCons(l, env); */
            S_SET(4, v);
            v = cdr(S(3));                   /* for (n = car(a); n; < n = cdr(n) > ) */
            goto let_loop;
        let_ret:
            PUSH_ENV(S(4));
            CALL_EVLIST(LET_2, cdr(S(2)));   /*  evlist(cctx, cdr(a), env); */
            break;
        LET_2:
            POP_ENV();
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
            S_SET(0,0);
            v = car(S(2));
            if (!SymP(v))
                goto ret;
            S_SET(3, v);
            CALL_EVAL(SETQ_1,cadr(S(2)));  /* r = eval(cctx,cadr(a), env); */
            break;
        case SETQ_1:
            S_SET(0, S(1));
            if ((v = assq(S(3), env))) {   /* r = ... */
                rplacd(v, S(1));
            }
            goto ret;
    
        }
        

    }
    
    return 0;
}

/*
Local Variables:
c-basic-offset:4
indent-tabs-mode:nil
End:
*/
