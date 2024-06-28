# Compiler and Linker
CXX = g++

# Runtime Files
RUN_FILES = jobExecutorServer.txt
TEMP_FILES = chr19.fa

# Executables
EXEC_JOB_COMMANDER = jobCommander
EXEC_JOB_EXECUTOR_SERVER = jobExecutorServer

# Flags, Libraries and Includes
CXXFLAGS ?= -std=c++17 -Wall -Werror
LDFLAGS ?=
LDLIBS ?= -lpthread -lm

# Source files for each executable
SOURCES_JOB_COMMANDER := JobCommander.cpp Commander.cpp  PipeManager.cpp 
SOURCES_JOB_EXECUTOR_SERVER := JobExecutorServer.cpp Server.cpp PipeManager.cpp SignalHandler.cpp

# Object files for each executable
OBJECTS_JOB_COMMANDER := $(SOURCES_JOB_COMMANDER:.cpp=.o)
OBJECTS_JOB_EXECUTOR_SERVER := $(SOURCES_JOB_EXECUTOR_SERVER:.cpp=.o)

# Dependency files for each executable
DEPS := $(OBJECTS_JOB_COMMANDER:.o=.d) $(OBJECTS_JOB_EXECUTOR_SERVER:.o=.d)

# Default target
all: $(EXEC_JOB_COMMANDER) $(EXEC_JOB_EXECUTOR_SERVER)

# Build rules for JobCommander
$(EXEC_JOB_COMMANDER): $(OBJECTS_JOB_COMMANDER)
	$(CXX) $(LDFLAGS) $^ -o $@ $(LDLIBS)

# Build rules for JobExecutorServer
$(EXEC_JOB_EXECUTOR_SERVER): $(OBJECTS_JOB_EXECUTOR_SERVER)
	$(CXX) $(LDFLAGS) $^ -o $@ $(LDLIBS)

# Generic rule for building objects
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -MMD -c $< -o $@

# Include dependency files
-include $(DEPS)

# Clean
clean:
	rm -f $(EXEC_JOB_COMMANDER) $(EXEC_JOB_EXECUTOR_SERVER) $(OBJECTS_JOB_COMMANDER) $(OBJECTS_JOB_EXECUTOR_SERVER) $(DEPS)
	rm -f $(RUN_FILES) $(TEMP_FILES)