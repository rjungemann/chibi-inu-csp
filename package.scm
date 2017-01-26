(package
  (maintainers "Roger Jungemann roger@thefifthricruit.com")
  (authors "Roger Jungemann roger@thefifthricruit.com")
  (version "0.0.1")
  (license mit)

  (library
    (name
      (chibi inu csp))
    (path "chibi/inu/csp.sld")
    (depends
      (chibi)))

  (library
    (chibi inu csp-test)
    (depends
      (chibi)
      (chibi test))
    (use-for test))

  (manual "chibi/inu/csp.html")
  (description "Lorem ipsum...")
  (test "run-tests.scm"))
