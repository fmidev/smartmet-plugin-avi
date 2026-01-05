SUBNAME = avi
SPEC = smartmet-plugin-$(SUBNAME)
INCDIR = smartmet/plugins/$(SUBNAME)

REQUIRES = configpp

include $(shell echo $${PREFIX-/usr})/share/smartmet/devel/makefile.inc

# Compiler options

DEFINES = -DUNIX -D_REENTRANT

LIBS += $(PREFIX_LDFLAGS) \
	$(REQUIRED_LIBS) \
	-lsmartmet-timeseries \
	-lsmartmet-spine \
	-lboost_thread \
	-lboost_iostreams \
	-lbz2 -lz

# What to install

LIBFILE = $(SUBNAME).so

# Compilation directories

vpath %.cpp $(SUBNAME)
vpath %.h $(SUBNAME)

# The files to be compiled

SRCS = $(wildcard $(SUBNAME)/*.cpp)
HDRS = $(wildcard $(SUBNAME)/*.h)
OBJS = $(patsubst %.cpp, obj/%.o, $(notdir $(SRCS)))

INCLUDES := -I$(SUBNAME) $(INCLUDES)

.PHONY: test rpm

# The rules

all: objdir $(LIBFILE)
debug: all
release: all
profile: all

$(LIBFILE): $(OBJS)
	$(CXX) $(LDFLAGS) -shared -rdynamic -o $(LIBFILE) $(OBJS) $(LIBS)

clean: 
	rm -f $(LIBFILE) *~ $(SUBNAME)/*~
	rm -rf obj
	$(MAKE) -C test $@

format:
	clang-format -i -style=file $(SUBNAME)/*.h $(SUBNAME)/*.cpp

install:
	@mkdir -p $(plugindir)
	$(INSTALL_PROG) $(LIBFILE) $(plugindir)/$(LIBFILE)

test:
	$(MAKE) -C test $@

objdir:
	@mkdir -p $(objdir)

rpm: clean $(SPEC).spec
	rm -f $(SPEC).tar.gz # Clean a possible leftover from previous attempt
	tar -czvf $(SPEC).tar.gz --exclude test --exclude-vcs --transform "s,^,$(SPEC)/," *
	rpmbuild -tb $(SPEC).tar.gz
	rm -f $(SPEC).tar.gz

.SUFFIXES: $(SUFFIXES) .cpp

obj/%.o: %.cpp
	$(CXX) $(CFLAGS) $(INCLUDES) -c -MD -MF $(patsubst obj/%.o, obj/%.d, $@) -MT $@ -o $@ $<

.PHONY: test

-include $(wildcard obj/*.d)
