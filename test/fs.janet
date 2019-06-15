(import build/uv :as uv)

#
# File System
#

(uv/enter-loop
  (let [f1 (yield (uv/fs/open "build/file1.txt" :wc 8r666))
        f2 (yield (uv/fs/open "build/file2.txt" :wc 8r666))
        f3 (yield (uv/fs/open "build/file3.txt" :wc 8r666))]
    (print f1 ", " f2 ", " f3)))
