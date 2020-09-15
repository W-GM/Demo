PATH_SRC=./src/
PATH_INCLUDE=./include/ 
PATH_INC_LIB=./include/library/
PATH_TEST=./test/
PATH_LIB=./lib/
PATH_BUILD=./build/
PATH_SRC_CONFIG=./src/config/
PATH_XXX=./xxx/

PATH_INC_LINUX=/home/wgm/文档/硬件资料/米尔/04-Source/MYiR-iMX-Linux/

all:
	g++ -g -DDEBUG -std=c++11 -Wall \
	-I$(PATH_INCLUDE) -I$(PATH_INC_LIB) -I$(PATH_INC_LINUX) \
	$(PATH_SRC)multitask.cpp $(PATH_SRC)uart.cpp $(PATH_SRC)xbee_op.cpp \
	$(PATH_SRC)xbee_request.cpp $(PATH_SRC)xbee.cpp $(PATH_SRC)xbee_response.cpp \
	$(PATH_SRC)helper.cpp $(PATH_SRC)sqlite_helper.cpp $(PATH_SRC)i2c.c \
	$(PATH_SRC)spi.c $(PATH_SRC)rs485.c \
	$(PATH_SRC_CONFIG)tinyxml2.cpp \
	$(PATH_TEST)main_thread.cpp \
	-lpthread -lmodbus -lsqlite3 \
	-o $(PATH_BUILD)cq_main
	
arm:
	arm-linux-gnueabihf-g++ -g -DDEBUG -DARMCQ -std=c++1y -Wall \
	-I$(PATH_INCLUDE) \
	-lpthread -lmodbus -lsqlite3 \
	-L$(PATH_LIB) \
	$(PATH_SRC)multitask.cpp $(PATH_SRC)uart.cpp  $(PATH_SRC)xbee_op.cpp \
	$(PATH_SRC)xbee_request.cpp $(PATH_SRC)xbee.cpp $(PATH_SRC)xbee_response.cpp \
	$(PATH_SRC)helper.cpp $(PATH_SRC)sqlite_helper.cpp $(PATH_SRC)i2c.c \
	$(PATH_SRC)spi.c $(PATH_SRC)rs485.c \
	$(PATH_SRC_CONFIG)tinyxml2.cpp \
	$(PATH_TEST)main_thread.cpp \
	-o $(PATH_BUILD)cq_arm_main
	
i2c:
	arm-linux-gnueabihf-g++ -g -DDEBUG -std=c++11 -Wall \
	-I$(PATH_INCLUDE) \
	$(PATH_SRC)i2c.c \
	$(PATH_TEST)i2c_test.c \
	-o $(PATH_BUILD)rtc
	
spi:
	arm-linux-gnueabihf-g++ -g -DDEBUG -std=c++11 -Wall \
	-I$(PATH_INCLUDE) \
	$(PATH_SRC)spi.c \
	$(PATH_TEST)spi_test.c \
	-o $(PATH_BUILD)adc

uart:
	arm-linux-gnueabihf-g++ -g -DDEBUG -std=c++11 -Wall \
	-I$(PATH_INCLUDE) \
	$(PATH_SRC)uart.cpp \
	$(PATH_TEST)uart_write.cpp \
	-o $(PATH_BUILD)uart_wr

485:
	gcc -g -DDEBUG -Wall \
	-I$(PATH_INCLUDE) \
	$(PATH_SRC)rs485.c \
	$(PATH_TEST)rs485-test.c \
	-o $(PATH_BUILD)rs485

485-arm:
	arm-linux-gnueabihf-gcc -g -DDEBUG -Wall \
	-I$(PATH_INCLUDE) \
	$(PATH_SRC)rs485.c \
	$(PATH_TEST)rs485-test.c \
	-o $(PATH_BUILD)rs485-arm

xbee:
	g++ -g -DDEBUG -std=c++1y -Wall \
	-I$(PATH_INCLUDE) \
	-L$(PATH_LIB) \
	$(PATH_SRC)uart.cpp  $(PATH_SRC)xbee_op.cpp $(PATH_SRC)i2c.c \
	$(PATH_SRC)xbee_request.cpp $(PATH_SRC)xbee.cpp $(PATH_SRC)xbee_response.cpp \
	$(PATH_TEST)xbee_test.cpp \
	-o $(PATH_BUILD)xbee-test

xbee-arm:
	arm-linux-gnueabihf-g++ -g -DDEBUG -DARMCQ -std=c++1y -Wall \
	-I$(PATH_INCLUDE) \
	-L$(PATH_LIB) \
	$(PATH_SRC)uart.cpp  $(PATH_SRC)xbee_op.cpp $(PATH_SRC)i2c.c \
	$(PATH_SRC)xbee_request.cpp $(PATH_SRC)xbee.cpp $(PATH_SRC)xbee_response.cpp \
	$(PATH_TEST)xbee_test.cpp \
	-o $(PATH_BUILD)xbee-test-arm

ttt:
	g++ -g -DDEBUG -DCURTIME -Wall \
	-I$(PATH_INCLUDE) \
	$(PATH_SRC)helper.cpp \
	$(PATH_XXX)test.cpp \
	-o $(PATH_XXX)ttt