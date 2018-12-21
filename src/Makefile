#
# Makefile in src
#
include ../Make.properties
TARGET=libcgh.a
BIN=asm

# subdirs
SUBDIRS=$(foreach dir,$(shell find . -mindepth 1 -type d),$(shell echo $(dir) ' '))


# objs
ALL_OBJS = $(subst .cpp,.o,$(foreach dir,$(SUBDIRS),$(wildcard $(dir)/*.cpp)))


all: $(TARGET)
	mv $(TARGET) $(LIBDIR)

bin: $(BIN)
	mv $(BIN) ../
	
$(BIN): $(ALL_OBJS) main.o
	$(CC) $(CPP_FLAGS) -o $@ $^


$(TARGET): $(ALL_OBJS)
	ar -r $@ $^

.PHONY:clean
clean:
	@for i in $(ALL_OBJS); do \
		echo "$$i"; \
		rm $$i; \
		done
	rm main.o

