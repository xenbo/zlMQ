# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.7

# Default target executed when no arguments are given to make.
default_target: all

.PHONY : default_target

# Allow only one "make -f Makefile2" at a time, but pass parallelism.
.NOTPARALLEL:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /home/dongbo01/clion-2017.1.3/bin/cmake/bin/cmake

# The command to remove a file.
RM = /home/dongbo01/clion-2017.1.3/bin/cmake/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/dongbo01/CLionProjects/MQSvr_2

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/dongbo01/CLionProjects/MQSvr_2

#=============================================================================
# Targets provided globally by CMake.

# Special rule for the target rebuild_cache
rebuild_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake to regenerate build system..."
	/home/dongbo01/clion-2017.1.3/bin/cmake/bin/cmake -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : rebuild_cache

# Special rule for the target rebuild_cache
rebuild_cache/fast: rebuild_cache

.PHONY : rebuild_cache/fast

# Special rule for the target edit_cache
edit_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "No interactive CMake dialog available..."
	/home/dongbo01/clion-2017.1.3/bin/cmake/bin/cmake -E echo No\ interactive\ CMake\ dialog\ available.
.PHONY : edit_cache

# Special rule for the target edit_cache
edit_cache/fast: edit_cache

.PHONY : edit_cache/fast

# The main all target
all: cmake_check_build_system
	$(CMAKE_COMMAND) -E cmake_progress_start /home/dongbo01/CLionProjects/MQSvr_2/CMakeFiles /home/dongbo01/CLionProjects/MQSvr_2/CMakeFiles/progress.marks
	$(MAKE) -f CMakeFiles/Makefile2 all
	$(CMAKE_COMMAND) -E cmake_progress_start /home/dongbo01/CLionProjects/MQSvr_2/CMakeFiles 0
.PHONY : all

# The main clean target
clean:
	$(MAKE) -f CMakeFiles/Makefile2 clean
.PHONY : clean

# The main clean target
clean/fast: clean

.PHONY : clean/fast

# Prepare targets for installation.
preinstall: all
	$(MAKE) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall

# Prepare targets for installation.
preinstall/fast:
	$(MAKE) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall/fast

# clear depends
depend:
	$(CMAKE_COMMAND) -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 1
.PHONY : depend

#=============================================================================
# Target rules for targets named MQSvr_2

# Build rule for target.
MQSvr_2: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 MQSvr_2
.PHONY : MQSvr_2

# fast build rule for target.
MQSvr_2/fast:
	$(MAKE) -f CMakeFiles/MQSvr_2.dir/build.make CMakeFiles/MQSvr_2.dir/build
.PHONY : MQSvr_2/fast

#=============================================================================
# Target rules for targets named zmqLevelDB

# Build rule for target.
zmqLevelDB: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 zmqLevelDB
.PHONY : zmqLevelDB

# fast build rule for target.
zmqLevelDB/fast:
	$(MAKE) -f CMakeFiles/zmqLevelDB.dir/build.make CMakeFiles/zmqLevelDB.dir/build
.PHONY : zmqLevelDB/fast

MQtoMQLBSvr.o: MQtoMQLBSvr.cpp.o

.PHONY : MQtoMQLBSvr.o

# target to build an object file
MQtoMQLBSvr.cpp.o:
	$(MAKE) -f CMakeFiles/MQSvr_2.dir/build.make CMakeFiles/MQSvr_2.dir/MQtoMQLBSvr.cpp.o
.PHONY : MQtoMQLBSvr.cpp.o

MQtoMQLBSvr.i: MQtoMQLBSvr.cpp.i

.PHONY : MQtoMQLBSvr.i

# target to preprocess a source file
MQtoMQLBSvr.cpp.i:
	$(MAKE) -f CMakeFiles/MQSvr_2.dir/build.make CMakeFiles/MQSvr_2.dir/MQtoMQLBSvr.cpp.i
.PHONY : MQtoMQLBSvr.cpp.i

MQtoMQLBSvr.s: MQtoMQLBSvr.cpp.s

.PHONY : MQtoMQLBSvr.s

