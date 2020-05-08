include $(DASSROOT)/cpp/make/dass_client_api.inc

all = lib needed smith


ifdef SUN
CPPFLAGS+=-I$(DASSROOT)/cpp/LogIF -xarch=sparc -m64 -library=stlport4
else
CPPFLAGS+=-I$(DASSROOT)/cpp/LogIF -g
endif

ifndef DISABLE_LOG4CPP
CPPFLAGS+=-I$(DASSROOT)/lib/cpp/log4cpp/include
endif

CPPFLAGS+=-I/home/hrdp/Dass/smith/include


clean:
	$(MAKE) -C $(DASSROOT)/cpp/make -f dass_c_Makefile clean
	rm -f bin/smith
	rm -f objects/*.o

lib:
	echo ">> compiling libs"
	$(MAKE) -C $(DASSROOT)/cpp/make -f dass_c_Makefile
	make -C $(DASSROOT)/cpp/Util
	make -C $(DASSROOT)/cpp/COMS
	make -C $(DASSROOT)/cpp/LogIF
	make -C $(DASSROOT)/cpp/LogIF/Config
	make -C $(DASSROOT)/cpp/DaSS-Net/Internals
	make -C $(DASSROOT)/cpp/COMS_DaSSNet
	make -C $(DASSROOT)/cpp/protocols
	make -C $(DASSROOT)/cpp/protocols/dass
	make -C $(DASSROOT)/cpp/protocols/dass/protocols
	make -C $(DASSROOT)/cpp/protocols/dass/protocols/session
	make -C $(DASSROOT)/cpp/protocols/dass/protocols/internals
	make -C $(DASSROOT)/cpp/protocols/dass/protocols/client
	make -C $(DASSROOT)/cpp/apis/dass
	make -C $(DASSROOT)/cpp/apis/dass/internals
	make -C $(DASSROOT)/cpp/apis/dass/client

needed:
	mkdir -p objects
	g++ -o objects/scan.o -c include/ini/scan.cpp
	g++ -o objects/ini.o -c include/ini/ini.cpp
	g++ -o objects/usoc.o -c include/usoc/usoc.cpp
	g++ $(CPPFLAGS) -o objects/pd.o -c include/dass/pd.cpp
	g++ $(CPPFLAGS) -o objects/pt.o -c include/dass/pt.cpp
	g++ $(CPPFLAGS) -o objects/main.o -c main.cpp

smith:
	echo ">> compiling main executable"
	mkdir -p bin
	$(CC) $(CPPFLAGS) $(DaSSCLIBINCL) -o bin/smith objects/*o $(DaSSCLIBS) $(LIBBS) #$(SSLLIB)

%.o: %.cpp
	echo ">> compiling object files"
	$(CC) $(CPPFLAGS) -c $< -o $*.o

depend:
	makedepend -- $(CPPFLAGS) -- $(MAKEDEPFLAGS) -- $(SOURCES)

# DO NOT DELETE
