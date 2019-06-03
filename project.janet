(declare-project
  :name "uv"
  :license "MIT"
  :author "Calvin Rose")

# Use pkg-config for now to get libuv flags
(defn sh [s] (let [f (file/popen s) x (:read f :all)] (:close f) x))
(def pkgout (sh "pkg-config libuv --libs --static"))
(def lflags (tuple/slice (string/split " " pkgout) 0 -2))

(declare-native
    :name "uv"
    :lflags [;default-lflags ;lflags]
    :embedded ["embed/entry.janet"]
    :source @["src/entry.c"
              "src/fs.c"
              "src/timer.c"
              "src/util.c"])
