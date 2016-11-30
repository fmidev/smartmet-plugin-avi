PROG = $(patsubst %.cpp,%,$(wildcard *Test.cpp))

MAINFLAGS = -std=c++0x -Wall -W -Wno-unused-parameter

EXTRAFLAGS = -Werror -pedantic -Wpointer-arith -Wcast-qual \
	-Wcast-align -Wwrite-strings -Wconversion -Winline \
	-Wctor-dtor-privacy -Wnon-virtual-dtor -Wno-pmf-conversions \
	-Wsign-promo -Wchar-subscripts -Wold-style-cast \
	-Wshadow -Woverloaded-virtual

DIFFICULTFLAGS = -Weffc++ -Wunreachable-code -Wredundant-decls

CC = g++
CFLAGS = -DUNIX -O0 -g $(MAINFLAGS)
INCLUDES = -I ../include -I /usr/include/smartmet -I /usr/include/mysql

LIBS =	-lsmartmet-spine \
	-lsmartmet_newbase \
	-lsmartmet_macgyver \
	-lboost_date_time \
	-lboost_thread \
	-lboost_filesystem \
	-lboost_iostreams \
	-lboost_regex \
	-lboost_system \
	-lconfig++ \
	-lbz2 -lz \
	-lpthread

all: $(PROG)

clean:
	rm -f $(PROG) *~

test: $(PROG)
	@echo Running tests:
	@mkdir -p tmp failures
	@rm -f tmp/* failures/*
	@for prog in $(PROG); do \
	  ./$$prog; \
	done

$(PROG) : % : %.cpp Makefile ../avi.so
	$(CC) $(CFLAGS) -o $@ $@.cpp $(INCLUDES) $(LIBS)
