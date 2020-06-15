(declare-project
  :name "uv"
  :license "MIT"
  :author "Calvin Rose")

(def lflags
  (case (os/which)
    :windows @["advapi32.lib"
               "iphlpapi.lib"
               "psapi.lib"
               "shell32.lib"
               "user32.lib"
               "userenv.lib"
               "ws2_32.lib"]
    @["-lpthread"]))

(def is-win (= :windows (os/which)))

(declare-native
  :name "uv"
  :lflags [;default-lflags ;lflags]
  :cflags [;default-cflags "-Ilibuv/include" "-Ilibuv/src" ;(if is-win [] ["-Wno-unused-parameter"])]
  :defines (case (os/which)
             :windows {"_WIN32_WINNT" "0x0600"
                       "_GNU_SOURCE" true}
             :macos {"_GNU_SOURCE" true}
             # default
             {"_POSIX_C_SOURCE" "200112"
              "_GNU_SOURCE" true})

    :embedded ["embed/entry.janet"]
    :headers @["src/entry.h"
               "src/handle.h"
               "src/stream.h"]

    :source @["libuv/src/fs-poll.c"
              "libuv/src/idna.c"
              "libuv/src/inet.c"
              "libuv/src/random.c"
              "libuv/src/strscpy.c"
              "libuv/src/threadpool.c"
              "libuv/src/timer.c"
              "libuv/src/uv-data-getter-setters.c"
              "libuv/src/uv-common.c"
              "libuv/src/version.c"

              # windows vs. non-windows
              ;(if (= (os/which) :windows)

                 # windows
                 ["libuv/src/win/async.c"
                  "libuv/src/win/core.c"
                  "libuv/src/win/detect-wakeup.c"
                  "libuv/src/win/dl.c"
                  "libuv/src/win/error.c"
                  "libuv/src/win/fs.c"
                  "libuv/src/win/fs-event.c"
                  "libuv/src/win/getaddrinfo.c"
                  "libuv/src/win/getnameinfo.c"
                  "libuv/src/win/handle.c"
                  "libuv/src/win/loop-watcher.c"
                  "libuv/src/win/pipe.c"
                  "libuv/src/win/thread.c"
                  "libuv/src/win/poll.c"
                  "libuv/src/win/process.c"
                  "libuv/src/win/process-stdio.c"
                  "libuv/src/win/signal.c"
                  "libuv/src/win/snprintf.c"
                  "libuv/src/win/stream.c"
                  "libuv/src/win/tcp.c"
                  "libuv/src/win/tty.c"
                  "libuv/src/win/udp.c"
                  "libuv/src/win/util.c"
                  "libuv/src/win/winapi.c"
                  "libuv/src/win/winsock.c"]
                 
                 # not windows (posix)
                 ["libuv/src/unix/async.c"
                  "libuv/src/unix/core.c"
                  "libuv/src/unix/dl.c"
                  "libuv/src/unix/fs.c"
                  "libuv/src/unix/getaddrinfo.c"
                  "libuv/src/unix/getnameinfo.c"
                  "libuv/src/unix/loop.c"
                  "libuv/src/unix/loop-watcher.c"
                  "libuv/src/unix/pipe.c"
                  "libuv/src/unix/poll.c"
                  "libuv/src/unix/process.c"
                  "libuv/src/unix/proctitle.c"
                  "libuv/src/unix/random-devurandom.c"
                  "libuv/src/unix/signal.c"
                  "libuv/src/unix/stream.c"
                  "libuv/src/unix/tcp.c"
                  "libuv/src/unix/thread.c"
                  "libuv/src/unix/tty.c"
                  "libuv/src/unix/udp.c"])

              # macos specific
              ;(if (= (os/which) :macos)
                ["libuv/src/unix/bsd-ifaddrs.c"
                 "libuv/src/unix/darwin.c"
                 "libuv/src/unix/fsevents.c"
                 "libuv/src/unix/kqueue.c"
                 "libuv/src/unix/darwin-proctitle.c"
                 "libuv/src/unix/random-getentropy.c"]
                [])

              # linux specific
              ;(if (= (os/which) :linux)
                 ["libuv/src/unix/linux-core.c"
                  "libuv/src/unix/linux-inotify.c"
                  "libuv/src/unix/linux-syscalls.c"
                  "libuv/src/unix/procfs-exepath.c"
                  "libuv/src/unix/random-getrandom.c"
                  "libuv/src/unix/random-sysctl.c"
                  "libuv/src/unix/sysinfo-loadavg.c"]
                 [])
              
              # Janet wrappers
              "src/entry.c"
              "src/fs.c"
              "src/handle.c"
              "src/stream.c"
              "src/tcp.c"
              "src/timer.c"
              "src/util.c"])
