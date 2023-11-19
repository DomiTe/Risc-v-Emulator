
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdint.h>


enum opcode_decode {R = 0x33, I = 0x13, S = 0x23, L = 0x03, B = 0x63, JALR = 0x67, JAL = 0x6F, AUIPC = 0x17, LUI = 0x37};

typedef struct {
    size_t data_mem_size_;
    uint32_t regfile_[32];
    uint32_t pc_;
    uint8_t* instr_mem_;
    uint8_t* data_mem_;
} CPU;

void CPU_open_instruction_mem(CPU* cpu, const char* filename);
void CPU_load_data_mem(CPU* cpu, const char* filename);

CPU* CPU_init(const char* path_to_inst_mem, const char* path_to_data_mem) {
	CPU* cpu = (CPU*) malloc(sizeof(CPU));
	cpu->data_mem_size_ = 0x400000;
    cpu->pc_ = 0x0;
    CPU_open_instruction_mem(cpu, path_to_inst_mem);
    CPU_load_data_mem(cpu, path_to_data_mem);
    return cpu;
}

void CPU_open_instruction_mem(CPU* cpu, const char* filename) {
	uint32_t  instr_mem_size;
	FILE* input_file = fopen(filename, "r");
	if (!input_file) {
			printf("no input\n");
			exit(EXIT_FAILURE);
	}
	struct stat sb;
	if (stat(filename, &sb) == -1) {
			printf("error stat\n");
			perror("stat");
		    exit(EXIT_FAILURE);
	}
	printf("size of instruction memory: %d Byte\n\n",sb.st_size);
	instr_mem_size =  sb.st_size;
	cpu->instr_mem_ = malloc(instr_mem_size);
	fread(cpu->instr_mem_, sb.st_size, 1, input_file);
	fclose(input_file);
	return;
}

void CPU_load_data_mem(CPU* cpu, const char* filename) {
	FILE* input_file = fopen(filename, "r");
	if (!input_file) {
			printf("no input\n");
			exit(EXIT_FAILURE);
	}
	struct stat sb;
	if (stat(filename, &sb) == -1) {
			printf("error stat\n");
			perror("stat");
		    exit(EXIT_FAILURE);
	}
	printf("read data for data memory: %d Byte\n\n",sb.st_size);

    cpu->data_mem_ = malloc(cpu->data_mem_size_);
	fread(cpu->data_mem_, sb.st_size, 1, input_file);
	fclose(input_file);
	return;
}


/**
 * Instruction fetch Instruction decode, Execute, Memory access, Write back
 */

