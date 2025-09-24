
BUILD_DIR    := build
BUILD_MIRROR := ${BUILD_DIR}/objs
LIB_DIR      := ${BUILD_DIR}/lib
INCLUDE_DIRS := ${BUILD_DIR}/include include
GTEST_TARGET := ${BUILD_DIR}/include/gtest

TEST_APP  := ${BUILD_DIR}/gtest_bitset
TEST_SRCS := $(wildcard test/*.cpp)
TEST_OBJS := $(addprefix ${BUILD_MIRROR}/,${TEST_SRCS:.cpp=.o})

.PHONY: test all
all: test
test: ${TEST_APP}
	@${TEST_APP}

CXXFLAGS := $(addprefix -I,${INCLUDE_DIRS}) -g -std=c++14 --coverage
LDFLAGS  := -L${LIB_DIR}
LDLIBS   := -lgtest -lgtest_main
CXX      := bear --append --output ${BUILD_DIR}/compile_commands.json -- ${CXX}

${TEST_APP}: ${TEST_OBJS} | ${BUILD_DIR}
	${CXX} ${LDFLAGS} ${CXXFLAGS} -o $@ $^ ${LDLIBS}

${TEST_OBJS}: ${BUILD_MIRROR}/%.o : %.cpp | ${BUILD_MIRROR} ${GTEST_TARGET}
	@mkdir -p $(dir $@)
	${CXX} -c -MMD -MP ${CXXFLAGS} -o $@ $<

${BUILD_DIR} ${BUILD_MIRROR} ${INCLUDE_DIRS} ${LIB_DIR}:
	@mkdir -p $@

coverage: test
	@gcovr  --exclude build/ \
			--exclude test/ \
			--exclude-unreachable-branches \
			--exclude-noncode-lines \
			--exclude-throw-branches \
			--gcov-delete \
			--print-summary \
			--html-nested build/coverage.html --sonarqube build/coverage.xml
	@echo "Coverage file is ${BUILD_DIR}/coverage.html"

sonarqube: coverage build/dummy_app
 # run the dummy app to get some
	@./build/dummy_app

build/dummy_app: src/main.cpp
	${CXX} ${CXXFLAGS} -o $@ $<

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

${GTEST_TARGET}: | ${GTEST_BUILD_DIR} ${LIB_DIR} ${INCLUDE_DIRS}
	@wget -q ${GTEST_TAR_URL} -O ${GTEST_TAR_FILE} 
	@tar -C ${GTEST_BUILD_DIR} --strip-components=1 -xzf ${GTEST_TAR_FILE} 
	@printf "Building gtest...\n"
	cmake -S ${GTEST_BUILD_DIR} -B ${GTEST_BUILD_DIR}
	cmake --build ${GTEST_BUILD_DIR} -j$(shell nproc) #-- --no-print-directory
	@cp ${GTEST_BUILD_DIR}/lib/* ${LIB_DIR}/
	@cp -r ${GTEST_BUILD_DIR}/googletest/include/* $(dir ${GTEST_TARGET})
