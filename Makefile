CC := g++
CFLAGS := -std=c++11 -I. -I./mruby/include -g -Wall -fprofile-arcs -ftest-coverage
LDFLAGS := -Lmruby/build/host/lib/ -lmruby
TEST_DIR := tests
BIN_DIR := bins
LOG_DIR := logs
OBJ_DIR := objs
LIBMRUBY := mruby/build/host/lib/libmruby.a

SOURCES := $(wildcard *.hpp)
TESTS := $(wildcard $(TEST_DIR)/*.cpp)
BINS := $(patsubst $(TEST_DIR)/%.cpp, $(BIN_DIR)/test_%, $(TESTS))

ALL_TEST_RESULTS := $(patsubst $(TEST_DIR)/%.cpp, $(LOG_DIR)/test_%, $(TESTS))
ALL_COUNTS := $(patsubst $(TEST_DIR)/%.cpp, %_ccount, $(TESTS))

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

$(LOG_DIR):
	@mkdir -p $(LOG_DIR)

$(LOG_DIR)/test_%: $(BIN_DIR)/test_%
	@mkdir -p $@
	@echo "TESTING $(@:$(LOG_DIR)/test_%=test_%)"
	@$(BIN_DIR)/$(@:$(LOG_DIR)/test_%=test_%) 1> $@/test.stdout 2> $@/test.stderr; echo "$$?" > $@/test.retcode
	@echo "LEAKCHK $(@:test_%=memtest_%)"
	@valgrind --error-exitcode=1 --leak-check=full --log-file=$@/valgrind.log $(BIN_DIR)/$(@:$(LOG_DIR)/%=%) 1> $@/valgrind.stdout 2> $@/valgrind.stderr; echo "$$?" > $@/valgrind.retcode
	@mv $(@:$(LOG_DIR)/test_%=%).gc* $(LOG_DIR)

all_tests: $(ALL_TEST_RESULTS)

count_files:
	@rm -f fail
	@rm -f pcount
	@rm -f fcount
	@rm -f lcount
	@touch pcount
	@touch fcount
	@touch lcount

%_ccount: count_files all_tests
	@if grep -q 0 $(LOG_DIR)/$(@:%_ccount=test_%)/test.retcode; then \
		if grep -q 0 $(LOG_DIR)/$(@:%_ccount=test_%)/valgrind.retcode; then \
			echo "$(GREEN)PASSED$(NC): $@"; \
			echo $@ >> pcount; \
		else \
			echo "$(LRED)LEAKED$(NC): $@)"; \
			echo $@ >> lcount; \
			cat $(LOG_DIR)/$(@:%_ccount=test_%)/valgrind.log; \
			cat $(LOG_DIR)/$(@:%_ccount=test_%)/valgrind.stdout; \
			cat $(LOG_DIR)/$(@:%_ccount=test_%)/valgrind.stderr; \
			touch fail; \
		fi; \
	else \
		echo "$(RED)FAILED$(NC): $@"; \
		cat $(LOG_DIR)/$(@:%_ccount=test_%)/test.stderr; \
		touch fail; \
			echo $@ >> fcount; \
	fi; \

all_counts: $(ALL_COUNTS)

summary: all_counts
	@echo $(words $(shell cat pcount)) "$(GREEN)passed$(NC)"
	@echo $(words $(shell cat fcount)) "$(RED)failing$(NC)"
	@echo $(words $(shell cat lcount)) "$(LRED)leaking$(NC)"
	@rm -f pcount
	@rm -f fcount
	@rm -f lcount
	ls

gcov.log: all_tests
	@gcov $(LOG_DIR)/*.gcda > gcov.log

test: summary
	@if [ -f fail ]; then echo "Test failures detected!"; exit 1; fi

lightclean:
	rm -f *.gcda *.gcno *.gcov *.gcno *.gcov

clean: lightclean
	rm -rf $(BIN_DIR)/* $(LOG_DIR)/* pcount fcount lcount

bigclean: clean
	cd mruby && make clean

distclean: bigclean
	rm -rf bins
	rm -rf logs

.SECONDARY:
.PHONY: distclean clean test
