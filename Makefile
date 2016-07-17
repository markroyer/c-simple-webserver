# Mark Royer
#
# A simple Makefile for compiling the program program and related 
# documentation.  To compile and generate docs a user can simply type
# 
# make all
#
# at the command line.  To remove all of the files that were built and leave
# only the source files, a user can type
#
# make clean
#
# at the command line.  For more information about Makefiles see
#
# http://www.gnu.org/software/make/manual/make.html
#
# The top level directory
rootdir = .

default: compile

compile: src/webserver.c src/webserver.h
        # Compile source files
        # Sun OS Build use -lsocket too
	@gcc src/webserver.c -lpthread -lnsl -o webserver

	@chmod +x webserver

docs: webserver.Doxyfile src/webserver.c src/webserver.h
	@doxygen webserver.Doxyfile

all: compile docs

clean:
	@rm -rf webserver WebContent/docs *~ 


