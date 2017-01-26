(define-library (chibi inu csp-test)
  (export run-tests)
  (import (chibi)
          (chibi inu csp)
          (chibi test))
  (begin
    (define (run-tests)
      (test-begin "csp")

      (define channels
        (csp-spawn (csp-lambda (parent me)
          (display "Hello, world!")
          (newline)
          (csp-usleep 500000)
          (display "popping first item... ")
          (display (csp-channel-pop-non-block me))
          (newline)
          (display "popping second item... ")
          (display (csp-channel-pop-non-block me))
          (newline))))

      (define me (car channels))
      (define child (cadr channels))

      (display "pushing first item...")
      (newline)
      (csp-channel-push child "hiii")
      (display "pushing second item...")
      (newline)
      (csp-channel-push child "hiii2")

      (display "reclaiming")
      (newline)
      (csp-reclaim child)
      (display "done")
      (newline)

      (test-end))))
