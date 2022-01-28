# Definitions

CC = gcc
CC_FLAGS = -fomit-frame-pointer -fno-asynchronous-unwind-tables -ggdb -Wall -Werror
CC_OPTIONS = -c
CC_SO_OPTIONS = -shared -fpic
CC_DL_OPTIONS = -rdynamic
OD = objdump
OD_FLAGS = -d -h -r -s -S -t 
RM = /bin/rm -f
LD = gcc
LIBS = -ldl

SRCS := ci.c handle_args.c interface.c lex.c parse.c eval.c print.c err_handler.c variable.c
OBJS := $(SRCS:%.c=%.o)

SRCSREF := ci.c handle_args.c interface-ref.c lex.c parse-ref.c eval-ref.c print.c err_handler.c variable-ref.c
OBJSREF := $(SRCS:%.c=%.o)

HANDOUT := tests ansicolors.h ci.c ci.h ci_reference driver.sh err_handler.c err_handler.h eval.c handle_args.c interface.c lex.c node.h parse.c print.c token.h type.h value.h variable.c variable.h
AUTOGRADER := tests tests-ref ansicolors.h ci.c ci.h err_handler.c err_handler.h eval-ref.c handle_args.c interface.c interface-ref.c lex.c node.h parse-ref.c print.c token.h type.h value.h variable-ref.c variable.h

HDRS := ci.h node.h
TESTS := tests/test_simple.txt

# Generic rules

%.i: %.c
	${CC} ${CC_OPTIONS} -E ${CC_FLAGS} $<

%.s: %.c
	${CC} ${CC_OPTIONS} -S ${CC_FLAGS} $<

%.o: %.c
	${CC} ${CC_OPTIONS} ${CC_FLAGS} $<

# Targets

all: ci ci_reference test clean

ci: ${OBJS} ${HDRS}
	${CC} ${CC_FLAGS} -o $@ ${OBJS}

ci_reference: ${SRCSREF} ${HDRS}
	${CC} -fomit-frame-pointer -fno-asynchronous-unwind-tables -Wall -Werror -O2 -o $@ ${SRCSREF}

test: ci
	chmod +x driver.sh
	./driver.sh ${TESTS}

handout: ci ci_reference
	rm -rf ci-lab ci-lab.tar
	mkdir ci-lab
	cp -r ${HANDOUT} ci-lab/
	cp Makefile-handout ci-lab/Makefile
	tar cvf ci-lab.tar ci-lab
	rm -rf ci-lab

autograder: FORCE
	rm -rf autograder/src autograder.zip
	mkdir autograder/src
	cp -r ${AUTOGRADER} autograder/src/
	cp Makefile autograder/src/Makefile
	cd autograder && zip -r ../autograder.zip ./*
	rm -rf autograder/src

clean:
	${RM} *.o *.so ci-lab.tar autograder.zip

FORCE:
