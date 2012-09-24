
CODE (0,"flags",    0,0,0,0,FLAGS,         v*e=POP;PUSH(mkInt(e->flags)););
CODE (0,"tag",      0,0,0,0,TAG,           v*e=POP;PUSH(mkInt(e->tag)););
CODE (0,"func",     0,0,0,0,FUNC,          v*e=POP;PUSH(func(e)););
CODE (0,"fwordp",   0,0,0,0,FWORDP,        v*e=POP;PUSH(mkInt(fwordp(e))););

CODE (0,":STR",     0,0,0,0,CONST_STR,     PUSH(mkInt(STR)););
CODE (0,":INT",     0,0,0,0,CONST_INT,     PUSH(mkInt(INT)););
CODE (0,":OP",      0,0,0,0,CONST_OP,      PUSH(mkInt(OP)););
CODE (0,":FUNC",    0,0,0,0,CONST_FUNC,    PUSH(mkInt(FUNC)););
CODE (0,":CONS",    0,0,0,0,CONST_CONS,    PUSH(mkInt(CONS)););
CODE (0,":SYM",     0,0,0,0,CONST_SYM,     PUSH(mkInt(SYM)););
CODE (0,":ARGEVAL", 0,0,0,0,CONST_ARGVAL,  PUSH(mkInt(1<<0)););
CODE (0,":FUNCENV", 0,0,0,0,CONST_FUNCENV, PUSH(mkInt(1<<1)););
CODE (0,":ARGBIND", 0,0,0,0,CONST_ARGBIND, PUSH(mkInt(1<<2)););
CODE (0,":WORDVAL", 0,0,0,0,CONST_WORDVAL, PUSH(mkInt(1<<4)););

CODE (0,"execute-list",0,"elispop",0,0,ELISP,      v*a=POP;v*o=cadr(cctx->w);v*r=op(o)(&cctx->lctx, a, TOSE);PUSH(r););

CODE (0,"lf-ev",    0,0,0,0,LFEVAL,        lf_eval(lfctx););
CODE (0,"lf-ap-p",  0,0,0,0,LFAPPLYFLAGS,  lf_apply_p(lfctx););
CODE (0,"lassq",    0,0,0,0,LASSQ,         v*e1;v*e0=POP;e1=assq(e0,TOSE);PUSH(cdr(e1));/*printf("---: lassq:");print((ctx*)cctx,stdout,cdr(e1)); printf("\n");*/);
CODE (0,"mklambda", 0,0,0,0,MKLAMBDA,      v*e0=POP;v*e1=lambdaFunc(((ctx *)cctx),e0,TOSE);PUSH(e1););


CODE (0,">e",       0,0,0,0,ETOR,          EPUSH(POP));
CODE (0,"edrop",    0,0,0,0,EDROP,         EPOP);
CODE (0,"etop>",    0,0,0,0,ETOP,          PUSH(TOSE););
CODE (0,"genv>",    0,0,0,0,GENV,          PUSH(((ctx *)cctx)->env););

/*
Local Variables:
c-basic-offset:4
indent-tabs-mode:nil
End:
*/
