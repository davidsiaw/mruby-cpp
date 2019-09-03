CC := g++
CFLAGS := -std=c++11 -I. -I./mruby/include -g -Wall -fprofile-arcs -ftest-coverage
LDFLAGS := -Lmruby/build/host/lib/ -lmruby
TEST_DIR := tests
BIN_DIR := bins
LOG_DIR := logs
LIBMRUBY := mruby/build/host/lib/libmruby.a
GITREF := master

SOURCES := $(wildcard *.hpp)
TESTS := $(wildcard $(TEST_DIR)/*.cpp)
BINS := $(patsubst $(TEST_DIR)/%.cpp, $(BIN_DIR)/test_%, $(TESTS))
ALL_TESTS := $(patsubst $(TEST_DIR)/%.cpp, test_%, $(TESTS))

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

test_%: $(BIN_DIR)/test_%
	@mkdir -p $(LOG_DIR)
	@mkdir -p $(BIN_DIR)
	valgrind --error-exitcode=1 --leak-check=full --log-file=$(LOG_DIR)/$@.valgrind $(BIN_DIR)/$@ 1> $(LOG_DIR)/$@.stdout 2> $(LOG_DIR)/$@.stderr; echo "$$?" > $(LOG_DIR)/$@.retcode

runtest: $(ALL_TESTS)
	@rm -f fail
	@rm -f pcount
	@rm -f fcount
	@echo "" > pcount
	@echo "" > fcount
	@$(foreach file, $(TESTCOMMAND), \
		if grep -q 0 $(LOG_DIR)/test_$(file).retcode; then \
			echo "$(GREEN)PASSED$(NC): $(file)"; \
			sed -i '$$ s/$$/pass /' pcount; \
		else \
			echo "$(RED)FAILED$(NC): $(file)"; \
			echo $(file) >> fail; \
			sed -i '$$ s/$$/fail /' fcount; \
		fi; \
	)

test: runtest
	@echo $(words $(shell cat pcount)) "passed"
	@echo $(words $(shell cat fcount)) "failures"
	@gcov $(LOG_DIR)/*.gcda > gcov.log
	cp *.gcda $(LOG_DIR) && rm -f *.gcda
	cp *.gcno $(LOG_DIR) && rm -f *.gcno
	cp *.gcov $(LOG_DIR) && rm -f *.gcov
	@if [ -f fail ]; then echo "Test failures detected!"; exit 1; fi;

lightclean:
	rm -f $(LOG_DIR)/*.gcda $(LOG_DIR)/*.gcno $(LOG_DIR)/*.gcov *.gcno *.gcov

clean: lightclean
	rm -f $(BIN_DIR)/* $(LOG_DIR)/* pcount fcount

bigclean: clean
	cd mruby && make clean

distclean: bigclean
	rm -rf mruby
	rm -rf bin
	rm -rf logs

.PHONY: distclean clean test runtest
