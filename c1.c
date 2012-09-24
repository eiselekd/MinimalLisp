/* Konrad Eisele <eiselekd@gmail.com, eiselekd@web.de>
 * Based on Ian Piumarta's lysp.c. 
 * Released under BSD-license */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

void error(char *f, ...) {  va_list ap; va_start(ap, f); vprintf(f, ap); exit(1); va_end(ap); }

enum { CONS, INT, STR, SYM, FUNC, OP };
struct ctx;
typedef struct v {
    unsigned int tag : 4;
    unsigned int flags : 28;
    union {
        long i;
        char *str;
        struct {
            struct v *a, *d;
        } cons;
        struct {
            struct v *f, *env;
        } fn;
        struct v*(*p)(struct ctx *, struct v*, struct v*);
    } u;
} v;
typedef struct ctx {
    v *sym;
    v *env;
} ctx;
v *apply(ctx *cctx, v *fn, v *args, v *env) ;
v *eval(ctx *cctx, v *e, v *env) ;
v *evlist(ctx *cctx, v *e, v *env) ;
v *assq(v *e, v *env) ;

#define FN_ARGEVALP(v) ((v)->flags & (1<<0))
#define FN_FUNCENVP(v) ((v)->flags & (1<<1))
#define FN_ARGBINDP(v) ((v)->flags & (1<<2))
#define FN_ARGEVAL_SET(v) (v)->flags |= ((1)<<0)
#define FN_FUNCENV_SET(v) (v)->flags |= ((1)<<1)
#define FN_ARGBIND_SET(v) (v)->flags |= ((1)<<2)
#define OP_ARGEVALP(v) FN_ARGEVALP(v)
#define OP_ARGEVAL_SET(v) FN_ARGEVAL_SET(v)

v *DotSym = 0;
v *mkV(int t) { v *v = malloc(sizeof(struct v)); v->tag = t; return v; }
v *mkCons(v *a, v *d) { v *v = mkV(CONS); v->u.cons.a = a; v->u.cons.d = d; return v;}
v *mkInt(long val) { v *v = mkV(INT); v->u.i = val; return v;}
v *mkFunc(v *f, v *env) { v *v = mkV(FUNC); v->u.fn.f = f; v->u.fn.env = env; return v;}
v *mkOp(struct v*(*p)(ctx *,struct v*, struct v*)) { v *v = mkV(OP); v->u.p = p; return v;}
v *_mkStr(char *s, int l, int t) { v *v = mkV(t); v->u.str = (char *)malloc(l+1); memcpy(v->u.str, s, l); v->u.str[l] = 0; return v; }
#define mkStr(s, l) _mkStr(s, l, STR)
#define mkSym(s, l) _mkStr(s, l, SYM)
#define is(v,t) ((v) && (v)->tag == t)
#define ConsP(v) is(v,CONS)
#define SymP(v) is(v,SYM)
#define car(v) (v)->u.cons.a
#define cdr(v) (v)->u.cons.d
#define num(v) (v)->u.i
#define str(v) (v)->u.str
#define func(v) (v)->u.fn.f
#define op(v) (v)->u.p
#define funcenv(v) (v)->u.fn.env
#define caar(v) car(car(v)) 
#define cdar(v) cdr(car(v))
#define cadr(v) car(cdr(v)) 
#define cadar(v) car(cdr(car(v)))
#define caddr(v) car(cdr(cdr(v)))
v *rplac(v *a, int ad, v *b) { ad ? (cdr(a) = b) : (car(a) = b); return b; }
#define rplaca(a,b) rplac(a,0,b)
#define rplacd(a,b) rplac(a,1,b)

void print(FILE *s, v *v) {
    char *d = "";
    if (!v) fprintf(s, "nil");
    else {
        switch(v->tag) {
        case CONS:
            fprintf(s, "(");
            for (; v && ConsP(v); v = cdr(v)) {
                fprintf(s," ");
                print(s, car(v));
            }
            if (v)
                print(s, v);
            fprintf(s, ")");
            break;
        case INT:
            fprintf(s, "%ld", v->u.i);
            break;
        case STR:
            d = "\"";
        case SYM:
            fprintf(s, "%s%s%s", d, v->u.str, d);
            break;
        }
    }
    fflush(s);
}

/*********/
/* operators functions operating on evaluated s-expressions args */

#define arith(n,i,o) v *n##Func(ctx *cctx, v *a, v *env) {        \
        long l;                                         \
        if (!a) return 0;                               \
        l = num(car(a));                                \
        if (a = cdr(a)) {                               \
            for (; a; a = cdr(a)) {                     \
                l o##= num(car(a));                     \
            }                                           \
        } else {                                        \
            l = i o l;                                  \
        }                                               \
        return mkInt(l);                                \
    }

arith(add,0,+);
arith(subtract,0,-);
arith(mul,1,*);
arith(div,1,/);
arith(mod,1,%);

#define rel(n,o) v *n##Func(ctx *cctx, v *a, v *env) {  \
        v *l = a;                                       \
        for (; cdr(l); l = cdr(l)) {                    \
            if (!(num(car(l)) o num(cadr(l)))) {        \
                return 0;                               \
            }                                           \
        }                                               \
        return a;                                       \
    }

rel(lt,<);
rel(lte,<=);
rel(eq,==);
rel(neq,!=);
rel(gte,>=);
rel(gt,>);

v*
printFunc(ctx *cctx, v *a, v *env ) {
    v *r = 0;
    for (; a; a = cdr(a)) {
        print(stdout, car(a));
    }
    fprintf(stdout, "\n");
    return 0;
}

