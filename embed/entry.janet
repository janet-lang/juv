(defmacro enter-loop
  [& body]
  (with-syms [c]
    ~(let [,c (coro ,;body)]
       (resume ,c)
       (,run))))
