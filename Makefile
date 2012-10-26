all:	l f lf flat

l: l1.c l.c
	gcc -g l.c -o l

f: f1.c f.c l1.c f.h lf.h
	gcc -g f.c -o f

lf: f1.c lf.c l1.c f.h lf.c lf1.c lf.h
	gcc -g lf.c -o lf

flat: l1.c flat.c flat1.c
	gcc -g flat.c -o flat

test: l f lf
	@echo Fibonacci 20 in LISP 
	./l c.l
	@echo Fibonacci 20 in FORTH 
	./f c.f
	@echo Fibonacci 20 in FORTH executing LISP 
	./lf lf.f c.l
	@echo Fibonacci 20 in FLAT-LISP 
	./flat c.l

clean:
	rm -f l f lf
