(import build/uv :as uv)

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
  (let [f (coro (for i 0 5
                  (print "i = " i)
                  (yield))
                (:repeat tt 10)
                (for i 0 15
                  (print "i = " i)
                  (yield)))
        t (uv/timer/new f)]
    (set tt t)
    (uv/timer/start t 50 1000)))
