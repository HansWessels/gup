
# bin\Debug-Unicode-32bit-x86\!Debug!x86!32!x86!Win32!Win32
EXEDIR=$(GUP_EXE_DIR)..\..\bin\$(CFG)-Unicode-$(PLATFORM)bit-$(ARCH)\\
TESTDATADIR=$(GUP_EXE_DIR)..\..\..\..\test\calgary-corpus\\

GUP=$(EXEDIR)gup.exe

all: hello test_help test_pack_round

hello:
	@echo "hello [$(EXEDIR)] [$(GUP_EXE_DIR)] [$(CFG)] [$(ARCH)] [$(PLATFORM)] [$(REST)]"
	cd

test_help:
	echo "test help output [$(GUP)]"
	$(GUP)
#	$(GUP) -?

test_pack_round:
	$(GUP) a -r -e test.arj $(TESTDATADIR)*


	


.PHONY: all

