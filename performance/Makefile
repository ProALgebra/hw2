TESTS=$(sort $(basename $(wildcard *.lama)))

LAMAC=../src/lamac

.PHONY: check $(TESTS)

check: $(TESTS)

$(TESTS): %: %.lama
	@echo $@
	`which time` -f "Sort\t%U" lamac -i Sort.lama </dev/null
	@echo $@
	$(LAMAC)  $< && `which time` -f "$@\t%U" ./$@

clean:
	$(RM) test*.log *.s *~ $(TESTS) *.i
