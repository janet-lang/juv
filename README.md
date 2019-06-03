# Janet libuv bindings

Bindings to the [libuv](https://libuv.org/) library for asynchronous IO.
Not yet ready for much of anything, still a working prototype.

## Building

Requires libuv to be installed with a working pkg-config file. This will work
on most linux distributions if you install libuv from a package manager. Use
`jpm build` to build the project.

## Testing

See the `test` directory for test scripts and programs.

`jpm test`
