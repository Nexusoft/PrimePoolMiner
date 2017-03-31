# Copyright (c) Videlicet[2014]++
# Distributed under the MIT/X11 software license, see the accompanying
# file license.txt or http://www.opensource.org/licenses/mit-license.php.

OUT_DIR = build

PGCC = pgcc
PGCXX = pgc++
PGLIBS = -loaccminer -laccapi -laccg -laccn -laccg2 -ldl -L./ -L/opt/pgi/linux86-64/17.1/lib -L/opt/pgi/linux86-64/16.10/lib
ACCFLAGS = -fast -Minfo=accel -Mprof=ccff -acc $(OPT) -ta=tesla:managed -ta=nvidia:nordc -std=c++11  -larmadillo -lgsl -w  #:managed #-larmadillo -lgsl -w 
#ACCFLAGS = -fast -std=c++11 # Uncomment this and comment the line above to disable OpenACC
PGCXXFLAGS = -fPIC
ACCLIBSFLAGS = -Wl,-rpath,./ -Wl,-rpath,/opt/pgi/linux86-64/17.1/lib -Wl,-rpath,/opt/pgi/linux86-64/16.10/lib

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

LDFLAGS= -Wl,--allow-multiple-definition
#DEBUGFLAGS= -ggdb -ffunction-sections -O0
CXXFLAGS=-Ofast
xCXXFLAGS= -pthread -m64 -static-libgcc -static-libstdc++ -Wall -Wextra -Wno-sign-compare -Wno-invalid-offsetof -Wno-unused-parameter -Wformat -Wformat-security $(DEBUGFLAGS) $(DEFS) $(CXXFLAGS) $(MARCHFLAGS)

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

liboaccminer.so: $(OUT_DIR)/$(OBJECTS)
	$(PGCXX) -shared -o $@ $^

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

$(OUT_DIR)/%.o: hash/%.cpp
	@mkdir -p $(@D)
	$(CXX) -c $(xCXXFLAGS) -MMD -o $@ $<
	@cp $(@:%.o=%.d) $(@:%.o=%.P); \
	  sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	      -e '/^$$/ d' -e 's/$$/ :/' < $(@:%.o=%.d) >> $(@:%.o=%.P); \
	  rm -f $(@:%.o=%.d)


nexus_cpuminer: $(OBJS:obj/%=$(OUT_DIR)/%) liboaccminer.so
	$(CXX) $(xCXXFLAGS) -rdynamic -o $@ $^ $(LDFLAGS) $(LIBS) $(PGLIBS) $(ACCLIBSFLAGS)

clean:
	-rm -f nexus_cpuminer
	-rm -f *.so
	-rm -f $(OUT_DIR)/*.o	
	-rm -f $(OUT_DIR)/*.P
	-rm -f $(OUT_DIR)/oacc/*.o	
	-rm -f $(OUT_DIR)/oacc/*.P
FORCE:
