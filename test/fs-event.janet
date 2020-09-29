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
    (let [callback (fn [handle path events]
                     (thread/send parent (string "Updated file: " path) 0))
          handle   (uv/fs-event/new (fiber/new (fn [&])))]
      (uv/fs-event/start handle callback "tmp" 0))))

(defn writer
  [parent]
  (thread/send parent "Writing first file..." 0)
  (spit "tmp/test.txt" "This is a test.")
  (thread/send parent "Writing second file..." 0)
  (spit "tmp/test_dir/test.txt" "This is a test."))

(def monitor-thread (thread/new monitor))
(def writer-thread (thread/new writer))

(print (thread/receive 5))
(print (thread/receive 5))
(print (thread/receive 5))
(print (thread/receive 5))

(rimraf "tmp")
