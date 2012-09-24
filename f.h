/* atomic operators, lots of redundant or unneeded operators... */
CODE (0,"nil",      0,">@markbeg",0,0,NIL,   PUSH(nil););
CODE (0,"car",      0,"@",0,0,CAR,           v*e=POP;PUSH(car(e)););    
CODE (0,"caar",     0,0,0,0,CAAR,            v*e=POP;PUSH(caar(e)););    
CODE (0,"cadr",     0,0,0,0,CADR,            v*e=POP;PUSH(cadr(e)););    
CODE (0,"caddr",    0,0,0,0,CADDR,           v*e=POP;PUSH(caddr(e)););   
CODE (0,"dcar",     0,0,0,0,DCAR,            PUSH(car(TOS)););           
CODE (0,"dcaar",    0,0,0,0,DCAAR,           PUSH(caar(TOS)););          
CODE (0,"dcadr",    0,0,0,0,DCADR,           PUSH(cadr(TOS)););          
CODE (0,"dcaddr",   0,0,0,0,DCADDR,          PUSH(caddr(TOS)););         
CODE (0,"cdr",      0,0,0,0,CDR,             v*e=POP;PUSH(cdr(e)););     
CODE (0,"dcdr",     0,0,0,0,DCDR,            PUSH(cdr(TOS)););           
CODE (0,"nfor",     0,0,0,0,NFOR,            int i=tonum(POP);v*p=nth(cctx->stk,(i));PUSH(caar(p));rplaca(p,cdar(p)););  
CODE (0,"n0for",    0,0,0,0,N0FOR,                            v*p=nth(cctx->stk,(0));PUSH(caar(p));rplaca(p,cdar(p)););  
CODE (0,"n1for",    0,0,0,0,N1FOR,                            v*p=nth(cctx->stk,(1));PUSH(caar(p));rplaca(p,cdar(p)););  
CODE (0,"n2for",    0,0,0,0,N2FOR,                            v*p=nth(cctx->stk,(2));PUSH(caar(p));rplaca(p,cdar(p)););  


CODE (0,"rplaca",   0,"!",0,0,RPLACA,        v*e=POP;rplaca(e,POP););                       
CODE (0,"rplacd",   0,0,0,0,RPLACD,          v*e=POP;rplacd(e,POP););                       
CODE (0,"cons",     0,0,0,0,CONS,            v*e0=POP;v*e1=POP;PUSH(mkCons(e0,e1)););       
CODE (0,"?nil",     0,0,0,0,PNIL,            v*e0=POP;PUSH(mkInt(e0?1:0)););                
CODE (0,"?true",    0,0,0,0,PTRUE,           v*e0=POP;PUSH(mkInt(tonum(e0)?1:0)););         
CODE (0,"?fword",   0,0,0,0,PFWORD,          v*e0=POP;PUSH(mkInt(fwordp(e0)?1:0)););         
CODE (0,"nth",      0,0,0,0,NTH,             v*e=POP;PUSH(car(nth(cctx->stk,tonum(e))));)   
CODE (0,"npush",    0,0,0,0,NPUSH,           int i=tonum(POP);v*e=POP;if(i==0){PUSH(e);}else{v*p=nth(cctx->stk,(i-1));rplacd(p,mkCons(e,cdr(p)));};) 
CODE (0,"nset",     0,0,0,0,NSET,            int i=tonum(POP);v*e=POP;v*p=nth(cctx->stk,(i));rplaca(p,e);)  
CODE (0,"n1-set",   0,0,0,0,N1SET,                            v*e=POP;v*p=nth(cctx->stk,1);  rplaca(p,e);)  
CODE (0,"second",   0,"1nth",0,0,SECOND,     PUSH(car(nth(cctx->stk,1)));)   
CODE (0,"third",    0,"2nth",0,0,THIRD,      PUSH(car(nth(cctx->stk,2)));)   
CODE (0,"forth",    0,"3nth",0,0,FORTH,      PUSH(car(nth(cctx->stk,3)));)   

CODE (0,"swap",     0,0,0,0,SWAP,            v*e0=POP;v*e1=POP;PUSH(e0);PUSH(e1);)
CODE (0,"rot",      0,0,0,0,ROT,             v*e =cddr(cctx->stk);rplacd(cdr(cctx->stk),cdr(e));PUSH(car(e));)
CODE (0,"drop",     0,0,0,0,DROP,            POP)
CODE (0,"2drop",    0,0,0,0,DROP2,           POP;POP;)
CODE (0,"3drop",    0,0,0,0,DROP3,           POP;POP;POP;)
CODE (0,"nthdrop",  0,0,0,0,NTHDROP,         int i=tonum(POP);if (i==0) {POP; } else { v*e=nth(cctx->stk,i-1);rplacd(e,cdr(cdr(e)));})
CODE (0,"over",     0,0,0,0,OVER,            PUSH(cadr(cctx->stk));)
CODE (0,"dup",      0,"n0-dup",0,0,dup,      PUSH(car(cctx->stk));)
CODE (0,"n1-dup",   0,0,0,0,n1dup,           PUSH(car(cdr(cctx->stk)));)
CODE (0,"n2-dup",   0,0,0,0,n2dup,           PUSH(car(cddr(cctx->stk)));)
CODE (0,"2dup",     0,0,0,0,dup2,            PUSH(cadr(cctx->stk));PUSH(cadr(cctx->stk));)

