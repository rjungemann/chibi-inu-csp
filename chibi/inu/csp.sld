(define-library (chibi inu csp)
  (import (chibi))
  (include-shared "csp")
  (include "csp-native.scm")
  (export csp-spawn
          csp-reclaim
          csp-channel-push
          csp-channel-pop-non-block
          csp-usleep
          csp-sleep
  )
)
