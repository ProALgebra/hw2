TESTS=$(sort $(basename $(wildcard test*.lama)))

LAMAC=lamac

.PHONY: check $(TESTS)

check: $(TESTS)

$(TESTS): %: %.lama
	@echo $@
	@echo $<
	@echo $@.input 
	
	$(LAMAC) -b $< 
	cat $@.input | ../byterun/interpreter $@.bc > $@.log
	diff $@.log orig/$@.log

# cat $@.input | LAMA=../runtime $(LAMAC) -i $< > $@.log && diff $@.log orig/$@.log
# cat $@.input | LAMA=../runtime $(LAMAC) -ds -s $< > $@.log && diff $@.log orig/$@.log
# LAMA=../runtime $(LAMAC) $< && cat $@.input | ./$@ > $@.log && diff $@.log orig/$@.log

clean:
	$(RM) test*.log *.s *~ $(TESTS) *.i
