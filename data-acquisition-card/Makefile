export DEBUG_DEF = $(debug)
all:

	@echo "Build Driver" 
	@cd ./driver && make
	@echo
	@echo
	@echo "Build libdaq"
	@cd ./libdaq  && make
	@echo
	@echo
	@echo "Build test"
	@cd ./test  && make
	@echo
	@echo
clean:
	@echo "Clean driver"
	@cd ./driver && make clean
	@echo
	@echo
	@echo "Clean libdaq"
	@cd ./libdaq  && make clean
	@echo
	@echo
	@echo "Clean test"
	@cd ./test && make clean
	@echo
	@echo
