PROG:=clide
CPPS:=$(wildcard src/*.cpp)
OBJS:=$(patsubst %.cpp,obj/%.o,$(CPPS))
CFLAGS:=-g -O0 -std=gnu++11
LFLAGS:=-g -std=gnu++11 -lncurses

all:	obj/$(PROG)

obj/$(PROG):	$(OBJS)
	g++ -o $@ $(OBJS) $(LFLAGS)

clean:
	rm -rf obj

obj/%.o:	%.cpp
	-mkdir -p $(dir $@)
	g++ -c -MMD -pipe -o $@ $< $(CFLAGS)


-include $(patsubst %.o,%.d,$(OBJS))
