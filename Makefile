 --------ͨ��makefile for gcc/g++/gnu cross compiler--------------------
# file		Makefile
# project	project description
# date		2016-01-08/12:03:58
# author	tracyone , tracyone@live.cn
# history
# 
# Recommand makefile rule:(��shell scriptͳһ)
# ����:�������͵Ⱥ�֮�䲻Ҫ�ӿո�,�û��������ñ���
# ����:��Բ���ŵ��ú���

# operating system name
OS_NAME:=$(shell uname -s)

# tools
AR:=ar
SED:=sed
AWK:=awk
CP:=cp -a
RM:=rm -rf
ECHO:=echo

# ���������ǰ׺...����˵arm-linux-
CROSS_COMPILE:=

# c compiler
CC:=${CROSS_COMPILE}gcc
# �����ѡ��
CFLAGS:=-c -Wall 
# ���ӵ�ѡ�-l��Ҫ���ӵĿ�,-staic��
CLDFLAGS:=

# cpp compiler
CXX:=${CROSS_COMPILE}g++
# �����ѡ��
CXXFLAGS:=-c -Wall 
# ���ӵ�ѡ�-l��Ҫ���ӵĿ�,-staic��
CXXLDFLAGS:=

# ����ʱָ���ĺ궨��:-Dѡ��
DFLAGS:=

# ���ͷ�ļ�������·��:-Iѡ���-L
INCULDES:=-I../list
LIBDIRS:=

# ָ��Դ����·�����Կո�ֿ�..
SRC_DIR:=. $(shell find . ! -path "." -type d | grep -Ev 'Debug|Release') ../list

# ָ��֧�ֵ�Դ������չ��
SFIX:=.c .cc .C .cpp

# ����Ҫ��װ��·��
prefix:=.

# ָ�������ļ�������·��
vpath %.cpp ${SRC_DIR}
vpath %.c ${SRC_DIR}
vpath %.C ${SRC_DIR}
vpath %.h ${SRC_DIR}

# ���尲װĿ¼
BIN_DIR=$(shell pwd)/Debug
# ���object���ļ���..
OBJDIR=${BIN_DIR}/objs
DEPENDDIR=${BIN_DIR}/dep

# �õ�Դ�ļ��ļ���
SOURCES:=$(foreach x,${SRC_DIR},\
			   $(wildcard $(addprefix ${x}/*,${SFIX})))

# ȥ��·����Ϣ�ͺ�׺,��׷��.o����չ��,�õ�Ŀ���ļ�����(����·��)����Ҫȥ��·����Ϣ
# ��������ʱ�����Ҳ���.o�ļ�
OBJS=$(addprefix ${OBJDIR}/,$(addsuffix .o,$(basename $(notdir ${SOURCES}))))

# �õ������ļ�������(��·��)
DEPENDS=$(addprefix ${DEPENDDIR}/,$(addsuffix .d,$(basename $(notdir ${SOURCES}))))

# ��������
PROGRAM:=test

# ���Ա�־�����Ϊ1��ô��صı���ѡ�������Ӧ�ĸı�
# �����ڵ���..�����1��ô����������Ż�..
DEBUG:=1

# αĿ�궨��
.PHONY:all clean distclean install print_info

# ����Ŀ��..
all:print_info ${PROGRAM} install

print_info:
	@${ECHO} "Do some setting......"
	@mkdir -p ${BIN_DIR}
	@mkdir -p ${OBJDIR}
	@mkdir -p ${DEPENDDIR}
ifeq ($(DEBUG),1)
	@${ECHO} "Setting project for debug"
CFLAGS+=-g
CXXFLAGS+=-g
BIN_DIR=./Debug
else
	@${ECHO} "Setting project for release"
CFLAGS+=-O2
CXXFLAGS+=-O2
BIN_DIR=./Release
endif


${PROGRAM}:${DEPENDS} ${OBJS}
ifeq ($(strip $(filter-out %.c,${SOURCES})),)
	${CC} -o ${BIN_DIR}/$@ ${LIBDIRS}  ${OBJS} ${CLDFLAGS}
else
	${CXX}  -o ${BIN_DIR}/$@ ${LIBDIRS} ${OBJS} ${CXXLDFLAGS}
endif

# rule for .o
${OBJDIR}/%.o:%.c
	${CC} $< -o $@ ${DFLAGS} ${CFLAGS} ${INCULDES} 

${OBJDIR}/%.o:%.cc
	${CXX}  $< -o $@ ${DFLAGS} ${CXXFLAGS} ${INCULDES}

${OBJDIR}/%.o:%.cpp
	${CXX}  $< -o $@ ${DFLAGS} ${CXXFLAGS} ${INCULDES}

${OBJDIR}/%.o:%.C
	${CXX}  $< -o $@ ${DFLAGS} ${CFLAGS} ${INCULDES}

# rule for .d
${DEPENDDIR}/%.d:%.c
	@${CC} -MM -MD ${INCULDES} $< -o $@

${DEPENDDIR}/%.d:%.cc
	@${CXX} -MM -MD ${INCULDES} $< -o $@

${DEPENDDIR}/%.d:%.cpp
	@${CXX} -MM -MD ${INCULDES} $< -o $@

${DEPENDDIR}/%.d:%.C
	@${CC} -MM -MD ${INCULDES} $< -o $@

clean:
	-${RM}  ${BIN_DIR}/${PROGRAM}
	-${RM}  ${OBJDIR}/*.o
	-${RM}  ${DEPENDDIR}/*.d
	-${RM}  ./Debug
	-${RM}  ./Release
	-${RM}  ${PROGRAM}
	-${RM}  ${prefix}/${PROGRAM}

install:
	@${ECHO} Starting install......
	install -v ${BIN_DIR}/${PROGRAM} ${prefix}

