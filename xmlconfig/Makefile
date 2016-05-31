#CROSS=
#CROSS=arm-none-linux-gnueabi-
all:
	@cd tinyxml;	make CROSS=arm-none-linux-gnueabi-
	@cd tinyxml;	make install
	@cd tinyxmlmgr;	make CROSS=arm-none-linux-gnueabi-
	@cd tinyxmlmgr;	make install

clean:
	@cd tinyxml;  make  clean
	@cd tinyxmlmgr;       make  clean
