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
ALL_MEMTESTS := $(patsubst $(TEST_DIR)/%.cpp, memtest_%, $(TESTS))

TESTCOMMAND := $(patsubst $(TEST_DIR)/%.cpp, %, $(wildcard $(TEST_DIR)/*.cpp))

RED := \033[0;31m
GREEN := \033[0;32m
LRED := \033[0;91m
NC := \033[0m # No Color

all: test

$(LIBMRUBY):
	cd mruby && make

$(BIN_DIR)/test_%: $(TEST_DIR)/%.cpp $(LIBMRUBY) $(SOURCES)
	@echo "COMPILE $@"
	@mkdir -p $(BIN_DIR)
	@$(CC) $(patsubst $(BIN_DIR)/test_%, $(TEST_DIR)/%.cpp, $@) $(CFLAGS) -o $@ $(LDFLAGS)

test_%: $(BIN_DIR)/test_%
	@echo "TESTING $(@:test_%=%)"
	@mkdir -p $(LOG_DIR)
	@$(BIN_DIR)/$@ 1> $(LOG_DIR)/$@.stdout 2> $(LOG_DIR)/$@.stderr; echo "$$?" > $(LOG_DIR)/$@.retcode

memtest_%: $(BIN_DIR)/test_%
	@echo "LEAKCHK $(@:memtest_%=%)"
	@mkdir -p $(LOG_DIR)
	@valgrind --error-exitcode=1 --leak-check=full --log-file=$(LOG_DIR)/$@.valgrind $(BIN_DIR)/$(@:memtest_%=test_%) 1> $(LOG_DIR)/$@.stdout 2> $(LOG_DIR)/$@.stderr; echo "$$?" > $(LOG_DIR)/$(@:memtest_%=test_%).memretcode

all_tests: $(ALL_TESTS)

all_memtests: $(ALL_MEMTESTS)

interm_clean: all_tests all_memtests
	@rm -f fail
	@rm -f pcount
	@rm -f fcount
	@rm -f lcount
	@touch pcount
	@touch fcount
	@touch lcount

summarize: interm_clean
	@$(foreach file, $(TESTCOMMAND), \
		if grep -q 0 $(LOG_DIR)/test_$(file).retcode; then \
			if grep -q 0 $(LOG_DIR)/test_$(file).memretcode; then \
				echo "$(GREEN)PASSED$(NC): $(file)"; \
				echo $(file) >> pcount; \
			else \
				echo "$(LRED)LEAKED$(NC): $(file)"; \
				echo $(file) >> lcount; \
			fi; \
		else \
			echo "$(RED)FAILED$(NC): $(file)"; \
			echo $(file) >> fail; \
				echo $(file) >> fcount; \
		fi; \
	)

runtest: summarize
	@echo $(words $(shell cat pcount)) "$(GREEN)passed$(NC)"
	@echo $(words $(shell cat fcount)) "$(RED)failures$(NC)"
	@echo $(words $(shell cat lcount)) "$(LRED)tests leaking$(NC)"
	@gcov *.gcda > gcov.log
	cp *.gcda $(LOG_DIR) && rm -f *.gcda
	cp *.gcno $(LOG_DIR) || :
	rm -f *.gcno
	cp *.gcov $(LOG_DIR) && rm -f *.gcov

test: runtest
	@if [ -f fail ]; then echo "Test failures detected!"; exit 1; fi

lightclean:
	rm -f *.gcda *.gcno *.gcov *.gcno *.gcov

clean: lightclean
	rm -f $(BIN_DIR)/* $(LOG_DIR)/* pcount fcount lcount

bigclean: clean
	cd mruby && make clean

distclean: bigclean
	rm -rf mruby
	rm -rf bins
	rm -rf logs

.SECONDARY:
.PHONY: distclean clean test
