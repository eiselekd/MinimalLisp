/* Konrad Eisele <eiselekd@gmail.com, eiselekd@web.de>
 * Inspired by Richard W.M. Jones minmal Forth tutorial and MinForth by Andreas Kochenburger
 * Released under BSD-license */

#ifndef F1_H
#define F1_H

/* reuse lisp code */
struct fctx; struct v;
char *foptoa(struct fctx *, struct v *);
#define RESOLVE_FOP(c,a) foptoa((struct fctx*)c,a)
#include "l1.c"

/* a word is a list, preceded by a FOP and followed by a list of word references,
   much like an m-expression: i.e.
   : ++ + + ; 
   would be modelled as "++" => ( @DOCOL (@+) (@+) )
   The main execution loop is 
   while(fx) {
       fx = fop(fx)(&cctx);
   }
   Each FOP when called will return the next FOP to call. In this model the
   "instruction pointer" is a reference to the list's Cons, incrementing the 
   instruction pointer is cdr, a jump is a reference to a Cons.  
   Stacks are also modeled as lists.

   The main problem in following a FORTH program is maybe
   that there are:
   - 2 states the Interpreter loop can be in (compile/noncompile), there
     is an immediate flag that forces execution of a word.
   - 2 sources of FOP sequences: One originating from the Interpreter
     loop and the other from a Word having been called and executing.
   The compile state only affects the words coming out of the Interpreter 
   loop. Once a word is detected to be immediate by the Interpreter loop,
   it, and its subwords are executed unaffected by the compiler state. 
   A word that is marked immediate is meant to be part of the compiler.
   A word like "then" could be meant to be used to compile the compiler
   primitive word or meant to be used to compile the user word, if it is
   meant to be used to compile a user word it shouldnt be immediate as for
   it then would already execute when the compiler primitive word is compiled.
   Here, the '_' prefix forces a word to be non-immediate.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#ifdef DBG
#define FPRINTF(...) fprintf(__VA_ARGS__)
#define PRINT(...) print(__VA_ARGS__)
#else
#define FPRINTF(...) 
#define PRINT(...) 
#endif
#define STK_INCR 16
struct fctx;
v *pop(v **stk) { v *e = *stk; *stk = cdr(e); return car(e); }
#define  COMMA(v) comma(cctx, cctx->here,v)
#define  RPUSH(v) cctx->rstk = mkCons(v, cctx->rstk)
#define  PUSH(v)  cctx->stk = mkCons(v, cctx->stk)
#define  RPOP     pop(&cctx->rstk)
#define  POP      pop(&cctx->stk)
#define  TOS         car(cctx->stk)
#define  TOS_SET(v)  rplaca(cctx->stk, v)
#define  TORS        car(cctx->rstk)
#define  TORS_SET(v) rplaca(cctx->rstk, v)
typedef v *(*fn)(struct fctx *cctx) ;
#define MAX_F 16
#define MAX_B 128
typedef struct fctx {
    ctx lctx;
    FILE *f[MAX_F]; long fc;
    char b[MAX_B];
    int flags;
    v *stk, *rstk;
    v *ip, *w, *cw, *env;
    v *def, *here, *latest;
    v *peek; int peek_imm;
} fctx;
#define LCTX ((ctx *)cctx)
#define FCTX_COMPILE(cctx) (cctx->flags & (1<<0))
#define FCTX_COMPILE_SET(cctx,v) (cctx->flags = (cctx->flags & ~(1<<0)) | ((v&1)<<0))
#define NEXT(cctx) ((cctx)->w = car((cctx)->ip), (cctx)->ip = cdr((cctx)->ip), car((cctx)->w))
#define FDBG stdout
void dbgprint(fctx *cctx, char *n);
void dbgfdef(fctx *cctx, v *n, v *l);
v *next(fctx *cctx, int *imm) ;
v * _create(fctx *cctx, v *a);
v * _enddef(fctx *cctx, v *a);

v *fop_interpret(fctx *cctx);
v *comma(fctx *cctx, struct v *f, struct v *v) ;
int key(fctx *cctx);
v *word(fctx *cctx);
v *wordStr(fctx *cctx);

#ifndef NOFOUT
#define FPRINT(...) printf(__VA_ARGS__)
#else
#define FPRINT(...) 
#endif

#define CODE(in,n,ia0,a0,ia1,a1,f,c)             \
    v *                                          \
    fop_##f(fctx *cctx) {                        \
        c;                                       \
        dbgprint(cctx,#f);                       \
        return NEXT(cctx);                       \
    };                                           \
    v *g_##f;                                    \

#define ARITH(n,a0,a1,f,op)                      \
    v *                                          \
    fop_##f(fctx *cctx) {                        \
        long a, b;                               \
        a = tonum(POP); b = tonum(POP);          \
        PUSH(mkInt(op));                         \
        FPRINT("[41m%ld%s%ld=%ld",a,n,b,(long)(op)); FPRINT("[49m\n"); \
        dbgprint(cctx,#f);                                          \
        return NEXT(cctx);                       \
    };                                           \
    v *g_##f;                                    \

#include "f.h"
v *g_QUIT, *g_COLON, *g_SEMICOLON;

#define RECSTK 0
void
dbgstk(fctx *cctx, v *c) {
    FPRINTF(FDBG, "["); 
    for (; c && ConsP(c); c = cdr(c)) {
        if (!RECSTK && ConsP(car(c))) {
            FPRINTF(FDBG, "(.0x%p.) ",car(c));    
        } else {
            PRINT(LCTX, FDBG, car(c));  FPRINTF(FDBG, " ");
        }
    }
    FPRINTF(FDBG, "]"); fflush(FDBG);
}

void
dbgprint(fctx *cctx, char *n) {
    v *c, *e = cctx->env, *w, *cw = 0;
#ifdef DBG
    for (e = cctx->env;e; e = cdr(e)) {
        c = cdar(e);
        for (;c; c = cdr(c)) {
            if ((cctx)->ip == c) {
                cw = cdar(e);
                goto found;
            }
        }
    }
found:    
    FPRINTF(FDBG, "%-10s.%-10s:rstk: ",cw ? foptoa(cctx, cw) : "<>", n);
    dbgstk(cctx,cctx->rstk);
    FPRINTF(FDBG, " stk:");
    dbgstk(cctx,cctx->stk);
    FPRINTF(FDBG, "\n",n);
#endif
}


void
dbgfdef(fctx *cctx, v *n, v *l)  {
    v *c, *d; int f = 0, i;
    FPRINTF(FDBG,"fdef: %-10s: ",n ? str(n) : "<null>");
    if (l->tag != CONS || !CONS_WORD(l)) {
        FPRINTF(FDBG,"!n!"); /*PRINT(LCTX, FDBG, l); */
    } else {
        /*if (FOP_IMMEDIATE(l))
          FPRINTF(FDBG,"!i!");*/
    }
    FPRINTF(FDBG,"( ");
    PRINT((ctx*)cctx, FDBG, car(l)); FPRINTF(FDBG," ");
    for (c = cdr(l); c && ConsP(c); c = cdr(c)) {
        if (fwordp(car(c))) {
            if (caar(c) == car(g_DOCOL)) {
                FPRINTF(FDBG,"([42m%s",foptoa(cctx,car(c)));FPRINTF(FDBG,"[49m)");
            } else {
                FPRINTF(FDBG,"([42m"); PRINT((ctx*)cctx, FDBG, caar(c)); FPRINTF(FDBG,"[49m)");
            }
        } else {
            for (i = 0,f = 0, d = l; d && ConsP(d); d = cdr(d), i++) {
                if (car(c) == d) {
                    f = 1;
                    FPRINTF(FDBG,"@[%d]",i); 
                    break;
                }
            }
            if (!f) {
                PRINT((ctx*)cctx, FDBG, car(c)); FPRINTF(FDBG," ");
            }
        }
        FPRINTF(FDBG," ");
    }
    FPRINTF(FDBG,")");
    FPRINTF(FDBG,"\n");
}

