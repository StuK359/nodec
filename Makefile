#-------------------------------------------------------------------------
#  Copyright 2016, Daan Leijen. 
#-------------------------------------------------------------------------

.PHONY : clean dist init tests staticlib main

include out/makefile.config

ifndef $(VARIANT)
VARIANT=debug
endif

CONFIGDIR  = out/$(CONFIG)
OUTDIR 		 = $(CONFIGDIR)/$(VARIANT)
INCLUDES   = -Iinc -I$(CONFIGDIR) -Ideps -Ideps/libuv/include

ifeq ($(VARIANT),release)
CCFLAGS    = $(CCFLAGSOPT) -DNDEBUG $(INCLUDES)
CXXFLAGS   = $(CXXFLAGSOPT) -DNDEBUG $(INCLUDES)
else ifeq ($(VARIANT),testopt)
CCFLAGS    = $(CCFLAGSOPT) $(INCLUDES)
CXXFLAGS   = $(CXXFLAGSOPT) $(INCLUDES)
else ifeq ($(VARIANT),debug)
CCFLAGS    = $(CCFLAGSDEBUG) $(INCLUDES)
CXXFLAGS   = $(CXXFLAGSDEBUG) $(INCLUDES)
else
VARIANTUNKNOWN=1
endif

# Use VALGRIND=1 to memory check under valgrind
ifeq ($(VALGRIND),1)
VALGRINDX=yes
else ifeq ($(VALGRIND),yes)
VALGRINDX=yes
else
VALGRINDX=
endif

ifeq ($(VALGRINDX),yes)
VALGRINDX=valgrind --leak-check=full --show-leak-kinds=all --suppressions=./valgrind.supp 
endif     

# Uncomment to generate assembly for nodec
# SHOWASM    = -Wa,-aln=$@.s

# -------------------------------------
# Sources
# -------------------------------------

SRCFILES = async.c channel.c dns.c fs.c http.c http_request.c \
           http_static.c http_url.c  \
           interleave.c memory.c mime.c \
           stream.c tcp.c timer.c tty.c            

CTESTS   =  

TESTFILES= main.c	$(CTESTS)				 

BENCHFILES=


SRCS     = $(patsubst %,src/%,$(SRCFILES)) $(patsubst %,src/%,$(ASMFILES)) deps/http-parser/http_parser.c
OBJS  	 = $(patsubst %.c,$(OUTDIR)/%$(OBJ), $(SRCFILES)) $(patsubst %$(ASM),$(OUTDIR)/%$(OBJ),$(ASMFILES)) $(OUTDIR)/http_parser$(OBJ)
HLIB     = $(OUTDIR)/nodec$(LIB)

LIBS     =  $(HLIB) -Ldeps/libhandler/$(OUTDIR) -Ldeps/libuv/.libs -deps/libz  -lhandler -luv -lz

TESTSRCS = $(patsubst %,test/%,$(TESTFILES)) 
TESTMAIN = $(OUTDIR)/nodec-tests$(EXE)

BENCHSRCS= $(patsubst %,test/%,$(BENCHFILES))
BENCHMAIN= $(OUTDIR)/nodec-bench$(EXE)



TESTFILESXX=main.cpp

OUTDIRXX = $(OUTDIR)xx
SRCSXX   = $(patsubst %,src/%,$(SRCFILES)) $(patsubst %,src/%,$(ASMFILES))
OBJSXX	 = $(patsubst %.c,$(OUTDIRXX)/%$(OBJ), $(SRCFILES)) $(patsubst %$(ASM),$(OUTDIRXX)/%$(OBJ),$(ASMFILES))
HLIBXX   =$(OUTDIRXX)/nodec++$(LIB)

TESTSRCSXX = $(patsubst %,test/%,$(TESTFILESXX)) 
TESTMAINXX = $(OUTDIRXX)/nodec-tests++$(EXE)

# -------------------------------------
# Main targets
# -------------------------------------

main: init staticlib

tests: init staticlib testmain
	@echo ""
	@echo "run tests"
	$(VALGRINDX) $(TESTMAIN)

bench: init staticlib benchmain
	@echo ""
	@echo "run benchmark"
	$(BENCHMAIN)



mainxx: initxx staticlibxx

testsxx: initxx staticlibxx testmainxx
	@echo ""
	@echo "run tests++"
	$(VALGRINDX) $(TESTMAINXX)

# -------------------------------------
# build tests
# -------------------------------------

testmain: $(TESTMAIN)

$(TESTMAIN): $(TESTSRCS) $(HLIB)
	$(CC) $(CCFLAGS)  $(LINKFLAGOUT)$@ $(TESTSRCS) $(LIBS) -lrt -lpthread -lnsl -ldl


testmainxx: $(TESTMAINXX)

$(TESTMAINXX): $(TESTSRCSXX) $(HLIBXX)
	$(CXX) $(CXXFLAGS)  $(LINKFLAGOUT)$@ $(TESTSRCSXX) $(HLIBXX)


