(declare-project
  :name "uv"
  :license "MIT"
  :author "Calvin Rose")

# Use pkg-config for now to libuv flags
(defn sh [s] (let [f (file/popen s) x (:read f :all)] (:close f) x))
(def lflags 
  (tuple/slice (string/split " " (sh "pkg-config libuv --libs --static")) 0 -2))

(declare-native
    :name "uv"
    :lflags [;default-lflags ;lflags]
    :embedded ["embed/entry.janet"]
    :source @["src/fs.c"
              "src/entry.c"
              "src/util.c"])