CODE (0,"RSPSTORE", 0,"RSP!",0,0,RSPSTORE,   cctx->rstk = POP);
CODE (0,"RSP@",     0,0,0,0,RSPFETCH,        PUSH(cctx->rstk));
CODE (0,"DSP!",     0,0,0,0,DSPSTORE,        cctx->stk = (POP));
CODE (0,"DSP@",     0,0,0,0,DSPFETCH,        PUSH(cctx->stk));
CODE (0,"r@",       0,0,0,0,RDCP,            PUSH(car(cctx->rstk)));
CODE (0,"2r@",      0,0,0,0,RDCP2,           PUSH(cadr(cctx->rstk));PUSH(car(cctx->rstk)));
CODE (0,">r",       0,0,0,0,TOR,             RPUSH(POP));
CODE (0,"2>r",      0,0,0,0,TOR2,            v*e0=POP;v*e1=POP;RPUSH(e1);RPUSH(e0););
CODE (0,"r>",       0,0,0,0,FROMR,           PUSH(RPOP));
CODE (0,"2r>",      0,0,0,0,FROMR2,          v*e0=RPOP;v*e1=RPOP;PUSH(e1);PUSH(e0););
CODE (0,"rrot",     0,0,0,0,RROT,            v*e =cddr(cctx->rstk);rplacd(cdr(cctx->rstk),cdr(e));RPUSH(car(e));)
CODE (0,"rpick",    0,0,0,0,RPICK,           PUSH(car(nth(cctx->rstk,tonum(POP)))));

CODE (0,"RDROP",    0,0,0,0,RDROP,           RPOP);
CODE (0,"DOCOL",    0,0,0,0,DOCOL,           RPUSH(cctx->ip); cctx->cw=cctx->w; cctx->ip = cdr(cctx->w););
CODE (0,"EXIT",     0,0,0,0,EXIT,            cctx->ip = RPOP;);

CODE (0,"BRANCH",   0,0,0,0,BRANCH,          cctx->ip = car(cctx->ip));
CODE (0,"zBRANCH",  0,0,0,0,ZBRANCH,         if (!tonum(POP)) { FPRINTF(FDBG,"zBRANCH-tk:\n"); return fop_BRANCH(cctx); }; cctx->ip=cdr(cctx->ip););
CODE (0,"VBRANCH",  0,0,0,0,VBRANCH,         long j0 = tonum(TORS); long j1 = tonum(cadr(cctx->rstk)); long i=tonum(POP);j0+=i;TORS_SET(mkInt(j0));if (j0 >= j1) { return fop_BRANCH(cctx); }; cctx->ip=cdr(cctx->ip););
CODE (0,"EXECUTE",  0,0,0,0,EXECUTE,         cctx->w=POP; dbgprint(cctx,"EXE"); return car(cctx->w););

