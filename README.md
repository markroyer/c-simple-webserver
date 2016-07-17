# c-simple-webserver

A very simple web server written in C.

This is a simple C Web Server.  The program will serve pages that are
placed in the WebContent directory.

## Setup

For Debian based systems, make sure that you have the Debian
`build-essentials` package installed.  If you want to also build
Doxygen documentation, install the `doxygen` package.

## Compilation

The system can be compiled by typing

```bash
make
```

in the root directory.  This will produce an executable called
webserver.

If you have Doxygen installed, you can type

```bash
make all
```

to build everything -- including the documentation files. 



## Usage

After building the program, it can be executed by typing

```bash
./webserver
```

at the command line.  The basic usage is as follows.


```bash
usage: webserver ipaddress [port]
```

By default the webserver runs with ip-address 127.0.0.1 and 1024.

Files are served from the `WebContent` directory.

## License

The project is licensed under the terms of the
[GPL3](https://www.gnu.org/licenses/gpl-3.0.en.html) license.


<!--  LocalWords:  webserver WebContent Doxygen doxygen ipaddress ip
 -->
