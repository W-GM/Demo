PATH_LIBRARY=./src/
PATH_INCLUDE=./include/ 
PATH_INC_LIB=./include/library/
PATH_TEST=./test/
PATH_LIB=./lib/
PATH_BUILD=./build/
PATH_SRC_CONFIG=./src/config/

PATH_INC_LINUX=/home/wgm/文档/硬件资料/米尔/04-Source/MYiR-iMX-Linux/

all:
	g++ -g -DDEBUG -std=c++11 -Wall \
	-I$(PATH_INCLUDE) -I$(PATH_INC_LIB) -I$(PATH_INC_LINUX) \
	$(PATH_LIBRARY)multitask.cpp $(PATH_LIBRARY)uart.cpp $(PATH_LIBRARY)xbee_op.cpp \
	$(PATH_LIBRARY)xbee_request.cpp $(PATH_LIBRARY)xbee.cpp $(PATH_LIBRARY)xbee_response.cpp \
	$(PATH_LIBRARY)helper.cpp $(PATH_LIBRARY)sqlite_helper.cpp $(PATH_LIBRARY)i2c.c \
	$(PATH_LIBRARY)spi.c \
	$(PATH_SRC_CONFIG)tinyxml2.cpp \
	$(PATH_TEST)main_thread.cpp \
	-lpthread -lmodbus -lsqlite3 \
	-o $(PATH_BUILD)cq_main
	
arm:
	arm-linux-gnueabihf-g++ -g -DDEBUG -std=c++1y -Wall \
	-I$(PATH_INCLUDE) \
	-lpthread -lmodbus -lsqlite3 \
	-L$(PATH_LIB) \
	$(PATH_LIBRARY)multitask.cpp $(PATH_LIBRARY)uart.cpp  $(PATH_LIBRARY)xbee_op.cpp \
	$(PATH_LIBRARY)xbee_request.cpp $(PATH_LIBRARY)xbee.cpp $(PATH_LIBRARY)xbee_response.cpp \
	$(PATH_LIBRARY)helper.cpp $(PATH_LIBRARY)sqlite_helper.cpp $(PATH_LIBRARY)i2c.c \
	$(PATH_LIBRARY)spi.c \
	$(PATH_SRC_CONFIG)tinyxml2.cpp \
	$(PATH_TEST)main_thread.cpp \
	-o $(PATH_BUILD)cq_arm_main
	
i2c:
	arm-linux-gnueabihf-g++ -g -DDEBUG -std=c++11 -Wall \
	-I$(PATH_INCLUDE) \
	$(PATH_LIBRARY)i2c.c \
	$(PATH_TEST)i2c_test.c \
	-o $(PATH_BUILD)rtc
	
spi:
	arm-linux-gnueabihf-g++ -g -DDEBUG -std=c++11 -Wall \
	-I$(PATH_INCLUDE) \
	$(PATH_LIBRARY)spi.c \
	$(PATH_TEST)spi_test.c \
	-o $(PATH_BUILD)adc

uart:
	arm-linux-gnueabihf-g++ -g -DDEBUG -std=c++11 -Wall \
	-I$(PATH_INCLUDE) \
	$(PATH_LIBRARY)uart.cpp \
	$(PATH_TEST)uart_write.cpp \
	-o $(PATH_BUILD)uart_wr

485:
	arm-linux-gnueabihf-g++ -g -DDEBUG -std=c++11 -Wall \
	-I$(PATH_INCLUDE) -I$(PATH_INC_LINUX) \
	$(PATH_TEST)rs485_test.cpp \
	-o $(PATH_BUILD)rs485

xbee_at:
	arm-linux-gnueabihf-g++ -g -DDEBUG -std=c++1y -Wall \
	-I$(PATH_INCLUDE) \
	-lpthread -lmodbus -lsqlite3 \
	-L$(PATH_LIB) \
	$(PATH_LIBRARY)uart.cpp  $(PATH_LIBRARY)xbee_op.cpp \
	$(PATH_LIBRARY)xbee_request.cpp $(PATH_LIBRARY)xbee.cpp $(PATH_LIBRARY)xbee_response.cpp \
	$(PATH_TEST)xbee_rx.cpp $(PATH_TEST)xbee_tx.cpp $(PATH_TEST)xbee_atcmd.cpp $(PATH_TEST)xbee_remote_atcmd.cpp \
	$(PATH_TEST)main_xbee_atCmd.cpp \
	-o $(PATH_BUILD)xbee_at