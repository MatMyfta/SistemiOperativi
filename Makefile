CFLAGS = -std=gnu90 -Wall -Wextra -Wno-unused
SRCS := $(shell find ./src/ -type f -iname "*.c")
DEPENDS := $(patsubst %.c,%.d,$(SRCS))
EXECUTABLES_NAMES := main analyzer counter p q
BINARY_OUTPUT_PREFIX := bin
EXECUTABLES := $(addprefix ${BINARY_OUTPUT_PREFIX}/, $(EXECUTABLES_NAMES))

all: build

# See PHONY targets
# https://www.gnu.org/software/make/manual/html_node/Phony-Targets.html
.PHONY: help
help:
	cat README.md

.PHONY: clean
clean:
	rm -rf bin/
	find src/ -type f \( -iname "*.a" -or -iname "*.o" -or -iname "*.a" \) -exec rm -v {} \;

.PHONY: build
build: ${DEPENDS} ${EXECUTABLES}

.PHONY: cleandep
cleandep:
	rm -rf $(DEPENDS)


################################################################################
# Executables
################################################################################
${EXECUTABLES}: $(wildcard *.o) $(wildcard *.a)
	@mkdir -p ${BINARY_OUTPUT_PREFIX}
	$(CC) $(CFLAGS) -o $@ $^

${BINARY_OUTPUT_PREFIX}/main: \
	./src/main/main.o \
	./src/analyzer/libanalyzer_api.a \
	./src/libcommon.a \
	./src/containers/libcontainers.a
${BINARY_OUTPUT_PREFIX}/analyzer: \
	src/analyzer/main.o \
	src/analyzer/counter/libcounter_api.a \
	./src/libcommon.a \
	./src/containers/libcontainers.a
${BINARY_OUTPUT_PREFIX}/counter: \
	src/analyzer/counter/main.o \
	src/analyzer/counter/path.o \
	src/analyzer/counter/counter.o src/analyzer/counter/counter_self.o \
	src/analyzer/p/libp_api.a \
	./src/libcommon.a \
	./src/containers/libcontainers.a
${BINARY_OUTPUT_PREFIX}/p: \
	src/analyzer/p/main.o \
	src/analyzer/p/p_self.o \
	src/analyzer/q/libq_api.a \
	./src/libcommon.a \
	./src/containers/libcontainers.a
${BINARY_OUTPUT_PREFIX}/q: \
	src/analyzer/q/main.o \
	src/analyzer/q/q_self.o \
	./src/libcommon.a \
	./src/containers/libcontainers.a

################################################################################
# Libraries
################################################################################
libcommon_srcs = ${shell find ./src/ -maxdepth 1 -type f -iname "*.c"}
libcommon_objs = $(patsubst %.c,%.o,$(libcommon_srcs))
src/libcommon.a: src/libcommon.a(${libcommon_objs})

libcontainers_srcs = ${shell find ./src/containers -maxdepth 1 -type f -iname "*.c"}
libcontainers_objs = $(patsubst %.c,%.o,$(libcontainers_srcs))
src/containers/libcontainers.a: src/containers/libcontainers.a(${libcontainers_objs})

src/analyzer/libanalyzer_api.a: src/analyzer/libanalyzer_api.a(./src/analyzer/analyzer.o)
src/analyzer/counter/libcounter_api.a: src/analyzer/counter/libcounter_api.a(src/analyzer/counter/counter.o)
src/analyzer/p/libp_api.a: src/analyzer/p/libp_api.a(src/analyzer/p/p.o)
src/analyzer/q/libq_api.a: src/analyzer/q/libq_api.a(src/analyzer/q/q.o)

########################################
# Source file dependencies
# See
# https://www.gnu.org/software/make/manual/html_node/Automatic-Prerequisites.html#Automatic-Prerequisites
########################################
%.d: %.c
	@set -e; rm -f $@; \
	$(CC) -MM -MT $(patsubst %.c,%.o,$<) $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$
include $(SRCS:.c=.d)