/* create a assq entry that points to cctx->def. cctx->def is the tail of the list during compilation. sreverse() in ENDDEF will move it to the head. cctx->here, cctx->latest point to a dummy assq during compilation. */
CODE (0,"CREATE",   0,0,0,0,CREATE,          v*n=POP;v*e;cctx->here = cctx->latest = mkCons(n,mkCons(0,e=mkCons(car(g_DOCOL),0))); cctx->env=mkCons(cctx->def=mkCons(n,e),cctx->env););
CODE (0,"ENDDEF",   0,0,0,0,ENDDEF,          v*e;e=cdr(cctx->here);rplaca((e),g_EXIT);/* this will revert cctx->def: */e=sreverse(e); CONS_WORD_VAL(e,1); FPRINTF(FDBG,"ENDDEF:"); dbgfdef(cctx,car(cctx->def),cdr(cctx->def)); cctx->latest = cctx->def; cctx->here = cctx->def = 0;);
CODE (0,"LIT",      0,0,0,0,LIT,             PUSH(car(cctx->ip)); cctx->ip=cdr(cctx->ip););
CODE (0,"SYM",      0,0,0,0,SYM,             v*s;v *e=car(cctx->ip);cctx->ip=cdr(cctx->ip); if((s = assq(e, cctx->env))) { dbgprint(cctx,"SYM"); cctx->w=cdr(s); return car(cctx->w); } else { FPRINTF(FDBG,"SYM:Symbol not found\n"); };);
CODE (0,"LITERAL",  0,0,0,0,LITERAL,         COMMA(g_LIT); COMMA(POP););
CODE (1,"QUOTE",    1,"'",0,0,QUOTE,         COMMA(g_LIT); COMMA(next(cctx,0)););
CODE (0,"next",     0,0,0,0,NEXT,            PUSH(next(cctx,0)););
CODE (0,"peek-next",0,0,0,0,PEEKNEXT,        PUSH(cctx->peek=next(cctx,&cctx->peek_imm)););
CODE (0,"COMMA",    0,",",0,"<resolve",COMMA,comma(cctx, cctx->here, POP));
CODE (0,"?,",       0,0,0,0,QCOMMA,          v*e=POP;switch(e->tag){case STR:case INT: COMMA(g_LIT); break; case SYM: COMMA(g_SYM); break; }; comma(cctx, cctx->here, e));
CODE (0,">mark",    0,0,0,0,MARK,            PUSH(comma(cctx,cctx->here,0)););
CODE (0,">resolve", 1,"then",0,0,RESOLVE,    v*e=POP;rplaca(e,cdr(cctx->here));dbgfdef(cctx, car(cctx->here), cdr(cctx->here)););
CODE (0,">@mark",   0,0,0,0,AMARK,           TOS_SET(mkCons(comma(cctx,cctx->here,0),TOS)););
CODE (0,">@resolve",0,0,0,0,ARESOLVE,        v*e;for(e=POP;e;e=cdr(e)){rplaca(car(e),cdr(cctx->here));};);
CODE (1,"?do",      1,"here",0,"<mark",DO,    PUSH(cdr(cctx->here));)
CODE (0,"c[",       0,0,0,0,LBRAC,           FCTX_COMPILE_SET(cctx,0);FPRINTF(FDBG, "[27m");); /* switch from immediate to compile, meaningful only inside interpreter loop, only used by ":" */
CODE (0,"]c",       0,0,0,0,RBRAC,           FCTX_COMPILE_SET(cctx,1);FPRINTF(FDBG, "[7m"););  /* switch from immediate to compile, meaningful only inside interpreter loop, only used by ";" */
CODE (0,"]",        0,0,0,0,QRBRAC,          v*e;); /* scaning border used by "[" */
CODE (0,"[",        0,0,0,0,QLBRAC,          while(car(cctx->ip)!=g_QRBRAC) {COMMA(car(cctx->ip)); cctx->ip=cdr(cctx->ip);};); /* "[" "]" is used in compiler words to specify a block of words to be copied into the compiled word */ 
CODE (1,"IMMEDIATE",0,0,0,0,IMMEDIATE,       FOP_IMMEDIATE_VAL(cctx->def,FOP_IMMEDIATE(cctx->def) ? 0 : 1);); /* set immediate flag */
 
CODE (0,"INTERPRET",0,0,0,0,INTERPRET,       return fop_interpret(cctx););
CODE (0,"KEY",      0,0,0,0,KEY,             PUSH(mkInt(key(cctx))););
CODE (0,"WORD",     0,0,0,0,WORD,            PUSH(wordStr(cctx)););

CODE (0,".",        0,0,0,0,POINT,           v*e = POP; print(LCTX, stdout, e););
CODE (0,"end",      0,0,0,0,END,             exit(0););

CODE (0,"ne",       0,0,0,0,NE,              v*e0=POP;v*e1=POP;PUSH(mkInt(!eq(e0,e1))););
CODE (0,"eq",       0,0,0,0,EQ,              v*e0=POP;v*e1=POP;PUSH(mkInt(eq(e0,e1))););

/* vars */
CODE (0,"RZ",       0,0,0,0,RZ,              PUSH(0));

ARITH ("+",         0,0,ADD,                 a +  b);
ARITH ("-",         0,0,SUB,                 a -  b);
ARITH ("*",         0,0,MUL,                 a *  b);
ARITH ("<<",        0,0,LSH,                 a << b);
ARITH (">>",        0,0,RSH,                 a >> b);
ARITH ("&",         0,0,BAND,                a &  b);
ARITH ("|",         0,0,BOR,                 a |  b);
ARITH ("^",         0,0,BXOR,                a ^  b);
ARITH ("&&",        0,0,LAND,                a && b);
ARITH ("||",        0,0,LOR,                 a || b);
ARITH ("=",         "==",0,EQU,              (a == b) ? 1 : 0);
ARITH ("<>",        "!=",0,NEQU,             (a != b) ? 1 : 0);
ARITH ("<",         0,0,LT,                  (a <  b) ? 1 : 0);
ARITH (">",         0,0,GT,                  (a >  b) ? 1 : 0);
ARITH ("<=",        0,0,LE,                  (a <  b) ? 1 : 0);
ARITH (">=",        0,0,GE,                  (a >  b) ? 1 : 0);



/*
Local Variables:
c-basic-offset:4
indent-tabs-mode:nil
End:
*/
