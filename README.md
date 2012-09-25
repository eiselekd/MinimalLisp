Note:
To limit the scope of this experiment the goal is to calculate 
"fibonacci 20" in each of the 3 steps. The LISP and FORTH interpreters
are minimal in a sense that they are not complete and dont use any
garbage collector or any optimization.

LISP evaluation in FORTH
========================

The goal is to implement a experimental LISP evaluator that recurses heap-based, 
meaning that you dont use the c-compiler generated stackframes for recursion into
eval(). 

Problem summary of c-based evaluation: 
--------------------------------------

To implement threading and continuation you need to make the implicit 
stack of the compiler explicit. This can be done by:

  Threading the LISP evaluator. 
    PThread: Complex
    Green-Threads: Minimal, however the fixed stack size limits recursion
      split-stack extension to GCC would overcome this problem, but then
      the it is not portable any more.
  Bytecode execution in VM.
    Bytecode execution doesnt use a stack but a flat switch loop, so here
    the problem doesnt exist. But it is complex and the seperate handling 
    of data and code doesnt feel like LISP.

Other solution:
---------------

I implement another way here which  which felt more "minimal". I implement 
a FORTH interpreter and implement the LISP evaluation in FORTH.  The recursion in 
LISP evaluation will be is entirely heap based as for the FORTH stacks are allocated 
on the heap.

Two interpreters are implemented. A minimal LISP (files l.c, l1.c), inspired and 
minimized from "Ian Piumarta's lysp.c". A minimal FORTH (files f.h, f.c, f1.c), 
inspired and minimized from MinForth and the "Jones FORTH tutorial".
Note that none of the interpreters is complete, instead it is complete enough to
satisfy the goal but still simple enough to be able to understand it. 

Step 1: 
-------
Fibonacci (file c.l) running in a minimal LISP interpreter (prog ./l). This interpreter 
uses c-recursion. 
$ make 
$ ./l c.l

Step 2:
------- 
Fibonacci (file c.f) running in a minimal FORTH interpreter (prog ./f). This interpreter
is non-recursive. 
$ make 
$ ./f c.f
The forth interpreter uses lists to model stacks and words, so it 
looks like LISP but the interpreter is FORTH like.

Step 3: 
-------
Fibonacci (file c.l) LISP evaluator written in FORTH (file lf.f) executing in a
extended minimal FORTH interpreter (prog lf). This interpreter is non-recursive.
$ make 
$ ./lf lf.f c.l
Here the c-functions that use eval() from the previous minimal LISP interpreter (file l1.c) 
are implemented in FORTH (file lf.f). The similarities between l1.c and lf.f 
should be obviouse.

Conclusion and todo
-------------------

Started a minimal solution in the end it turns out to be quite complex anyway.
Maybe the best solution is to implement all in c. The recursive path could be 
modeled inside an explicite state struct and the evalution inside a flat loop.



