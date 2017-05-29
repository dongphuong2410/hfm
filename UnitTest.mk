TEST_FILE_LIST=$(shell cd $(TESTDIR); ls | egrep '^test[0-9]+$$' | sort -n)
CMP=$(TESTDIR)/cmp
TMP=$(TESTDIR)/tmp

ready: $(CMP) $(TMP)

$(TMP):
	@ - [ ! -d $(TMP) ] && mkdir $(TMP)

run : ready $(TESTDIR)/$(X)
	@cd $(TESTDIR); ./$(X)

test_one: ready $(CMP)/$(X).want
	@echo "Testing $X"
	@$(MAKE) --no-print-directory run > $(TMP)/$X.got
	@if diff -s $(TMP)/$X.got $(CMP)/$X.want > /dev/null;    \
		then echo PASSED $X ; \
		else echo FAILED $X, got $(TMP)/$X.got;    \
	fi

newcache: ready
	@$(MAKE) --no-print-directory run > $(CMP)/$X.want
	@echo New test result cached to $(CMP)/$X.want

test:; @$(foreach x, $(TEST_FILE_LIST), $(MAKE) X=$x test_one;) 

cache: 
	@$(foreach x, $(TEST_FILE_LIST), $(MAKE) X=$x newcache;)

score:
	@$(MAKE) test  | cut -d' ' -f1 | egrep '(PASSED|FAILED)' | sort | uniq -c

