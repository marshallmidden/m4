#-----------------------------------------------------------------------------
.PHONY: all test
all test: ../crypt
	../crypt DSC_13838.cap DSC_13838.zip 13838 0
	../crypt DSC_13839.cap DSC_13839.zip 13839 0
	../crypt ICON-13838.cap ICON-13838.zip 13838 0
	../crypt ICON-13839.cap ICON-13839.zip 13839 0

	-mkdir unzip1
	-mkdir unzip2
	-mkdir unzip3
	-mkdir unzip4

	cd unzip1 ; unzip ../DSC_13838.zip
	cd unzip2 ; unzip ../DSC_13839.zip
	cd unzip3 ; unzip ../ICON-13838.zip
	cd unzip4 ; unzip ../ICON-13839.zip
#-----------------------------------------------------------------------------
.PHONY: ../crypt
../crypt:
	${MAKE} -C .. crypt
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -rf *.zip unzip[1234]
#-----------------------------------------------------------------------------
