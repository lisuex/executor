# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.18

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Disable VCS-based implicit rules.
% : %,v


# Disable VCS-based implicit rules.
% : RCS/%


# Disable VCS-based implicit rules.
% : RCS/%,v


# Disable VCS-based implicit rules.
% : SCCS/s.%


# Disable VCS-based implicit rules.
% : s.%


.SUFFIXES: .hpux_make_needs_suffix_list


# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/students/inf/k/kl439965/pw/kl439965

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/students/inf/k/kl439965/pw/kl439965

# Include any dependencies generated for this target.
include CMakeFiles/executor.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/executor.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/executor.dir/flags.make

CMakeFiles/executor.dir/executor.c.o: CMakeFiles/executor.dir/flags.make
CMakeFiles/executor.dir/executor.c.o: executor.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/students/inf/k/kl439965/pw/kl439965/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/executor.dir/executor.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/executor.dir/executor.c.o -c /home/students/inf/k/kl439965/pw/kl439965/executor.c

CMakeFiles/executor.dir/executor.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/executor.dir/executor.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/students/inf/k/kl439965/pw/kl439965/executor.c > CMakeFiles/executor.dir/executor.c.i

CMakeFiles/executor.dir/executor.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/executor.dir/executor.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/students/inf/k/kl439965/pw/kl439965/executor.c -o CMakeFiles/executor.dir/executor.c.s

# Object files for target executor
executor_OBJECTS = \
"CMakeFiles/executor.dir/executor.c.o"

# External object files for target executor
executor_EXTERNAL_OBJECTS =

executor: CMakeFiles/executor.dir/executor.c.o
executor: CMakeFiles/executor.dir/build.make
executor: liberr.a
executor: libutils.a
executor: CMakeFiles/executor.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/students/inf/k/kl439965/pw/kl439965/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable executor"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/executor.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/executor.dir/build: executor

.PHONY : CMakeFiles/executor.dir/build

CMakeFiles/executor.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/executor.dir/cmake_clean.cmake
.PHONY : CMakeFiles/executor.dir/clean

CMakeFiles/executor.dir/depend:
	cd /home/students/inf/k/kl439965/pw/kl439965 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/students/inf/k/kl439965/pw/kl439965 /home/students/inf/k/kl439965/pw/kl439965 /home/students/inf/k/kl439965/pw/kl439965 /home/students/inf/k/kl439965/pw/kl439965 /home/students/inf/k/kl439965/pw/kl439965/CMakeFiles/executor.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/executor.dir/depend