void CPU_execute(CPU* cpu) {

	uint32_t instruction = *(uint32_t*)(cpu->instr_mem_ + (cpu->pc_  & 0xFFFFF));

/**
 * Instruction decode 
 */

    uint32_t opcode = instruction & 0x7F;
    uint8_t funct3 = (uint8_t)((instruction >> 12) & 0x7);
    uint8_t funct7 = (instruction >> 25) & 0x7F;

    uint8_t rd = (instruction >> 7) & 0x1F;
    uint8_t rs1 = (instruction >> 15) & 0x1F;
    uint8_t rs2 = (instruction >> 20) &0x1F;

    uint32_t imm_U = (instruction & 0xFFFFF000);

    int imm_I = (instruction & 0x80000000) ? 0xFFFFF000 
				|instruction >> 20 : instruction >> 20;

    int imm_S = (instruction & 0x80000000) ? 0xFFFFF000 
				|(instruction & 0xF80) >> 7 
				|(instruction & 0xFE000000) >> 20 : (instruction & 0xF80) >> 7 | (instruction & 0xFE000000) >> 20 ;

    int imm_B = (instruction & 0x80000000) ? 0xFFFFE000 
				|(instruction & 0x80) << 4
				|(instruction >> 7) & 0x1E 
				|(instruction & 0x80000000) >> 19 
				|(instruction >> 20) & 0x7E0 : (instruction & 0x80) << 4 | (instruction >> 7) & 0x1E | (instruction & 0x80000000) >> 19 | (instruction >> 20) & 0x7E0;

    int imm_J = (instruction & 0x80000000) ? 0xFFE00000 
				|(instruction & 0x80000000) >> 11 
				|(instruction & 0xFF000) 
				|(instruction & 0x00100000) >> 9 
				|(instruction & 0x7FE00000) >> 20 : (instruction & 0x80000000) >> 11 | (instruction & 0xFF000) |  (instruction & 0x00100000) >> 9 | (instruction & 0x7FE00000) >> 20;

    int32_t sb_address = cpu->regfile_[rs1] + (uint32_t)(imm_S);
	int shamt = rs2;
	
	/**
	 * Execution of Instruction
	 */

	switch (opcode)
	{
	case LUI:
		cpu->regfile_[rd] = imm_U;
		break;

	case AUIPC:
		cpu->regfile_[rd] = (cpu->pc_ + imm_U);
		break;

	case JAL:
		cpu->regfile_[rd] = cpu->pc_ + 0x4;
		cpu->pc_ = (int32_t)(imm_J) + cpu->pc_ - 0x4;
		break;

	case JALR:
		cpu->regfile_[rd] = cpu->pc_ + 0x4;
		cpu->pc_ = ((cpu->regfile_[rs1] + (int32_t)imm_I) & 0xFFFFFFFE) - 0x4;
		break;

	case B:
		switch (funct3)
		{
		case 0b000: //BEQ
			if((cpu->regfile_[rs1]) == (cpu->regfile_[rs2])){
			cpu->pc_ = (cpu->pc_ + (int32_t)(imm_B)) - 0x4;
			}
			break;

		case 0b001: // BNE
			if((cpu->regfile_[rs1]) != (cpu->regfile_[rs2])){
			cpu->pc_ = cpu->pc_ + (int32_t)(imm_B) - 0x4 ;
			}
			break;

		case 0b100: //BLT
			if((signed long)(cpu->regfile_[rs1]) < (signed long)(cpu->regfile_[rs2])){
			cpu->pc_ = cpu->pc_ + (int32_t)(imm_B) - 0x4 ;
			}
			break;

		case 0b101: // BGE
			if((int32_t)(cpu->regfile_[rs1]) >= (int32_t)(cpu->regfile_[rs2])){
				cpu->pc_ = cpu->pc_ + (int32_t)(imm_B) - 0x4;
			}
			break;

		case 0b110: // BLTU
            if ((cpu->regfile_[rs1]) < (cpu->regfile_[rs2])){
                cpu->pc_ = cpu->pc_ + (int32_t)(imm_B) - 0x4;
            }
			break;

		case 0b111: //BGEU
			if((cpu->regfile_[rs1]) >= (cpu->regfile_[rs2])){
				cpu->pc_ = cpu->pc_ + (int32_t)imm_B - 0x4;
			}
			break;

		default: 
			break;
		}
		break;

	case L:
		switch (funct3)
		{
		case 0b000: //LB
			cpu->regfile_[rd] = (cpu->data_mem_[cpu->regfile_[rs1] + imm_I]);
			break;

		case 0b001: //LH
            cpu->regfile_[rd] = *(uint16_t*)(cpu->regfile_[rs1] + imm_I + cpu->data_mem_);
			break;

		case 0b010: //LW
            cpu->regfile_[rd] = *(uint32_t*)(cpu->regfile_[rs1] + imm_I + cpu->data_mem_);
			break;

		case 0b100: //LBU
            cpu->regfile_[rd] = cpu->data_mem_[cpu->regfile_[rs1] + imm_I];
			break;

		case 0b101: //LHU
            cpu->regfile_[rd] = *(uint16_t*)(cpu->regfile_[rs1] + imm_I + cpu->data_mem_);
			break;

		default:
			break;
		}
		break;

	case S:
		switch (funct3)
        {
		case 0b000: //SB
        { 
            if(sb_address == 0x5000){
                putchar((uint8_t)cpu->regfile_[rs2]);
            }else{
                cpu->data_mem_[sb_address] = (uint8_t)cpu->regfile_[rs2];
            }
			break;}

		case 0b001: //SH
            *(uint16_t*)(cpu->data_mem_ + cpu->regfile_[rs1] + imm_S) = (uint16_t)(cpu->regfile_[rs2]);
			break;

		case 0b010: //SW
            *(uint32_t*)(cpu->data_mem_ + cpu->regfile_[rs1] + imm_S) = (uint32_t)(cpu->regfile_[rs2]);
			break;

		default:
			break;
		}
		break;

	case I:
		switch (funct3)
		{
		case 0b000: //ADDI
			cpu->regfile_[rd] = cpu->regfile_[rs1] + imm_I;
			break;

		case 0b001://SLLI
			cpu->regfile_[rd] = cpu->regfile_[rs1] << shamt;
			break;

		case 0b010: //SLTI
			cpu->regfile_[rd] = ((signed long)cpu->regfile_[rs1] < (signed long)imm_I) ? 1 : 0;
			break;

		case 0b011: //SLTIU
			cpu->regfile_[rd] = ((unsigned)cpu->regfile_[rs1] < (unsigned)imm_I) ? 1: 0;
			break;

		case 0b100: // XORI
			cpu->regfile_[rd] = cpu->regfile_[rs1] ^ imm_I;
			break;

		case 0b110: //ORI
			cpu->regfile_[rd] = cpu->regfile_[rs1] | imm_I;
			break;

		case 0b111: //ANDI
			cpu->regfile_[rd] = cpu->regfile_[rs1] & imm_I;
			break;

		case 0b101:
			switch (funct7)
			{
			case 0b0: //SRLI
				cpu->regfile_[rd] = cpu->regfile_[rs1] >> shamt;
				break;

			case 0b100000: //SRAI
				cpu->regfile_[rd] = (int32_t)cpu->regfile_[rs1] >> (int32_t)imm_I;
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
		break;

	case R:
		switch (funct3)
		{
		case 0b000:
			switch (funct7)
			{
			case 0b0: //ADD
				cpu->regfile_[rd] = (cpu->regfile_[rs1]) + (cpu->regfile_[rs2]);
				break;

			case 0b100000: //SUB
				cpu->regfile_[rd] = (cpu->regfile_[rs1]) - (cpu->regfile_[rs2]);
				break;

			default:
				break;
			}
			break;

		case 0b001: //SLL
			cpu->regfile_[rd] = (cpu->regfile_[rs1]) << (cpu->regfile_[rs2] & 0x1F);
			break;

		case 0b010: //SLT
			cpu->regfile_[rd] = (int32_t )(cpu->regfile_[rs1]) < (int32_t)(cpu->regfile_[rs2]) ?1:0 ;
			break;

		case 0b011: //SLTU
			cpu->regfile_[rd] = (cpu->regfile_[rs1] < cpu->regfile_[rs2])?1:0;
			break;

		case 0b100: //XOR
			cpu->regfile_[rd] = (cpu->regfile_[rs1]) ^ (cpu->regfile_[rs2]);
			break;

		case 0b101:
			switch (funct7)
			{
			case 0b0: //SRL
				cpu->regfile_[rd] = (cpu->regfile_[rs1]) >> (cpu->regfile_[rs2] & 0x1F);
				break;

			case 0b100000: //SRA
				cpu->regfile_[rd] = (int32_t)(cpu->regfile_[rs1]) >> (unsigned long)(cpu->regfile_[rs2]);
				break;

			default:
				break;
			}
			break;

		case 0b110: //OR
			cpu->regfile_[rd] = (cpu->regfile_[rs1]) | (cpu->regfile_[rs2]);
			break;

		case 0b111: //AND
			cpu->regfile_[rd] = (cpu->regfile_[rs1]) & (cpu->regfile_[rs2]);
			break;

		default:
			break;
		}
		break;

	default:
		break;
	}
    cpu->regfile_[0] = 0;
	cpu->pc_ += 0x4;
	
	// TODO

}

int main(int argc, char* argv[]) {
	printf("C Praktikum\nHU Risc-V  Emulator 2022\n");

	CPU* cpu_inst;

	cpu_inst = CPU_init(argv[1], argv[2]);
    for(uint32_t i = 0; i <1000000; i++) { // run 70000 cycles
    	CPU_execute(cpu_inst);
    }

	printf("\n-----------------------RISC-V program terminate------------------------\nRegfile values:\n");

	//output Regfile
	for(uint32_t i = 0; i <= 31; i++) {
    	printf("%d: %X\n",i,cpu_inst->regfile_[i]);
    }
    fflush(stdout);

	return 0;
}
