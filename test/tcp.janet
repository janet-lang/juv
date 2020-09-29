(import build/uv :as uv)

#
# TCP
#

# Echo server
# (uv/enter-loop
#   (def server (uv/tcp/new))
#   (:bind server "0.0.0.0" 8120)
#   (:listen server (fn [&]
#                     (print "connected!")
#                     (def client (uv/tcp/new))
#                     (:accept server client)
#                     (:read-start client)
#                     (while true
#                       (def chunk (yield))
#                       (if chunk
#                         (do
#                           (:write stdout chunk)
#                           (yield (:write client "Howdy, pardner!\n"))
#                           (yield (:write client chunk)))
#                         (do
#                           (print "---done!---")
#                           (:read-stop client)
#                           (break)))))))

# Simple stuff
(uv/enter-loop
  (def conn (uv/tcp/connect (uv/tcp/new) "example.com" 80))
  (def res (yield))
  (print res))