/*********/
/* syntactical functions operating on non-evaluated s-expressions args */

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

v*
whileFunc(ctx *cctx, v *a, v *env ) 
{
    v *r = 0, *n, *l;
    while(eval(cctx,car(a),env)) r = evlist(cctx, cdr(a),env);
    return r;
}

v*
ifFunc(ctx *cctx, v *a, v *env ) 
{
    v *r = 0, *n, *l;
    r = eval(cctx, car(a), env) ? eval(cctx,cadr(a),env) : eval(cctx,caddr(a), env);
    return r;
}

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

v*
lambdaFunc(ctx *cctx, v *a, v *env ) {
    v *r = 0, *n, *l;
    r = mkFunc(a, env);
    FN_ARGEVAL_SET(r);
    FN_FUNCENV_SET(r);
    FN_ARGBIND_SET(r);
    return r;
}

/*********/
/* eval and apply */

v *
evlist(ctx *cctx, v *l, v *env)
{
    struct v *v = l, *r = 0;
    for (; v; v = cdr(v)) {
        r = eval(cctx, car(v),env);
    }
    return r;
}

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

v *
evbind(v *n, v *a, v *ev)
{
    if (ConsP(n)) {
        for (; n; n = cdr(n), a = cdr(a)) {
            ev = mkCons(mkCons(car(n), car(a)),ev);
        }
    } else {
        ev = mkCons(mkCons(n,a),ev);
    }
    return ev;
}

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
        fprintf(stderr, "Cannot apply to "); print(stderr, fn); fprintf(stderr, "\n"); 
        break;
    }
    return r;
}

/*******/
/* helpers */

v *
assq(v *e, v *env) 
{
    for (; env; env = cdr(env)) {
        if (e == caar(env)) return car(env);
    }
    return 0;
}

v *
unique(ctx *cctx, char *s, int l) 
{
    v *v; char *cmp;
    for (v = cctx->sym; v; v = cdr(v)) {
        cmp = str(car(v));
        if (strlen(cmp) == l && strncmp(cmp, s, l) == 0) {
            return car(v);
        }
    }
    if (!v) {
        cctx->sym = mkCons((v = mkSym(s,l)), cctx->sym);
    }
    return v;
}

v *
reverse(v *v)
{
    struct v *r = 0;
    if (v) {
        r = mkCons(car(v), 0);
        while(v = cdr(v)) {
            if ((v == DotSym))
                r = car(r);
            else
                r = mkCons(car(v),r);
        }
    }
    return r;
}

/*******/
/* main, setup and parsing */

int
main(int argc, char **argv) 
{
    ctx cctx; 
    FILE *f; char *m = 0, *lft; long l, val; int c, ad = 1, i; 
    v *cl = mkCons(0,0), *v, *n, *r;
    struct {
        struct v *(*fn)(ctx *, struct v *, struct v *); char *n; int argeval;
    } ops[] = {

#define defarith(fn,s) { fn##Func, #s, 1 },
        defarith(add,+)
        defarith(subtract,-)
        defarith(mul,*)
        defarith(div,/)
        defarith(mod,%)
        defarith(lt,<)
        defarith(lte,<=)
        defarith(eq,==)
        defarith(neq,!=)
        defarith(gte,>=)
        defarith(gt,>)
        
        { printFunc, "print", 1 },
        { letFunc, "let", 0 },
        { setqFunc, "setq", 0 },
        { defineFunc, "define", 0 },
        { lambdaFunc, "lambda", 0 },
        { ifFunc, "if", 0 },
        { whileFunc, "while", 0 },
        { 0, 0, 0 }
    };

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
    while((c = *m)) {
        switch (c) {
        case '.':
            break;
        case '(':
            cl = mkCons(0,cl);
            break;
        case ')':
            v = car(cl);
            cl = cdr(cl);
            r = reverse(v);
            rplaca(cl,mkCons(r,car(cl)));
            break;
        case '0': case '1': case '2': case '3': case '4': case '5':
        case '6': case '7': case '8': case '9': 
        num:
            val = strtol(m, &m, 0); m--;
            rplaca(cl,mkCons(mkInt(val),car(cl)));
            break;
        case ' ': case '\n': case '\t': 
            break;
        case '+': case '-': 
            if (!isdigit(m[1])) {
                goto sym;
            } else {
                goto num;
            }
            break;
        default:
#           define SPECIAL "<=!>"
            if (isalpha(c) || strchr(SPECIAL,c)) {
            sym:
                lft = m;
                while (isalnum(c = *++m) || strchr(SPECIAL,c));
                v = unique(&cctx, lft, m-lft); m--;
                rplaca(cl,mkCons(v,car(cl)));
            } else {
                error("Invalid char '%c'\n", c);
            }
            break;
        }
        m++;
    }
    v = reverse(car(cl));
    print(stderr, v ); printf("\n");
    
    for (i = 0; i < ((sizeof(ops) / sizeof(ops[0])) - 1); i++) {
        struct v *s;
        s = mkOp(ops[i].fn);
        cctx.env = mkCons(mkCons(unique(&cctx, ops[i].n, strlen(ops[i].n)),s),cctx.env);
        if (ops[i].argeval)
            OP_ARGEVAL_SET(s);
    }
    
    evlist(&cctx, v, cctx.env);
    
    return 0;
}

/*
Local Variables:
c-basic-offset:4
indent-tabs-mode:nil
End:
*/
