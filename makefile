# Copyright (c) Videlicet[2014]++
# Distributed under the MIT/X11 software license, see the accompanying
# file license.txt or http://www.opensource.org/licenses/mit-license.php.

LDFLAGS= -Wl,--allow-multiple-definition

OBJ	= o
EXE	= 
DLL = .so
UNAME := $(shell uname -a)
ifeq ($(findstring CYGWIN_NT, $(UNAME)), CYGWIN_NT)
OBJ	= obj
EXE	= .exe
DLL = .dll
LDFLAGS += -Wl,--stack,100000000
endif


OUT_DIR = build

DEFS=-DBOOST_SPIRIT_THREADSAFE -DBOOST_THREAD_USE_LIB

DEFS += $(addprefix -I,$(CURDIR) $(CURDIR)/$(OUT_DIR) $(CURDIR)/hash $(BOOST_INCLUDE_PATH) $(OPENSSL_INCLUDE_PATH))
LIBS = $(addprefix -L,$(BOOST_LIB_PATH) $(BDB_LIB_PATH) $(OPENSSL_LIB_PATH))

#For Boost 1.55 Builds Uncomment the Following
BOOST_LIB_PATH=/usr/local/lib
#BOOST_INCLUDE_PATH=/usr/include/boost


LIBS += \
   -l boost_system$(BOOST_LIB_SUFFIX) \
   -l boost_filesystem$(BOOST_LIB_SUFFIX) \
   -l boost_program_options$(BOOST_LIB_SUFFIX) \
   -l boost_thread$(BOOST_LIB_SUFFIX) \
   -l crypto \
   -l gmp


ifndef MARCHFLAGS
MARCHFLAGS=-mtune=native -march=native
#	MARCHFLAGS=-mtune=ivybridge -march=ivybridge -mavx -fabi-version=0
#	MARCHFLAGS=-march=broadwell -march=broadwell -mavx2 -fabi-version=0  
endif

$(info $(DEBUG))
ifeq ($(DEBUG),TRUE)
	CXXFLAGS=-ggdb -ffunction-sections -O0
else
	CXXFLAGS=-Ofast
endif

xCXXFLAGS= -std=gnu++11 -pthread -m64 -static-libgcc -static-libstdc++ -Wno-sign-compare -Wno-invalid-offsetof -Wno-unused-parameter -Wformat -Wformat-security $(DEBUGFLAGS) $(DEFS) $(CXXFLAGS) $(MARCHFLAGS)

ifdef PGI
$(info Using PGI OpenACC compiler)
PGCC = pgcc
PGCXX = pgc++
PGLIBS = -laccapi -laccg -laccn -laccg2 -ldl -L./ -L/opt/pgi/linux86-64/17.1/lib -L/opt/pgi/linux86-64/16.10/lib
ACCFLAGS = -fast -Minfo=accel -Mprof=ccff -acc $(OPT) -ta=tesla:managed -ta=nvidia:nordc -std=c++11 # -larmadillo -lgsl -w  #:managed #-larmadillo -lgsl -w 
#ACCFLAGS = -fast -std=c++11 # Uncomment this and comment the line above to disable OpenACC
ACCLIBSFLAGS = -Wl,-rpath,./ -Wl,-rpath,/opt/pgi/linux86-64/17.1/lib -Wl,-rpath,/opt/pgi/linux86-64/16.10/lib
xCXXFLAGS += -D __PGI
else # No PGI Compiler
$(info Using GCC compiler)
PGCC = $(CC)
PGCXX = $(CXX)
PGLIBS = -L./
ACCFLAGS = $(xCXXFLAGS)
ACCLIBSFLAGS = -Wl,-rpath,./
endif

PGCXXFLAGS = -fPIC

HEADERS = $(wildcard *.h)
OBJS= \
	$(OUT_DIR)/skein.o \
	$(OUT_DIR)/skein_block.o \
	$(OUT_DIR)/KeccakDuplex.o \
	$(OUT_DIR)/KeccakSponge.o \
	$(OUT_DIR)/Keccak-compact64.o \
	$(OUT_DIR)/KeccakHash.o \
	$(OUT_DIR)/util.o \
	$(OUT_DIR)/prime.o \
	$(OUT_DIR)/miner.o \
	$(OUT_DIR)/config.o

OBJECTS = $(patsubst %.cpp,%.o,$(wildcard oacc/*.c*))

all: nexus_cpuminer

liboaccminer$(DLL): $(OUT_DIR)/$(OBJECTS)
	$(PGCXX) -m64 -shared -o $@ $^ $(LIBS)

$(OUT_DIR)/oacc/%.o: oacc/%.cpp
	@mkdir -p $(@D)
	$(PGCXX) $(PGCXXFLAGS) $(ACCFLAGS) -o $@ -c $<
	

$(OUT_DIR)/%.o: %.cpp $(HEADERS)
	@mkdir -p $(@D)
	$(CXX) -c $(xCXXFLAGS) -MMD -o $@ $<
	@cp $(@:%.o=%.d) $(@:%.o=%.P); \
	  sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	      -e '/^$$/ d' -e 's/$$/ :/' < $(@:%.o=%.d) >> $(@:%.o=%.P); \
	  rm -f $(@:%.o=%.d)

$(OUT_DIR)/%.o: hash/%.c $(HEADERS)
	@mkdir -p $(@D)
	$(CXX) -c $(xCXXFLAGS) -MMD -fpermissive -o $@ $<
	@cp $(@:%.o=%.d) $(@:%.o=%.P); \
	  sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	      -e '/^$$/ d' -e 's/$$/ :/' < $(@:%.o=%.d) >> $(@:%.o=%.P); \
	  rm -f $(@:%.o=%.d)

$(OUT_DIR)/%.o: hash/%.cpp $(HEADERS)
	@mkdir -p $(@D)
	$(CXX) -c $(xCXXFLAGS) -MMD -o $@ $<
	@cp $(@:%.o=%.d) $(@:%.o=%.P); \
	  sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	      -e '/^$$/ d' -e 's/$$/ :/' < $(@:%.o=%.d) >> $(@:%.o=%.P); \
	  rm -f $(@:%.o=%.d)


nexus_cpuminer: $(OBJS:obj/%=$(OUT_DIR)/%) liboaccminer$(DLL)
	$(CXX) $(xCXXFLAGS) -rdynamic -o $@ $^ $(LDFLAGS) -loaccminer $(LIBS) $(PGLIBS) $(ACCLIBSFLAGS) 

clean:
	-rm -f nexus_cpuminer$(EXE)
	-rm -f liboaccminer$(DLL)
	-rm -f $(OUT_DIR)/*.o	
	-rm -f $(OUT_DIR)/*.P
	-rm -f $(OUT_DIR)/oacc/*.o	
	-rm -f $(OUT_DIR)/oacc/*.P
FORCE:
