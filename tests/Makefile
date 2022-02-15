LEVEL = ..

## change or use make TARGET=inputfilename
TARGET=$(filename)

## replace LLVMROOT and SROALIB as appropriate
NETID = `kunli3`
LLVMROOT = /Users/likun/Projects/VSCodeProjects/Course/CS526/llvm-8.0.1
SROALIB  = $(LEVEL)/build/pass/libSROA.so



LLVMGCC = $(LLVMROOT)/bin/clang
LLVMAS  = $(LLVMROOT)/bin/llvm-as
LLVMDIS = $(LLVMROOT)/bin/llvm-dis
LLVMOPT = $(LLVMROOT)/bin/opt


## Other choices: test or comparecfe (these will be provided later)
default: debug
DEBUGOPTS =  -verify -inline -mem2reg -simplifycfg -mem2reg -globalopt -globaldce -instcombine -adce -instcombine -simplifycfg

testsimple: debug
debug: $(TARGET).debug.bc
disassembly: $(TARGET).llvm.ll $(TARGET).debug.ll

## test (implement yourself) can be used to test out your solution with a known comparison target.
## I will use different pass pipelines during the actual testing.
## Please test your solution with different pass pipelines when using the test target.
test: $(TARGET).test.bc


.PRECIOUS: %.ll


%.ll: %.bc
	$(LLVMDIS) -f $<


%.llvm.bc: %.c
	$(LLVMGCC) -S -emit-llvm -o - $< | $(LLVMAS) -o=$@

%.debug.bc: %.llvm.bc
	$(LLVMOPT) $(DEBUGOPTS) < $< | \
	$(LLVMOPT) -load $(SROALIB) -scalarrepl-kunli3 -o=$@

%.test.bc: %.llvm.bc
	$(LLVMOPT) < $< | \
	$(LLVMOPT) -load $(SROALIB) -scalarrepl-kunli3 -o=$@

test_pass: $(TARGET).test.ll
	$(LLVMROOT)/bin/clang -Xclang -load -Xclang $(LEVEL)/build/pass/libSROA.* $(TARGET).c \
		&& echo "[function output]": \
		&& ./a.out \
		&& echo "[SROA test]": \
		&& FILE_NAME=${TARGET}.test.ll python3 test.py

clean: 
	$(RM) -f *.debug.bc *.test.bc *.llvm.bc *.ll *.out

.PHONY: test_pass
