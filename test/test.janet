(import build/uv :as uv)

(print "janet version: " janet/version)
(print "uv version: " uv/version)

#
# File System
#

(uv/enter-loop
  (let [f1 (yield (uv/fs/open "build/file1.txt" :wc 8r666))
        f2 (yield (uv/fs/open "build/file2.txt" :wc 8r666))
        f3 (yield (uv/fs/open "build/file3.txt" :wc 8r666))]
    (print f1 ", " f2 ", " f3)))

#
# Timer
#

(uv/enter-loop
  (let [f (coro (for i 0 15
                  (print "i = " i)
                  (yield)))
        t (uv/timer/new f)]
    (:start t 100 1000)))

(uv/enter-loop
  (var tt nil)
  (let [f (coro (for i 0 15
                  (print "i = " i)
                  (yield))
                (:repeat tt 10)
                (for i 0 150
                  (print "i = " i)
                  (yield)))
        t (uv/timer/new f)]
    (set tt t)
    (uv/timer/start t 100 1000)))
