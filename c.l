(define nfibs
  (lambda (n)
    (if (< n 2)
	n
	(+ (nfibs (- n 1)) (nfibs (- n 2))))))

(nfibs 20)
