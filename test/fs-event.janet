(import build/uv :as uv)

(defn- rimraf
  [path]
  (if-let [m (os/stat path :mode)]
    (if (= m :directory)
      (do
        (each subpath (os/dir path) (rimraf (string path "/" subpath)))
        (os/rmdir path))
      (os/rm path))))

#
# FS Event Monitor
#

(os/mkdir "tmp")

(defn monitor
  [parent]
  (uv/enter-loop
    (let [f (coro (while true
                    (thread/send parent "Event received")
                    (yield)))
          t (uv/fs-event/new f)]
      (uv/fs-event/start t "tmp" 4))))

(defn writer
  [parent]
  (thread/send parent "Writing first file...")
  (spit "tmp/test.txt" "This is a test."))

(def monitor-thread (thread/new monitor))
(def writer-thread (thread/new writer))

(print (thread/receive 5))
(print (thread/receive 5))

(rimraf "tmp")
