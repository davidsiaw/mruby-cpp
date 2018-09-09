CC := g++
CFLAGS := -std=c++11 -I. -I./mruby/include -Wall -fprofile-arcs -ftest-coverage
LDFLAGS := -Lmruby/build/host/lib/ -lmruby
TEST_DIR := tests
BIN_DIR := bins
LOG_DIR := logs
LIBMRUBY := mruby/build/host/lib/libmruby.a
GITREF := 1.4.0

SOURCES := $(wildcard *.hpp)
TESTS := $(wildcard $(TEST_DIR)/*.cpp)
BINS := $(patsubst $(TEST_DIR)/%.cpp, $(BIN_DIR)/test_%, $(TESTS))

TESTCOMMAND := $(patsubst $(TEST_DIR)/%.cpp, %, $(wildcard $(TEST_DIR)/*.cpp))

RED := \033[0;31m
GREEN := \033[0;32m
NC := \033[0m # No Color

all: test

mruby/minirake:
	git clone https://github.com/mruby/mruby

mruby/gitref: mruby/minirake
	echo $(GITREF) > mruby/gitref

$(LIBMRUBY): mruby/gitref
	cd mruby && git checkout `cat gitref`
	cd mruby && make

$(BIN_DIR)/test_%: $(TEST_DIR)/%.cpp $(LIBMRUBY) $(SOURCES)
	@mkdir -p $(BIN_DIR)
	$(CC) $(patsubst $(BIN_DIR)/test_%, $(TEST_DIR)/%.cpp, $@) $(CFLAGS) -o $@ $(LDFLAGS)

test: $(BINS)
	@mkdir -p $(LOG_DIR)
	@rm -f fail
	@$(foreach file, $(TESTCOMMAND), \
		if sh -c $(BIN_DIR)/test_$(file) 1> $(LOG_DIR)/$(file).stdout 2> $(LOG_DIR)/$(file).stderr; then \
			echo "$(GREEN)PASSED$(NC): $(file)"; \
		else \
			echo "$(RED)FAILED$(NC): $(file)"; \
			echo fail > fail; \
		fi; \
	)
	@if [ -f ./fail ]; then echo "Test failures detected!"; exit 1; fi;
	@gcov *.gcda > gcov.log

lightclean:
	rm *.gcda; rm *.gcno; rm *.gcov

clean: lightclean
	rm $(BIN_DIR)/*; rm gcov.log

bigclean: clean
	cd mruby && make clean

distclean: bigclean
	rm -rf mruby
	rm -rf bin
	rm -rf logs

.PHONY: distclean clean test
