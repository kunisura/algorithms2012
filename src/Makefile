# $Id: Makefile 9 2011-11-16 06:38:04Z iwashita $

TARGET	= $(patsubst %.cpp,%,$(wildcard *.cpp))

SRCS	= $(wildcard *.cpp */*.cpp)
OBJS	= $(SRCS:%.cpp=%.o)

CPPFLAGS	= $(CPPDEBUG) -I. -I../src -std=c++11
CXXFLAGS	= $(CXXDEBUG) -Wall -fmessage-length=0
LDFLAGS		= $(CXXDEBUG) -static

CPPDEBUG	= -DNDEBUG
CXXDEBUG	= -O3

debug:	CPPDEBUG = -DDEBUG
debug:	CXXDEBUG = -g

ddsat:	LDLIBS = -lcudd
test1:	LDLIBS = -lcudd
zsligen: LDLIBS = -lcudd -lsapporo
zsligen2: LDLIBS = -lcudd

%: %.o
	$(CXX) $(LDFLAGS) $(TARGET_ARCH) $^ $(LDLIBS) -o $@

.PONY: all debug clean depend

all:	$(TARGET)
debug:	$(TARGET)
clean:
	$(RM) $(OBJS) $(TARGET)

define make-depend
	$(RM) depend.in
	for i in $(SRCS:%.cpp=%); do\
		$(CPP) $(CPPFLAGS) -MM $$i.cpp | perl -p0 -e\
			"s!^\S+.o:!$$i.o:!;\
			 s/ *[^ ]*\.cpp//g" >> depend.in;\
	done
	for i in $(TARGET); do\
		$(CPP) $(CPPFLAGS) -MM $$i.cpp | perl -p0 -e\
			's/^(\S+)\.o:/$$1:/;\
			 s/ *(\S+)\.[ch]pp/-f"$$1.cpp"?" $$1.o":""/eg;\
			 s/\n *\\\n/\n/g' >> depend.in;\
	done
endef

depend:
	$(make-depend)

depend.in:
	@ $(make-depend)

-include depend.in
