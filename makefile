# programs
CC              :=gcc
SHELL           :=/bin/bash

# paths
SRCDIR           =.
VPATH           +=${SRCDIR}
NAME             =listwise

BINDIR           =/usr/local/bin
INSTALL         :=install
INSTALL_PROGRAM :=${INSTALL}

export

#
# required objects
#
SOURCES          =main.c args.c

#
# compiler flags
#  C/L OPTS  - internal to makefile flags
#  C/L FLAGS - user specified compiler flags
# 
COPTS						+=-m64 -O0 -g3 -Werror -D_GNU_SOURCE -fPIC -I${SRCDIR} -I${SRCDIR}/common
%.o : COPTS     +=-c
LOPTS            =-llistwise

#
# recipes
#

# link the executable
${NAME}: ${SOURCES:.c=.o}
	${CC} ${COPTS} ${CFLAGS} -o $@ *.o ${LOPTS} ${LFLAGS}

#
# phony targets
#

.PHONY: install uninstall clean

install: ${NAME}
	${INSTALL} -d               ${DESTDIR}/${BINDIR}
	${INSTALL_PROGRAM} ${NAME}  ${DESTDIR}/${BINDIR}/${NAME}
	ln -vfs listwise /usr/local/bin/lw

uninstall:
	rm -f												${DESTDIR}/${BINDIR}/${NAME}

clean ::
	rm -f *.o ${NAME}
