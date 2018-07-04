#
# ===============================================
# |                                             |
# |  If using for the first time in a project,  |
# | run "make tree" to initialize the directory |
# |                 structure.                  |
# |                                             |
# +---------------------------------------------+
# |                                             |
# |   More information on make targets can be   |
# |                 found below.                |
# |                                             |
# ===============================================
#
#
#
# ================
# |              |
# |     ABOUT    |
# |              |
# ================
#
# This Makefile was created to try and minimize the effort
# required to compile most C projects (although it could be
# extended to projects of other languages) by automating
# compile rules and adding a few other functions (e.g. `tree`,
# `zip`).
# Most commands are easily customizable by altering the values
# on some variables, but work in their default state, so you
# could use this Makefile as it is for a C project.
#



#
# Output file name
#

OUT := neuralnetwork.out


#
# Zip file name
#

ZIP := proj.zip


#
# Compiler
#
CC := gcc


#
# Expressions which describe source and compiled files
#
SRC_FILE := c
COMP_FILE := o

SRC_PTRN := "*."$(SRC_FILE)


#
# Library names without `-l` prefix
# e.g.: if you use math.h and GL/gl.h, instead of `-lm -lGL`,
# set the definition to
#     LIBS := m GL
#

LIBS := m


#
# Compile flags
#

C_FLAGS = -fopenmp -O3 -L$(LIB_DIR) -I$(INC_DIR) $(addprefix -l,$(LIBS))


#
# Flags for running valgrind
#

VALGRIND_FLAGS := --leak-check=full --show-leak-kinds=all --track-origins=yes


#
# Flags for running gdb
#

GDB_FLAGS := 

#
# Tree customization
# Default tree structure:
#
# - Project directory
#   \_ Makefile - This file
#   \_ src/ - Source files (subdirectories included)
#   |  \_ .obj/ - Object files
#   |  \_ .dep/ - Dependency files
#   \_ include/ - Header files
#   \_ test/ - Test case files
#   \_ lib/ - Libraries
#   \_ build/ - Output directory
#


#
# ===============
# |             |
# |   TARGETS   |
# |             |
# ===============
#
# The main targets of this Makefile are:
#   - tree:        Initialize project tree.
#               Must be run before any other commands.
#
#     - all:        Compile all modified/uncompiled files to create an executable.
#
#     - rebuild:    Recompile all files regardless of modifications date.
#
#     - clean:    Cleanup compiled files and intermediate Makefile files.
#
#     - run:        Run generated executable.
#
#     - test:        Compares generated output to an expected output file.
#                 More information can be found below.
#
#    - arun:        Same as running `all`, followed by `run`.
#
#    - rebrun:    Same as running `rebuild`, followed by `run`.
#
#    - zip:        Create a .zip file with the following files and directories:
#                - This file
#                - BLD_DIR/ (*)
#                - INC_DIR/
#                - SRC_DIR/ (*)
#                - TST_DIR/
#                - LIB_DIR/
#
#                (*) The executable, *.o files and *.d (intermediate Makefile
#                files) are not included in the final .zip.
#
#                The .zip file is created in the project root (the same directory
#                as this file).
#
#    - valgrind:    Same as `run`, except executes over valgrind.
#
#    - gdb:        Same as `run`, except executes over gdb.
#
#    - g:        Same as `clean` followed by `all`, but compiles using
#                the `-g` flag.
#
#    - destroy-tree-yes-i-am-sure:    THERE IS NO WAY TO REVERSE THIS.
#                                    All files, directories and subdirectories
#                                    are removed, except for this file.
#
#
# +=========================+
# |                         |
# | DIR TREE CUSTOMIZATION  |
# |                         |
# +=========================+
#

SRC_DIR := ./src
OBJ_DIR := $(SRC_DIR)/.obj
DEP_DIR := $(SRC_DIR)/.dep
INC_DIR := ./include
TST_DIR := ./test
LIB_DIR := ./lib
BLD_DIR := ./build


#
# +=========================+
# |                         |
# |      TEST CASE I/O      |
# |                         |
# +=========================+
#
# You can define a file to be read as stdin by setting the command line
# variable `case`. The input file is the one returned by `case_ifile` (defined
# below). The file name returned by  `case_ifile` is looked for in the `TST_DIR`
# directory.
# If the make target is `test`, then the expected output is the one returned by
# `case_ofile`, and also looked for in `TST_DIR`. After execution, the expected
# output and the actual output are compared using the command stored in the
# `diff` variable.
#


# File where the output of the last execution is saved to
STDOUT_LOG := out.log


#
# Diff command used for comparing output and expected output
# $(1) is the actual output
# $(2) is the expected output
#
diff = vimdiff -MR $(1) $(2) 2> /dev/null


#
# Functions that return the input and output files associated with a test case
# Default is case=case_name -> case_name.in as input and case_name.out as output
#
case_ifile = $(1:%=%.in)
case_ofile = $(1:%=%.out)


#
# END OF CUSTOM STUFF
#
# If you want to fully understand how this Makefile works, feel free too look
# down there, but DON'T TOUCH ANYTHING.
#
#
# SERIOUSLY, DON'T DO IT.
#
#
#
# IF YOU MESS WITH ANYTHING DOWN HERE IT COULD VERY WELL STOP WORKING, SO DON'T.
#
#
#
#
#
#
#
#
#
#
# YOU HAVE BEEN WARNED.


.PHONY: all run test clean arun rebrun rebuild zip tree\
        destroy-tree-yes-i-am-sure valgrind gdb g

# Find all source files
SOURCES := $(shell find $(SRC_DIR) -name $(SRC_PTRN) 2> /dev/null)


