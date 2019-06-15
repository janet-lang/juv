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
    :cflags [;default-cflags "-g"]
    :lflags [;default-lflags ;lflags]
    :defines {"_POSIX_C_SOURCE" "200112"}
    :embedded ["embed/entry.janet"]
    :headers @["src/entry.h"
               "src/handle.h"
               "src/stream.h"]
    :source @["src/entry.c"
              "src/fs.c"
              "src/handle.c"
              "src/stream.c"
              "src/tcp.c"
              "src/timer.c"
              "src/util.c"])
