(import
  (chibi inu csp)
  (rename (chibi inu csp-test)
          (run-tests csp-test-run-tests)))

(csp-test-run-tests)
