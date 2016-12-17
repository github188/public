###### Record 进程 ########
# 系统平台
ARCH = $(shell arch)
BITS =
ifeq "$(findstring x86_64,$(ARCH))" "x86_64"
BITS = 64
endif
#开关
ON = 1
OFF = 0

#版本/
VERSION =


#日期
#DATE=$(shell date +%Y%m%d%H%M%S)

# 目标文件名字
#APP = control_$(VERSION)_$(ARCH)
APP = record
BIN_DIR = bin
$(shell mkdir -p $(BIN_DIR)) 
TARGET =$(BIN_DIR)/$(APP)


# c源文件
SRC = $(wildcard ./*.c)\
 $(wildcard ../share/xml/*.c)\
 $(wildcard ../share/device/*.c)\
 $(wildcard ../share/curl$(BIT)/*.c)\
 $(wildcard ../share/http/*.c)\
 $(wildcard ../share/middle/*.c)\
 $(wildcard ../share/md5/*.c)\
 $(wildcard ../share/communtication/*.c)\
 $(wildcard ../share/nslog/*.c)\
 $(wildcard ../share/libmedia/*.c)\
 $(wildcard ../share/*.c)\
 $(wildcard ../share/list/*.c)\
 $(wildcard ../share/socket/*.c)\
 $(wildcard ../share/stream/*.c)\
 $(wildcard ../share/timer/*.c)\
 $(wildcard ../share/network/*.c)\
 $(wildcard ../share/database/*.c)
# object文件i
OBJS = $(SRC:.c=.o)
OBJS_DIR = objs
$(shell mkdir -p $(OBJS_DIR)) 
#OBJS_DEST = $(wildcard $(OBJS_DIR)/*.o)
EXTERN_OBJ :=  $(foreach n,$(notdir $(OBJS)),$(OBJS_DIR)/$(n))


# 编译器
CC = /opt/CodeSourcery/Sourcery_G++_Lite/bin/arm-none-linux-gnueabi-gcc
STRIP = /opt/CodeSourcery/Sourcery_G++_Lite/bin/arm-none-linux-gnueabi-strip

# 头文件路径，可以任意指定头文件
INCLUDE := -I./\
 -I../share\
 -I../share/middle\
 -I../share/xml\
 -I../share/device\
 -I../share/curl$(BITS)\
 -I../share/http\
 -I../share/iconv\
 -I../share/communtication\
 -I../share/include\
 -I../share/socket\
 -I../share/stream\
 -I../share/nslog\
 -I../share/md5\
-I../share/include\
 -I../share/list\
 -I../share/database\
 -I../share/timer\
 -I../share/xmlconfig/tinyxmlmgr\
 -I../share/libmedia\
 -I../share/network\
 -I../share/rtspserver/mediaServer\
 -I../share/database

# 链接库
#LIBS := -lpthread -lrt -lm -lstdc++
LIBS := -lpthread -lrt -lm -ldl

#LOCAL_LIBS := ../../lib/libxml2.a   
#LOCAL_LIBS += ../../lib/libz.a
#LOCAL_LIBS += ../../lib/libcurl.a
#LOCAL_LIBS += ../../lib/libzlog.a
#LOCAL_LIBS := $(wildcard ../../lib$(BITS)/*.a)
LOCAL_LIBS +=-L ../../lib
LOCAL_LIBS +=-L ../../bin
LOCAL_LIBS += ../../lib/libavcodec.a
LOCAL_LIBS += ../../lib/libswresample.a
LOCAL_LIBS += ../../lib/libcrypto.a
LOCAL_LIBS += ../../lib/libavdevice.a
LOCAL_LIBS += ../../lib/libswscale.a
LOCAL_LIBS += ../../lib/libavdevice.a
LOCAL_LIBS += ../../lib/libavfilter.a
LOCAL_LIBS += ../../lib/libavformat.a
LOCAL_LIBS += ../../lib/libavutil.a
LOCAL_LIBS += ../../lib/libEasyAACEncoder.a
LOCAL_LIBS += ../../lib/libiconv.so.2.5.1
LOCAL_LIBS += ../../lib/libxml2.a
LOCAL_LIBS += ../../lib/libzlog.a
LOCAL_LIBS += ../../lib/libz.a
LOCAL_LIBS += ../../lib/libcurl.a
LOCAL_LIBS += -ltixml
LOCAL_LIBS += -ltixmlconfig
LOCAL_LIBS += -lRtspServer


LIBS += '-Wl,--start-group' $(LOCAL_LIBS) '-Wl,--end-group'

# 编译选项
CFLAGS = -g -Wall -D_FILE_OFFSET_BITS=64

#-Werror
#CFLAGS := -O2 -Wall -Wno-strict-aliasing -g
#CFLAGS = -O2
all: $(TARGET)

# 定义目标文件生成规则
$(TARGET):FIRST $(OBJS)
	echo #$(OBJS)#
	echo #$(EXTERN_OBJ)#
	$(CC) -o $(TARGET) $(EXTERN_OBJ) $(LIBS)

FIRST:
	rm -rf $(OBJS)
	
.SUFFIXES:.c	
# 宏定义
# 定义obejct文件生成规则
.c.o:
	$(CC) $(INCLUDE) $(CFLAGS) -c $<
	@mv *.o $(OBJS_DIR)
# 打安装包
pkgs:
	mkdir -p pkg/$(APP)
	cp -rf $(TARGET) pkg/$(APP)
	cp -rf pkg/$(SERVICE_TYPE)d pkg/$(APP)
	cp -rf pkg/$(TARGET)d pkg/$(APP)
	cp -rf pkg/install_$(SERVICE_TYPE) pkg/$(APP)
	cp -rf pkg/uninstall_$(SERVICE_TYPE) pkg/$(APP)
	cp -rf pkg/readme_$(SERVICE_TYPE) pkg/$(APP)
	cp -rf pkg/cfg_$(SERVICE_TYPE).xml pkg/$(APP)
	cp -rf pkg/nslog_$(SERVICE_TYPE).conf pkg/$(APP)
	echo $(APP) > pkg/version
	cp -rf pkg/version pkg/$(APP)
	cd pkg;tar cfvz $(APP).build$(DATE).tgz $(APP);rm -rf $(APP)
	cd pkg;cat inst_script.sh $(APP).build$(DATE).tgz > $(APP)_intaller;chmod +x $(APP)_intaller

# 定义清理函数
.PHONY:clean
install:
	$(STRIP) ./bin/record
	cp ./bin/record  /home/ydl/workdir/rfs/opt/dvr_rdk/ti816x_2.8/bin
clean:
	@rm -rf $(OBJS_DIR) $(TARGET)
	@rm -rf pkg/$(APP)*
	@rm -rf $(OBJS_DIR)
	@rm -rf $(BIN_DIR)
	@rm -rf version.h
	@rm -rf control_config.xml log_file
