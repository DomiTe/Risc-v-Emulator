# RISC-V Emulator

## Overview

This project is a simple RISC-V emulator written in C, simulating the instruction execution of a RISC-V CPU. The emulator can load instructions and data into memory and execute them, supporting basic RISC-V instructions like LUI, AUIPC, JAL, JALR, and various types of branching, load, and store operations.

## Features

- **Instruction Loading:** Load RISC-V instructions from an external file.
- **Data Memory:** Load data from a separate file for memory access operations.
- **CPU Simulation:** Supports RISC-V instruction fetch, decode, execution, memory access, and write-back phases.
- **Supported Instructions:** Implements key RISC-V instruction types such as R-type, I-type, S-type, B-type, JAL, JALR, AUIPC, and LUI.
- **Registers and Memory:** A register file of 32 registers is simulated along with a program counter (PC), instruction memory, and data memory.

## Structure

### Key Components:
- **CPU Struct:**
  - `regfile_[32]`: 32 general-purpose registers.
  - `pc_`: The program counter for instruction tracking.
  - `instr_mem_`: Pointer to instruction memory.
  - `data_mem_`: Pointer to data memory.
  
- **Instruction Formats:**
  - The emulator decodes R-type, I-type, S-type, B-type, JAL, and JALR instructions based on opcode and funct3/funct7 fields.

### Functions:
- `CPU_init`: Initializes the CPU, loads instruction and data memory.
- `CPU_execute`: Executes a single instruction cycle, including instruction fetch, decode, and execution phases.
- `CPU_open_instruction_mem`: Loads instructions from a file into instruction memory.
- `CPU_load_data_mem`: Loads data from a file into data memory.
  
### Main Loop:
- The main function runs the CPU for a set number of cycles, executing instructions sequentially until a termination condition is met (such as an instruction limit).

## Usage

### Compilation
To compile the emulator, use the following command:
```bash
gcc main.c -o hu_risc-v_emu -std=c11
```

### Running the Emulator
The emulator requires two input files:
1. **Instruction memory file:** The file containing the RISC-V binary instructions.
2. **Data memory file:** The file containing initial data for the data memory.

Run the emulator with:
```bash
./hu - risc - av_emu <instruction_memory_file> <data_memory_file>
```

### Example:
```bash
./ hu - risc - av_emu instruction_mem . bin data_mem . bin
```

### Output:
- After execution, the register file values are printed to the console, showing the final state of the CPU registers.

## Limitations

- This emulator supports only a subset of RISC-V instructions, specifically those related to basic arithmetic, load/store, and control flow.
- No advanced features like interrupts, floating-point instructions, or exceptions are implemented.
  
## Future Work
- Implement support for additional RISC-V instructions.
- Add error handling for invalid instructions and memory accesses.
- Improve the performance of the execution loop by optimizing memory access.

## Credits

This emulator was developed as part of a university project at Humboldt-Universität zu Berlin.

---

Enjoy using the RISC-V emulator!
