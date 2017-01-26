(define-library (chibi inu csp-test)
  (export run-tests)
  (import (chibi)
          (chibi inu csp)
          (chibi test))
  (begin
    (define (run-tests)
      (test-begin "csp")

      (csp-spawn (lambda ()
        (display "Hello, world!")))

      (test-end))))
