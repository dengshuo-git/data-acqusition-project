all:
	@rm -fr ./lib
	@rm -fr ./bin
	@mkdir ./lib
	@mkdir ./bin
	@echo "1)Bulid data-acquisition-card" 
	@cd ./data-acquisition-card && make
	@echo
	@echo
	@echo "2)Build libsocket"
	@cd ./libsocket && make
	@echo
	@echo
	@echo "3)Build liblog"
	@cd ./liblog && make
	@echo
	@echo
	@echo "4)Build acquisition-storage-report"
	@cd ./acquisition-storage-report && make
	@echo
	@echo
	@echo "5)Build configuration-management"
	@cd ./configuration-management && make
	@echo
	@echo
	@echo "6)Build configuration-menu"
	@cd ./configuration-menu && make
	@echo
	@echo
	@echo "7)Build log"
	@cd ./log && make
	@echo
	@echo
	@echo "8)Build tcp-menu"
	@cd ./tcp-menu && make
	@echo
	@echo
clean:
	@rm -fr ./lib
	@rm -fr ./bin
	@echo "1)data-acquisition-card" 
	@cd ./data-acquisition-card && make clean
	@echo
	@echo
	@echo "2)Build libsocket"
	@cd ./libsocket && make clean
	@echo
	@echo
	@echo "3)Build liblog"
	@cd ./liblog && make clean
	@echo
	@echo
	@echo "4)Build acquisition-storage-report"
	@cd ./acquisition-storage-report && make clean
	@echo
	@echo
	@echo "5)Build configuration-management"
	@cd ./configuration-management && make clean
	@echo
	@echo
	@echo "6)Build configuration-menu"
	@cd ./configuration-menu && make clean
	@echo
	@echo
	@echo "7)Build log"
	@cd ./log && make clean
	@echo
	@echo
	@echo "7)Build liblog"
	@cd ./liblog && make clean
	@echo
	@echo
	@echo "8)Build tcp-menu"
	@cd ./tcp-menu && make clean
	@echo
	@echo
