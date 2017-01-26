(define-library (chibi inu csp-test)
  (export run-tests)
  (import (chibi)
          (chibi inu csp)
          (chibi test))
  (begin
    (define (run-tests)
      (test-begin "csp")

      (define child
        (csp-spawn (lambda (parent me)
          (display "Hello, world!")
          (newline)
          (csp-usleep 500000)
          (display "1 ")
          (display (csp-channel-pop-non-block me))
          (newline)
          (display "2 ")
          (display (csp-channel-pop-non-block me))
          (newline))))

      (csp-channel-push child "hiii")
      (csp-channel-push child "hiii2")

      (csp-sleep 2)
      (display "reclaiming")
      (newline)
      (csp-reclaim child)
      (display "done")
      (newline)

      (test-end))))
