VPATH=/usr/include:/usr/local/include

CPPFLAGS=-ggdb -I boost/concept_check/include \
				 -I boost/config/include \
				 -I boost/core/include \
				 -I boost/exception/include \
				 -I boost/iterator/include \
				 -I boost/mpl/include \
				 -I boost/optional/include \
				 -I boost/preprocessor/include \
				 -I boost/smart_ptr/include \
				 -I boost/spirit_classic/include \
				 -I boost/static_assert/include \
				 -I boost/type_traits/include \
				 -I boost/utility/include

LDFLAGS=-lstdc++

SRCS=main.cpp compiler.cpp compiler_save.cpp context.cpp floattable.cpp \
		 functions.cpp opcodes.cpp stdlib.cpp stringtable.cpp value.cpp vmachine.cpp


OBJS=$(SRCS:.cpp=.o)

.PHONY: all clean

all: dscript

clean:
		rm dscript; rm *.dep; rm *.o

dscript: $(OBJS)
	g++ $(LDFLAGS) -o dscript $(OBJS)

-include $(subst .cpp,.dep,$(SRCS))


%.dep: %.cpp
	g++ -M $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*, \1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

