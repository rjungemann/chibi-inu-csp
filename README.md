(chibi inu csp)
===============

Purpose
-------

`(chibi inu csp)` allows for multiple Chibi Scheme VMs to run in multiple
threads, making use of multiple cores. The threads communicate using Go-like
channels. These are similar to queues in that a parent VM can spawn a child and
one can send messages along a channel, and the other can pull messages off at
their leisure.

For some background, many (most?) Scheme implementations use green threads
instead of native threads. This does allow for some degree of concurrency but
does not allow for multiple cores to be used by the same application from
within the same process.

Chibi Scheme does not allow for threading support out of the box either. But
Chibi Scheme has the advantages that many VMs can be run in the same process
and each VM is very lightweight. So it's no problem to have multiple VMs all
running in different threads. The trick is how to share data between the VMs.

Installation
------------

Until this library is ready to be hosted at http://snow-fort.org, you can
install it like so:

Make sure you have Ruby installed. **This is only needed for development
purposes (for running `rake`), and this dependency will go away soon.**

Make sure you have Chibi Scheme installed.

Run `rake install`.

To run the tests, run `rake test`.

To remove the library, run `rake remove`.

Usage
-----

From within a Chibi Scheme VM:

```scheme
(import (chibi)
        (chibi inu csp))

(define (println . args)
  (for-each display args)
  (newline))

(define channels
  ;; This spawns a thread containing a fresh Chibi VM.
  (csp-spawn
    ;; This is a special serializable lambda. Any variables available in the
    ;; parent scope **WILL NOT BE AVAILABLE IN HERE**.
    ;;
    ;; `parent` is a reference to a channel which can be used to push messages
    ;; from the child to the parent.
    ;;
    ;; `me` is a reference to a channel which the child can use to listen for
    ;; messages from the parent.
    ;;
    (csp-lambda (parent me)
      (println "Child started!")

      ;; Sleep for one second. You can also use `usleep` to specify sleep time
      ;; in nanoseconds.
      (csp-sleep 1)

      (println "Popping first item... " (csp-cahnnel-pop-non-block me))
      (println "Popping second item... " (csp-cahnnel-pop-non-block me)))))

;; From the parent, you can access its channel.
(define me (car channels))

;; And you can access the child channel.
(define child (cadr channels))

;; Let's push a couple items to the child.
(println "pushing first item...")
(csp-channel-push child "hiii")
(println "pushing second item...")
(csp-channel-push child "hiii2")

;; Finally, to block until the child is done, and then clean up after it:
(println "reclaiming")
(csp-reclaim child)

(println "done")
```

TODO
----

* Finish `csp-pop`, for blocking pop semantics.
* `csp-peek`
* I was pretty thorough with memory management and mutexes, but it needs another
  audit.
* Allow for an identical API for spawning children in other processes.