# target to generate assembly for a file
MQtoMQLBSvr.cpp.s:
	$(MAKE) -f CMakeFiles/MQSvr_2.dir/build.make CMakeFiles/MQSvr_2.dir/MQtoMQLBSvr.cpp.s
.PHONY : MQtoMQLBSvr.cpp.s

Svr.o: Svr.cpp.o

.PHONY : Svr.o

# target to build an object file
Svr.cpp.o:
	$(MAKE) -f CMakeFiles/MQSvr_2.dir/build.make CMakeFiles/MQSvr_2.dir/Svr.cpp.o
.PHONY : Svr.cpp.o

Svr.i: Svr.cpp.i

.PHONY : Svr.i

# target to preprocess a source file
Svr.cpp.i:
	$(MAKE) -f CMakeFiles/MQSvr_2.dir/build.make CMakeFiles/MQSvr_2.dir/Svr.cpp.i
.PHONY : Svr.cpp.i

Svr.s: Svr.cpp.s

.PHONY : Svr.s

# target to generate assembly for a file
Svr.cpp.s:
	$(MAKE) -f CMakeFiles/MQSvr_2.dir/build.make CMakeFiles/MQSvr_2.dir/Svr.cpp.s
.PHONY : Svr.cpp.s

crc64.o: crc64.cpp.o

.PHONY : crc64.o

# target to build an object file
crc64.cpp.o:
	$(MAKE) -f CMakeFiles/MQSvr_2.dir/build.make CMakeFiles/MQSvr_2.dir/crc64.cpp.o
.PHONY : crc64.cpp.o

crc64.i: crc64.cpp.i

.PHONY : crc64.i

# target to preprocess a source file
crc64.cpp.i:
	$(MAKE) -f CMakeFiles/MQSvr_2.dir/build.make CMakeFiles/MQSvr_2.dir/crc64.cpp.i
.PHONY : crc64.cpp.i

crc64.s: crc64.cpp.s

.PHONY : crc64.s

# target to generate assembly for a file
crc64.cpp.s:
	$(MAKE) -f CMakeFiles/MQSvr_2.dir/build.make CMakeFiles/MQSvr_2.dir/crc64.cpp.s
.PHONY : crc64.cpp.s

main.o: main.cpp.o

.PHONY : main.o

# target to build an object file
main.cpp.o:
	$(MAKE) -f CMakeFiles/MQSvr_2.dir/build.make CMakeFiles/MQSvr_2.dir/main.cpp.o
.PHONY : main.cpp.o

main.i: main.cpp.i

.PHONY : main.i

# target to preprocess a source file
main.cpp.i:
	$(MAKE) -f CMakeFiles/MQSvr_2.dir/build.make CMakeFiles/MQSvr_2.dir/main.cpp.i
.PHONY : main.cpp.i

main.s: main.cpp.s

.PHONY : main.s

# target to generate assembly for a file
main.cpp.s:
	$(MAKE) -f CMakeFiles/MQSvr_2.dir/build.make CMakeFiles/MQSvr_2.dir/main.cpp.s
.PHONY : main.cpp.s

zmqLevelDB/getcmdFromzmq.o: zmqLevelDB/getcmdFromzmq.cpp.o

.PHONY : zmqLevelDB/getcmdFromzmq.o

# target to build an object file
zmqLevelDB/getcmdFromzmq.cpp.o:
	$(MAKE) -f CMakeFiles/zmqLevelDB.dir/build.make CMakeFiles/zmqLevelDB.dir/zmqLevelDB/getcmdFromzmq.cpp.o
.PHONY : zmqLevelDB/getcmdFromzmq.cpp.o

zmqLevelDB/getcmdFromzmq.i: zmqLevelDB/getcmdFromzmq.cpp.i

.PHONY : zmqLevelDB/getcmdFromzmq.i

# target to preprocess a source file
zmqLevelDB/getcmdFromzmq.cpp.i:
	$(MAKE) -f CMakeFiles/zmqLevelDB.dir/build.make CMakeFiles/zmqLevelDB.dir/zmqLevelDB/getcmdFromzmq.cpp.i
.PHONY : zmqLevelDB/getcmdFromzmq.cpp.i