v *
fdef(fctx *cctx, char *n, int cnt, ...) 
{
    va_list ap; int i, j; unsigned long b;
    va_start(ap, cnt);
    v *e, *l = mkCons(car(g_DOCOL), 0), *u;
    for (i = 0; i < cnt; i++) {
        e = (v *)va_arg(ap, v *);
        if ((b = ((unsigned long)e)-1) < 128) {
            for (j = 0, e = l; j < b; j++) {
                e = cdr(e);
            }
        }
        l = mkCons(e, l);
    }
    l = nreverse(mkCons(g_EXIT, l));
    u = unique(&cctx->lctx, n, strlen(n));
    cctx->env = mkCons(e = mkCons(u, l),cctx->env);
    va_end(ap);
    FOP_IMMEDIATE_VAL(e,1);
    CONS_WORD_VAL(l,1);
    dbgfdef(cctx, u, l);
    return l;
}


void
f_init(fctx *cctx) 
{
    l_init(&cctx->lctx) ;
    
#undef ARITH
#undef CODE
#define ARITH(n,a0,a1,f,c) CODE(0,n,0,a0,0,a1,f,c)
#define CODE(in,n,ia0,a0,ia1,a1,f,c) {                       \
        v *e, *ei;                                           \
        g_##f = mkCons(mkFop((fopfn)fop_##f), 0);            \
        CONS_WORD_VAL(g_##f,1);                              \
        if (a0) { cctx->env = mkCons(ei=mkCons(unique(&cctx->lctx, a0, strlen(a0)), e = mkCons(mkFop((fopfn)fop_##f), 0)),cctx->env); FOP_IMMEDIATE_VAL(ei, ia0); CONS_WORD_VAL(e,1); } \
        if (a1) { cctx->env = mkCons(ei=mkCons(unique(&cctx->lctx, a1, strlen(a1)), e = mkCons(mkFop((fopfn)fop_##f), 0)),cctx->env); FOP_IMMEDIATE_VAL(ei, ia1); CONS_WORD_VAL(e,1); } \
        cctx->env = mkCons(ei=mkCons(unique(&cctx->lctx, n,  strlen(n)),  g_##f),cctx->env); FOP_IMMEDIATE_VAL(ei, in); \
    }
    
    #include "f.h"
    
    g_COLON     = fdef(cctx, ":",    3, g_WORD,  g_CREATE, g_RBRAC );
    g_SEMICOLON = fdef(cctx, ";",    2, g_LBRAC, g_ENDDEF );
    g_QUIT      = fdef(cctx, "QUIT", 6, g_LIT,   mkInt(0), g_RSPSTORE, g_INTERPRET, g_BRANCH, (long)2 );
    
}

int 
key(fctx *cctx)
{
    int c = EOF;
    if (cctx->fc >= 0) {
        while ((c = getc(cctx->f[cctx->fc])) == EOF) {
            fclose(cctx->f[cctx->fc--]);
            if (cctx->fc < 0)
                return c;
        }
    }
    return c;
}

v *
word(fctx *cctx)
{
    int c, l = 0, bc = 0; char *n = 0; v *s;
retry:
    while(strchr(" \n\t",(c = key(cctx))));
    if (c == '(') { /* eat comments */
        cctx->b[l++] = c; 
        if (strchr(" \t",(c = key(cctx)))) {
            l--; bc = 1;
            do { 
                c = key(cctx);
                switch(c) {
                case ')': bc--; break;
                case '(': bc++; break;
                };
            } while(!(bc <= 0 || c == EOF));
            goto retry;
        };
    } else if (c == '\\') {
        do {
            c = key(cctx);
        } while(!(c == '\n' || c == EOF));
        goto retry;
    } 
    do {
        cctx->b[l++] = c; 
        c = key(cctx);
    } while(!(c == ' ' || c == '\n' || c == EOF));
    s = unique(&cctx->lctx, cctx->b, l);
    return s;
}

v *wordStr(fctx *cctx) { v *s = word(cctx); return s; }

char *foptoa(struct fctx *cctx, struct v *v) {
    struct v *c, *e, *n;
    for (c = cctx->env; c; c = cdr(c)) {
        n = caar(c); e = cdar(c);
        if ((v == (fwordp(v) ? e : car(e))))
            return str(n);
    }
    return "<undef>";
}

v *
find(fctx *cctx, v *s)
{
    v *e = 0;
    if ((e = assq(s, cctx->env))) {
        return (e);
    }
    return 0;
}

v *
next(fctx *cctx, int *imm) 
{
    char *b;
    v *w= 0, *s, *s2, *nm; char *e, *n; long v; int l;
    if ((s = cctx->peek)) {
        if(imm)
            *imm = cctx->peek_imm; 
        cctx->peek = 0;
        return s;
    }
    nm = s = word(cctx);
    n = str(s); l = strlen(n); 
    if (!(w = find(cctx, s))) {
        if (n[0] == '_' && l > 1) { /* search for maybe immediate version _<name> but return non-imm */
            if ((s2 = unique(&cctx->lctx, n+1, l-1))) {
                if ((w = find(cctx, s2))) {
                    *imm = 0;
                    goto doword;
                }
            }
        }
        if ((strchr("+-",n[0]) && isdigit(n[1])) ||
            isdigit(n[0])) {
            v = (long)strtoul(n, &e, 0);
            if (n + l == e) {
                s = mkInt(v);
            }
        }
        FPRINTF(FDBG, ">"); PRINT(LCTX,FDBG, s); FPRINTF(FDBG, "\n");
        if (imm)
            *imm = 0;
    } else {
        if (imm)
            *imm = FOP_IMMEDIATE(w);
        doword:
        s = cdr(w);
        FPRINTF(FDBG, ">["); PRINT(LCTX,FDBG, nm); FPRINTF(FDBG, "]\n");
    }
    return s;
}

v *
comma(fctx *cctx, struct v *f, struct v *v) 
{
    struct v *e, *l = cdr(f); 
    rplaca(l,v);
    rplacd(f,e=mkCons(0,cdr(f)));
    FPRINTF(FDBG, "%-10s: @ ",""); 
    dbgfdef(cctx, car(f), cdr(f)); FPRINTF(FDBG, "\n");
    return l;
}

v *
fop_interpret(fctx *cctx)
{
    int lit, imm = 0; long val;
    struct v *v = next(cctx, &imm);
    if (v) {
        switch(v->tag) {
            
        case CONS:
            
            if ((cctx->w = v)) {
                if (imm || !FCTX_COMPILE(cctx)) {
                    return car(cctx->w);
                } else { /* compiling */
                    comma(cctx, cctx->here, v);
                }
            }
            return NEXT(cctx);
            break;
            
        case STR: case INT:
            
            if (FCTX_COMPILE(cctx)) {
                comma(cctx, cctx->here, g_LIT);
                comma(cctx, cctx->here, v);
            } else {
                PUSH(v);
            }
            return NEXT(cctx);
            break;
        
        case SYM:
            if (FCTX_COMPILE(cctx)) {
                comma(cctx, cctx->here, g_SYM);
                comma(cctx, cctx->here, v);
                return NEXT(cctx);
            }
            break;
            
        }
    }
    dbgprint(cctx," * ");

    if (cctx->fc >= 0) 
        return NEXT(cctx);
    else 
        return 0;
    
}

#endif

/*
Local Variables:
c-basic-offset:4
indent-tabs-mode:nil
End:
*/
