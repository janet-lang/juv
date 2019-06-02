(import build/uv :as uv)

(print "uv version: " uv/version)

(uv/enter-loop
  (let [f1 (yield (uv/fs/open "file1.txt" :wc))
        f2 (yield (uv/fs/open "file2.txt" :wc))
        f3 (yield (uv/fs/open "file3.txt" :wc))]
    (print f1 ", " f2 ", " f3)))