# -------------------------------------
# build benchmark
# -------------------------------------

benchmain: $(BENCHMAIN)

$(BENCHMAIN): $(BENCHSRCS) $(HLIB)
	$(CC) $(CCFLAGS) $(LINKFLAGOUT)$@  $(BENCHSRCS) $(HLIB) -lm


benchmainxx: $(BENCHMAINXX)

$(BENCHMAINXX): $(BENCHSRCSXX) $(HLIBXX)
	$(CXX) $(CXXFLAGS) $(LINKFLAGOUT)$@  $(BENCHSRCSXX) $(HLIBXX)


# -------------------------------------
# build the static library
# -------------------------------------

staticlib: initxx $(HLIB)

$(HLIB): $(OBJS)
	$(AR) $(ARFLAGS)  $(ARFLAGOUT)$@ $(OBJS) 

$(OUTDIR)/%$(OBJ): src/%.c
	$(CC) $(CCFLAGS) $(CCFLAG99) $(CCFLAGOUT)$@ -c $< $(SHOWASM)

$(OUTDIR)/%$(OBJ): src/%$(ASM)
	$(CC) $(ASMFLAGS)  $(ASMFLAGOUT)$@ -c $< 

$(OUTDIR)/http_parser$(OBJ): deps/http-parser/http_parser.c
	$(CC) $(CCFLAGS) $(CCFLAG99) $(CCFLAGOUT)$@ -c $< $(SHOWASM)


staticlibxx: $(HLIBXX)

$(HLIBXX): $(OBJSXX)
	$(AR) $(ARFLAGS)  $(ARFLAGOUT)$@ $(OBJSXX) 

$(OUTDIRXX)/%$(OBJ): src/%.cpp
	$(CXX) $(CXXFLAGS) $(CCFLAGOUT)$@ -c $< 

$(OUTDIRXX)/%$(OBJ): src/%.c
	$(CXX) $(CXXFLAGS) $(CCFLAGOUT)$@ -c $< $(SHOWASM)  

$(OUTDIRXX)/%$(OBJ): src/%$(ASM)
	$(CXX) $(ASMFLAGS)  $(ASMFLAGOUT)$@ -c $< 


# -------------------------------------
# other targets
# -------------------------------------

docs: 

clean:
	rm -rf $(CONFIGDIR)/*/*
	touch $(CONFIGDIR)/makefile.depend

init:
	@echo "use 'make help' for help"
	@echo "build variant: $(VARIANT), configuration: $(CONFIG)"
	@if test "$(VARIANTUNKNOWN)" = "1"; then echo ""; echo "Error: unknown build variant: $(VARIANT)"; echo "Use one of 'debug', 'release', or 'testopt'"; false; fi
	@if test -d "$(OUTDIR)/asm"; then :; else $(MKDIR) "$(OUTDIR)/asm"; fi

initxx: init	
	@if test -d "$(OUTDIRXX)/asm"; then :; else $(MKDIR) "$(OUTDIRXX)/asm"; fi

help:
	@echo "Usage: make <target>"
	@echo "Or   : make VARIANT=<variant> <target>"
	@echo "Or   : make VALGRIND=1 tests"
	@echo ""
	@echo "Variants:"
	@echo "  debug       : Build a debug version (default)"
	@echo "  testopt     : Build an optimized version but with assertions"
	@echo "  release     : Build an optimized release version"
	@echo ""
	@echo "Targets:"
	@echo "  main        : Build a static library (default)"
	@echo "  tests       : Run tests"
	@echo "  mainxx      : Build a static library for C++"
	@echo "  testsxx     : Run tests for C++"
	@echo "  bench       : Run benchmarks, use 'VARIANT=release'"	
	@echo "  clean       : Clean output directory"
	@echo "  depend      : Generate dependencies"
	@echo ""
	@echo "Configuration:"
	@echo "  output dir  : $(OUTDIR)"
	@echo "  c-compiler  : $(CC) $(CCFLAGS)"
	@echo ""

# dependencies
# [gcc -MM] generates the dependencies without the full
# directory name, ie.
#  evaluator.o: ...
# instead of
#  core/evaluator.o: ..
# we therefore use [sed] to append the directory name
depend: init
	$(CCDEPEND) $(INCLUDES) src/*.c > $(CONFIGDIR)/temp.depend
	sed -e "s|\(.*\.o\)|$(CONFIGDIR)/\$$(VARIANT)/\1|g" $(CONFIGDIR)/temp.depend > $(CONFIGDIR)/makefile.depend
	$(CCDEPEND) $(INCLUDES) test/*.c > $(CONFIGDIR)/temp.depend
	sed -e "s|\(.*\.o\)|$(CONFIGDIR)/\$$(VARIANT)/\1|g" $(CONFIGDIR)/temp.depend >> $(CONFIGDIR)/makefile.depend
	$(RM) $(CONFIGDIR)/temp.depend

include $(CONFIGDIR)/makefile.depend