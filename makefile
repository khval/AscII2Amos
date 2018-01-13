
# Version to build
VERSION = 1

#CC     = $(CROSS_COMPILE)g++ 
#CXX    = $(CROSS_COMPILE)c++ 
#AS     = $(CROSS_COMPILE)as 
#LD     = $(CROSS_COMPILE)ld 
#RANLIB = $(CROSS_COMPILE)ranlib 
#RM     = delete
# RM     = rm

warnings = -wall -Wuninitialized


main_objects = init.cpp what_is.cpp  support_functions.cpp special.cpp datatypes.cpp
main_SRCS = ascii2token.cpp

objects = $(main_objects:.cpp=.o)
programs= $(main_SRCS:.cpp=.exe)

%.o:		%.cpp 
	g++ $(warnings) -c -O2 -D__USE_INLINE__ $(@:.o=.cpp) -o $@

%.exe:		%.cpp $(objects)
	g++ $(warnings) -O2 -D__USE_INLINE__ $(@:.exe=.cpp) $(objects) -o $@

all:	 $(programs) $(objects)

clean:
	delete $(programs) $(objects)

.PHONY: revision 
revision:
	bumprev $(VERSION) $(programs)