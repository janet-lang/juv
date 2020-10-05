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
(os/mkdir "tmp/test_dir")

(defn monitor
  [parent]
  (uv/enter-loop
    (var counter 1)
    (let [callback (fn [handle path events]
                     (print "File " counter " changed: " path)
                     (set counter (+ 1 counter))
                     (thread/send parent (string "Updated file: " path) 0))
          fiber    (fiber/new (fn []) :yi)
          handle   (uv/fs-event/new fiber)]
      (uv/fs-event/start handle callback "tmp" 0)
      (thread/send parent "Monitor started"))))

(defn writer
  [parent]
  (thread/send parent "Writing first file..." 0)
  (spit "tmp/test.txt" "This is a test.")
  (thread/send parent "Writing second file..." 0)
  (spit "tmp/test_dir/test.txt" "This is a test.")
  (thread/exit))

(def monitor-thread (thread/new monitor))
(def monitor-start-msg (thread/receive 5))
(print monitor-start-msg)

(def writer-thread (thread/new writer))

(loop [i :range [0 4]]
  (try
    (do
      (def msg1 (string "Receiving message " (+ 1 i)))
      (print msg1)
      (def msg2 (string "Message " (+ 1 i) ": " (thread/receive 5)))
      (print msg2))
    ([err] (print err))))

(rimraf "tmp")
