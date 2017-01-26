(define-syntax csp-lambda
  (syntax-rules ()
    ((_ rest ...)
     (quote (lambda rest ...)))))

(define (csp-spawn csp-callback)
  (define out (open-output-string))
  (write csp-callback out)
  (csp-spawn-original (get-output-string out)))
