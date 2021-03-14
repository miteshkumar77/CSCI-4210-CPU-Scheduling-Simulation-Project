TARGETFULL ?= full.out
TARGETLIM ?= limited.out
SRC_DIRS ?= ./src
CC = clang++
CXX = clang++
SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s)
OBJS := $(addsuffix .o,$(basename $(SRCS)))
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))


limited: $(TARGETLIM)
CPPFLAGS ?= $(INC_FLAGS) -Wall -Werror -g -D DISPLAY_MAX_T=1000
$(TARGETLIM): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@ $(LOADLIBES) $(LDLIBS)

full: $(TARGETFULL)
CPPFLAGS ?= $(INC_FLAGS) -Wall -Werror -g
$(TARGETFULL): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@ $(LOADLIBES) $(LDLIBS) 
	 

.PHONY: clean limited full
clean:
	$(RM) $(TARGET) $(OBJS) $(DEPS)

-include $(DEPS)