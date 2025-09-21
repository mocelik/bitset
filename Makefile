
BUILD_DIR    := $(shell pwd)/build
BUILD_MIRROR := ${BUILD_DIR}/objs
LIB_DIR      := ${BUILD_DIR}/lib
INCLUDE_DIR  := ${BUILD_DIR}/include
GTEST_TARGET := ${INCLUDE_DIR}/gtest

TEST_APP  := ${BUILD_DIR}/gtest_bitset
TEST_SRCS := test/test.cpp
TEST_OBJS := ${BUILD_MIRROR}/${TEST_SRCS:.cpp=.o}

.PHONY: test all
all: test
test: ${TEST_APP}
	@${TEST_APP}

CFLAGS  := -I${INCLUDE_DIR} -g -Og -std=c++17 --coverage
LDFLAGS := -L${LIB_DIR}
LDLIBS  := -lgtest -lgtest_main
CXX     := bear --append --output ${BUILD_DIR}/compile_commands.json -- ${CXX}

${TEST_APP}: ${TEST_OBJS} | ${BUILD_DIR}
	${CXX} ${LDFLAGS} ${CFLAGS} -o $@ $^ -I${INCLUDE_DIR} ${LDLIBS}

${TEST_OBJS}: ${BUILD_MIRROR}/%.o : %.cpp | ${BUILD_MIRROR} ${GTEST_TARGET}
	@mkdir -p $(dir $@)
	${CXX} -c -MMD -MP ${CFLAGS} -o $@ $<

${BUILD_DIR} ${BUILD_MIRROR} ${INCLUDE_DIR} ${LIB_DIR}:
	@mkdir -p $@

coverage: test
	@gcovr -e '/.*/build/' -e '/.*/test/' --html-nested build/coverage.html --sonarqube build/coverage.xml
	@echo "Coverage file is ${BUILD_DIR}/coverage.html"

clean:
	@rm -rf ${BUILD_DIR}

-include $(patsubst %.o,%.d,${TEST_OBJS})

### Download, build and "install" gtest into build directory
################################################################################

GTEST_BUILD_DIR := ${BUILD_DIR}/gtest
GTEST_TAR_URL := https://github.com/google/googletest/releases/download/v1.15.2/googletest-1.15.2.tar.gz
GTEST_TAR_FILE := ${GTEST_BUILD_DIR}/googletest-1.15.2.tar.gz

${GTEST_BUILD_DIR}:
	@mkdir -p $@

${INCLUDE_DIR}/gtest: | ${GTEST_BUILD_DIR} ${LIB_DIR} ${INCLUDE_DIR}
	@wget -q ${GTEST_TAR_URL} -O ${GTEST_TAR_FILE} 
	@tar -C ${GTEST_BUILD_DIR} --strip-components=1 -xzf ${GTEST_TAR_FILE} 
	@printf "Building gtest...\n"
	cmake -S ${GTEST_BUILD_DIR} -B ${GTEST_BUILD_DIR}
	cmake --build ${GTEST_BUILD_DIR} -j$(shell nproc) #-- --no-print-directory
	@cp ${GTEST_BUILD_DIR}/lib/* ${LIB_DIR}/
	@cp -r ${GTEST_BUILD_DIR}/googletest/include/* ${INCLUDE_DIR}
