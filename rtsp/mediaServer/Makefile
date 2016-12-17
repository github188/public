INCLUDES = -I../UsageEnvironment/include -I../groupsock/include -I../liveMedia/include -I../BasicUsageEnvironment/include
# Default library filename suffixes for each library that we link with.  The "config.*" file might redefine these later.
libliveMedia_LIB_SUFFIX = $(LIB_SUFFIX)
libBasicUsageEnvironment_LIB_SUFFIX = $(LIB_SUFFIX)
libUsageEnvironment_LIB_SUFFIX = $(LIB_SUFFIX)
libgroupsock_LIB_SUFFIX = $(LIB_SUFFIX)
##### Change the following for your environment:
CROSS_COMPILE?=		/opt/CodeSourcery/Sourcery_G++_Lite/bin/arm-none-linux-gnueabi-
COMPILE_OPTS =		$(INCLUDES) -I. -O2 -DSOCKLEN_T=socklen_t -DNO_SSTREAM=1 -D_LARGEFILE_SOURCE=1 -D_FILE_OFFSET_BITS=64
C =			c
C_COMPILER =		$(CROSS_COMPILE)gcc
C_FLAGS =		$(COMPILE_OPTS) -g
CPP =			cpp
CPLUSPLUS_COMPILER =	$(CROSS_COMPILE)g++
CPLUSPLUS_FLAGS =	$(COMPILE_OPTS) -Wall -g -DBSD=1
OBJ =			o
LINK =			$(CROSS_COMPILE)g++ -o
LINK_OPTS =		
CONSOLE_LINK_OPTS =	$(LINK_OPTS)
LIBRARY_LINK =		$(CROSS_COMPILE)ar cr 
LIBRARY_LINK_OPTS =	$(LINK_OPTS)
LIB_SUFFIX =			a
LIBS_FOR_CONSOLE_APPLICATION =
LIBS_FOR_GUI_APPLICATION =
EXE =
arm=1
MEDIASERVERSRC=$(wildcard ./*.cpp)
CXXOBJS=$(patsubst %.cpp,%.o,$(MEDIASERVERSRC))

CXXOBJTARGET=$(CXXOBJS):%.o:%.cpp

COBJS=$(patsubst %.c,%.o,$(CCMEDIASERVERSRC))
COBJTARGET=$(COBJS):%.o:%.c

OBJS=$(COBJS) $(CXXOBJS)

LDFLAGS =-shared



CPLUSPLUS_COMPILER+=-Wall -Werror -fmessage-length=0
C_COMPILER+=-fmessage-length=0
LIB=../liveMedia/libliveMedia.a ../groupsock/libgroupsock.a \
../BasicUsageEnvironment/libBasicUsageEnvironment.a ../UsageEnvironment/libUsageEnvironment.a 
ifeq ($(arm), 1)
LIB+=-lpthread -lm -lrt 
MODULENAME=libRtsp.so
else
LIB+=-lpthread -lm -lrt ../../../../lib/x86/libzlog.a
MODULENAME=../../../../bin/x86/libRtspServer.so
endif


	
all:$(OBJS)
	$(CPLUSPLUS_COMPILER) $(LDFLAGS) -o $(MODULENAME) $^ $(LIB)
$(CXXOBJTARGET)
	$(CPLUSPLUS_COMPILER) $(CPLUSPLUS_FLAGS) $<  -c -fPIC -o $@ $(INCLUDES)
$(COBJTARGET)
	$(C_COMPILER) $(CPLUSPLUS_FLAGS) $<  -c  -fPIC -o $@ $(INCLUDES)
	echo $(CXXOBJTARGET)
	echo $(COBJTARGET)

clean:
	@echo Please make in directory build
	@echo $(OBJS)
	rm -f $(MODULENAME) 
	rm -f $(OBJS)
	