zmqLevelDB/getcmdFromzmq.s: zmqLevelDB/getcmdFromzmq.cpp.s

.PHONY : zmqLevelDB/getcmdFromzmq.s

# target to generate assembly for a file
zmqLevelDB/getcmdFromzmq.cpp.s:
	$(MAKE) -f CMakeFiles/zmqLevelDB.dir/build.make CMakeFiles/zmqLevelDB.dir/zmqLevelDB/getcmdFromzmq.cpp.s
.PHONY : zmqLevelDB/getcmdFromzmq.cpp.s

zmqLevelDB/job.o: zmqLevelDB/job.cpp.o

.PHONY : zmqLevelDB/job.o

# target to build an object file
zmqLevelDB/job.cpp.o:
	$(MAKE) -f CMakeFiles/zmqLevelDB.dir/build.make CMakeFiles/zmqLevelDB.dir/zmqLevelDB/job.cpp.o
.PHONY : zmqLevelDB/job.cpp.o

zmqLevelDB/job.i: zmqLevelDB/job.cpp.i

.PHONY : zmqLevelDB/job.i

# target to preprocess a source file
zmqLevelDB/job.cpp.i:
	$(MAKE) -f CMakeFiles/zmqLevelDB.dir/build.make CMakeFiles/zmqLevelDB.dir/zmqLevelDB/job.cpp.i
.PHONY : zmqLevelDB/job.cpp.i

zmqLevelDB/job.s: zmqLevelDB/job.cpp.s

.PHONY : zmqLevelDB/job.s

# target to generate assembly for a file
zmqLevelDB/job.cpp.s:
	$(MAKE) -f CMakeFiles/zmqLevelDB.dir/build.make CMakeFiles/zmqLevelDB.dir/zmqLevelDB/job.cpp.s
.PHONY : zmqLevelDB/job.cpp.s

zmqLevelDB/opleveldb.o: zmqLevelDB/opleveldb.cpp.o

.PHONY : zmqLevelDB/opleveldb.o

# target to build an object file
zmqLevelDB/opleveldb.cpp.o:
	$(MAKE) -f CMakeFiles/zmqLevelDB.dir/build.make CMakeFiles/zmqLevelDB.dir/zmqLevelDB/opleveldb.cpp.o
.PHONY : zmqLevelDB/opleveldb.cpp.o

zmqLevelDB/opleveldb.i: zmqLevelDB/opleveldb.cpp.i

.PHONY : zmqLevelDB/opleveldb.i

# target to preprocess a source file
zmqLevelDB/opleveldb.cpp.i:
	$(MAKE) -f CMakeFiles/zmqLevelDB.dir/build.make CMakeFiles/zmqLevelDB.dir/zmqLevelDB/opleveldb.cpp.i
.PHONY : zmqLevelDB/opleveldb.cpp.i

zmqLevelDB/opleveldb.s: zmqLevelDB/opleveldb.cpp.s

.PHONY : zmqLevelDB/opleveldb.s

# target to generate assembly for a file
zmqLevelDB/opleveldb.cpp.s:
	$(MAKE) -f CMakeFiles/zmqLevelDB.dir/build.make CMakeFiles/zmqLevelDB.dir/zmqLevelDB/opleveldb.cpp.s
.PHONY : zmqLevelDB/opleveldb.cpp.s

zmqLevelDB/pushout.o: zmqLevelDB/pushout.cpp.o

.PHONY : zmqLevelDB/pushout.o

# target to build an object file
zmqLevelDB/pushout.cpp.o:
	$(MAKE) -f CMakeFiles/zmqLevelDB.dir/build.make CMakeFiles/zmqLevelDB.dir/zmqLevelDB/pushout.cpp.o
.PHONY : zmqLevelDB/pushout.cpp.o

zmqLevelDB/pushout.i: zmqLevelDB/pushout.cpp.i

.PHONY : zmqLevelDB/pushout.i

# target to preprocess a source file
zmqLevelDB/pushout.cpp.i:
	$(MAKE) -f CMakeFiles/zmqLevelDB.dir/build.make CMakeFiles/zmqLevelDB.dir/zmqLevelDB/pushout.cpp.i
