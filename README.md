# Janet libuv bindings

Bindings to the [libuv](https://libuv.org/) library for asynchronous IO.
Not yet ready for much of anything, still a working prototype. Kept in sync
with the latest Janet from master, and is not guaranteed to work with anything but.

## Installation

```
[sudo] jpm install https://github.com/janet-lang/juv.git
```

## Building

Make sure you have cloned the libuv submodule as well. You can use `git submodule update --init --recursive` to do this.

```
jpm build
```

## Testing

See the `test` directory for test scripts and programs.

```
jpm test
```

## Design

`juv` puts a thin abstraction over libuv to be completely fiber based instead of
callback based. This means all libuv calls that would normally take a callback
instead take a fiber, either explicitly or by capturing the current fiber. When
the asynchronous event happens and the C callback is called, the fiber is resumed
with the event's payload.

To suspend a fiber until some next event, call a `juv` function that
captures the current fiber, and then `yield` to the main event loop. When the
event fires, the fiber will be resumed with the resulting payload
(much of the file API will work like this.) Such functions
must be called inside this event loop.

Using Janet's multiple signal types, we could designate a user signal, such
as `:user9`, for all libuv related scheduling. This would make it very unlikely
for `juv` to interfere with other uses of `yield` in the program not related
to scheduling. For now, we use `yield` for convenience and easy of prototyping.

