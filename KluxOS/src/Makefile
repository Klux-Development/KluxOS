# Add this line to the existing Makefile
# Rule for compiling assembly (.s) files

# Add assembly objects
ASM_SRCS = $(wildcard kernel/asm/*.s)
ASM_OBJS = $(patsubst %.s,%.o,$(ASM_SRCS))

# Add ASM_OBJS to the OBJ list
OBJS += $(ASM_OBJS)

# Compilation rule for assembly files
%.o: %.s
	$(AS) $(ASFLAGS) -o $@ $< 