OBJECTS := $(addsuffix .$(COMP_FILE), $(basename $(SOURCES)))
DEPS := $(addsuffix .d, $(basename $(SOURCES)))

# Add $(OBJ_DIR)/ as a prefix to each object file bare name
OBJECTS := $(addprefix $(OBJ_DIR)/,$(notdir $(OBJECTS)))
DEPS := $(addprefix $(DEP_DIR)/,$(notdir $(DEPS)))

# Search path for make
# Allows use of pattern rules in
# directories discovered in runtime
SRC_SUBDIR := $(shell find $(SRC_DIR) -type d 2> /dev/null)
VPATH += $(SRC_SUBDIR)



RUN_CMD := $(BLD_DIR)/$(OUT)

debug_mode = 

ifneq (,$(findstring g,$(MAKECMDGOALS)))
debug_mode = yep
endif

ifneq (,$(findstring valgrind,$(MAKECMDGOALS)))
RUN_CMD := valgrind $(VALGRIND_FLAGS) $(RUN_CMD)
debug_mode = yep
endif

ifneq (,$(findstring gdb,$(MAKECMDGOALS)))
RUN_CMD := gdb $(RUN_CMD)
debug_mode = yep
endif

RUN_CMD := $(RUN_CMD) $(ARGS)

ifdef debug_mode
C_FLAGS += -g -Wall -Wextra -O0
endif

ifdef case
I_FILE := $(TST_DIR)/$(call case_ifile,$(case))
O_FILE := $(TST_DIR)/$(call case_ofile,$(case))


ifndef I_FILE
    $(error `case_ifile` function did not return a file name)
endif


ifneq (,$(findstring test,$(MAKECMDGOALS)))

ifndef O_FILE
    $(error `case_ofile` function did not return a file name)
endif

endif

RUN_CMD += < $(I_FILE)

endif


all: $(BLD_DIR)/$(OUT)

g: clean all

debug: g

run:
	@$(RUN_CMD) | tee $(STDOUT_LOG)
	@printf "====================\n"

valgrind: run

gdb: run

test: run
	@$(call diff,$(STDOUT_LOG),$(O_FILE))


clean:
	-@rm -f $(ZIP).zip
	-@rm -f $(OBJ_DIR)/*.$(COMP_FILE)
	-@rm -f $(DEP_DIR)/*.d
	-@rm -f --preserve-root $(BLD_DIR)/*

arun: all run

rebrun: rebuild run

rebuild: clean all

release: rebuild

zip:
	@zip -9q $(ZIP).zip $(BLD_DIR)/.gitkeep $(firstword $(MAKEFILE_LIST)) -r \
	    $(INC_DIR)/ $(SRC_DIR)/ $(LIB_DIR)/ $(TST_DIR)/ -x \*.$(COMP_FILE) \*.d


$(BLD_DIR)/$(OUT): $(OBJECTS)
	@printf "Linking object files... "
	@$(CC) $^ $(C_FLAGS) $(CFLAGS) -o $(BLD_DIR)/$(OUT) 
	@printf "\n"
	@printf "====================\n"
	@printf " COMPILING COMPLETE \n"
	@printf "====================\n\n"

$(OBJ_DIR)/%.$(COMP_FILE):
	@printf "Building -%s-... " $(notdir $(basename $<))
	@$(CC) $(C_FLAGS) $(CFLAGS) -c -o $@ $<
	@printf "Done.\n"

$(DEP_DIR)/%.d: %.$(SRC_FILE)
	@$(CC) $(C_FLAGS) -MM -MT'$(OBJ_DIR)/$(notdir $(@:%.d=%.$(COMP_FILE)))' $< > $@

ifeq (,$(findstring clean,$(MAKECMDGOALS)))
-include $(DEPS)
endif

.gitignore:
	@echo "$(OBJ_DIR:./%=/%)/*" >> .gitignore
	@echo "$(DEP_DIR:./%=/%)/*" >> .gitignore
	@echo "$(LIB_DIR:./%=/%)/*" >> .gitignore
	@echo "$(BLD_DIR:./%=/%)/*" >> .gitignore
	@echo "!**/.gitkeep" >> .gitignore

tree: .gitignore
	@printf "Creating project tree...\n"
	-@mkdir -p $(SRC_DIR)
	-@touch $(SRC_DIR)/.gitkeep
	@printf "%s directory created.\n" "Source"
	-@mkdir -p $(INC_DIR)
	-@touch $(INC_DIR)/.gitkeep
	@printf "%s directory created.\n" "Include"
	-@mkdir -p $(DEP_DIR)
	-@touch $(DEP_DIR)/.gitkeep
	@printf "%s directory created.\n" "Dependency"
	-@mkdir -p $(OBJ_DIR)
	-@touch $(OBJ_DIR)/.gitkeep
	@printf "%s directory created.\n" "Object"
	-@mkdir -p $(TST_DIR)
	-@touch $(TST_DIR)/.gitkeep
	@printf "%s directory created.\n" "Test case"
	-@mkdir -p $(LIB_DIR)
	-@touch $(LIB_DIR)/.gitkeep
	@printf "%s directory created.\n" "Library"
	-@mkdir -p $(BLD_DIR)
	-@touch $(BLD_DIR)/.gitkeep
	@printf "%s directory created.\n" "Build"
	@printf "Project tree complete.\n"
	@printf "======================\n\n"

destroy-tree-yes-i-am-sure:
	-@rm --preserve-root -rf $(SRC_DIR) $(INC_DIR) $(DEP_DIR) $(OBJ_DIR)
	-@rm --preserve-root -rf $(TST_DIR) $(LIB_DIR) $(BLD_DIR)
	-@rm -f $(STDOUT_LOG) .gitignore
	@printf "Too late to change your mind.\n"
	@printf "Goodbye project!\n"
