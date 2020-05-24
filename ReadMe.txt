1 程序编译
cd data-acqusition-project
make clean
make

2 程序加载
cd ./bin
打开三个窗口

第一个窗口
1) insmod driver.ko
    ./daq_function

第二个窗口
2）./configuration_management

第三个窗口
3)./tcp_client  ip  port
例子：./tcp_client  192.168.1.106 20001