.PHONY : zmqLevelDB/pushout.cpp.i

zmqLevelDB/pushout.s: zmqLevelDB/pushout.cpp.s

.PHONY : zmqLevelDB/pushout.s

# target to generate assembly for a file
zmqLevelDB/pushout.cpp.s:
	$(MAKE) -f CMakeFiles/zmqLevelDB.dir/build.make CMakeFiles/zmqLevelDB.dir/zmqLevelDB/pushout.cpp.s
.PHONY : zmqLevelDB/pushout.cpp.s

zmqLevelDB/zmqpipe.o: zmqLevelDB/zmqpipe.cpp.o

.PHONY : zmqLevelDB/zmqpipe.o

# target to build an object file
zmqLevelDB/zmqpipe.cpp.o:
	$(MAKE) -f CMakeFiles/zmqLevelDB.dir/build.make CMakeFiles/zmqLevelDB.dir/zmqLevelDB/zmqpipe.cpp.o
.PHONY : zmqLevelDB/zmqpipe.cpp.o

zmqLevelDB/zmqpipe.i: zmqLevelDB/zmqpipe.cpp.i

.PHONY : zmqLevelDB/zmqpipe.i

# target to preprocess a source file
zmqLevelDB/zmqpipe.cpp.i:
	$(MAKE) -f CMakeFiles/zmqLevelDB.dir/build.make CMakeFiles/zmqLevelDB.dir/zmqLevelDB/zmqpipe.cpp.i
.PHONY : zmqLevelDB/zmqpipe.cpp.i

zmqLevelDB/zmqpipe.s: zmqLevelDB/zmqpipe.cpp.s

.PHONY : zmqLevelDB/zmqpipe.s

# target to generate assembly for a file
zmqLevelDB/zmqpipe.cpp.s:
	$(MAKE) -f CMakeFiles/zmqLevelDB.dir/build.make CMakeFiles/zmqLevelDB.dir/zmqLevelDB/zmqpipe.cpp.s
.PHONY : zmqLevelDB/zmqpipe.cpp.s

# Help Target
help:
	@echo "The following are some of the valid targets for this Makefile:"
	@echo "... all (the default if no target is provided)"
	@echo "... clean"
	@echo "... depend"
	@echo "... rebuild_cache"
	@echo "... edit_cache"
	@echo "... MQSvr_2"
	@echo "... zmqLevelDB"
	@echo "... MQtoMQLBSvr.o"
	@echo "... MQtoMQLBSvr.i"
	@echo "... MQtoMQLBSvr.s"
	@echo "... Svr.o"
	@echo "... Svr.i"
	@echo "... Svr.s"
	@echo "... crc64.o"
	@echo "... crc64.i"
	@echo "... crc64.s"
	@echo "... main.o"
	@echo "... main.i"
	@echo "... main.s"
	@echo "... zmqLevelDB/getcmdFromzmq.o"
	@echo "... zmqLevelDB/getcmdFromzmq.i"
	@echo "... zmqLevelDB/getcmdFromzmq.s"
	@echo "... zmqLevelDB/job.o"
	@echo "... zmqLevelDB/job.i"
	@echo "... zmqLevelDB/job.s"
	@echo "... zmqLevelDB/opleveldb.o"
	@echo "... zmqLevelDB/opleveldb.i"
	@echo "... zmqLevelDB/opleveldb.s"
	@echo "... zmqLevelDB/pushout.o"
	@echo "... zmqLevelDB/pushout.i"
	@echo "... zmqLevelDB/pushout.s"
	@echo "... zmqLevelDB/zmqpipe.o"
	@echo "... zmqLevelDB/zmqpipe.i"
	@echo "... zmqLevelDB/zmqpipe.s"
.PHONY : help



#=============================================================================
# Special targets to cleanup operation of make.

# Special rule to run CMake to check the build system integrity.
# No rule that depends on this can have commands that come from listfiles
# because they might be regenerated.
cmake_check_build_system:
	$(CMAKE_COMMAND) -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 0
.PHONY : cmake_check_build_system

