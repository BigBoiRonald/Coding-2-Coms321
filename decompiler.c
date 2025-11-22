#include<stdio.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<sys/mman.h>

int main(int argc, char[] argv){
    int[] lineInfo;
    struct stat buffer;
    int *instArr;
    char *output;

    FILE *f = fopen(argv[1], "r");
    if(f == NULL){
        printf("file not found");
        return 1;//error code
    }
    fstat(f, &buffer);
    if(fgets(lineInfo, sizeof(lineInfo), f) == NULL){
        printf("file empty");
        return 1;
    }
    instArr = mmap(NULL, buffer.st_size/4, PROT_READ | PROT_WRITE, MAP_PRIVATE, f, 0);
    output = calloc(buffer.st_size/4, sizeof(*output));

    for(int i = 0; i < buffer.st_size/4; i++){
        instArr[i] = be32toh(instArr[i]);
        decompile(instArr[i], output+i);
    }

    return 0;//no error
}

char[] decompile(int inst, char *storeLoc){
    char[10] opcode;
    char[1] format;
    int shamt;
    int rdShift = inst >> 21;

    if(inst >> 26 == 100101){//Branch
        opcode = "BL";
        format = "B";
    }if(inst >> 26 == 000101){
        opcode = "B";
        format = "B";
    }if(inst >> 24 == 10110101){//Cond. Branch
        opcode = "CBNZ";
        format = "CB"
    }if(inst >> 24 == 10110100){
        opcode = "CBZ";
        format = "CB"
    }if(inst >> 22 == 1101001000){//Immediate
        opcode = "EORI";
        format = "I";
    }if(inst >> 22 == 1001000100){
        opcode = "ADDI";
        format = "I";
    }if(inst >> 22 == 1011000100){
        opcode = "ADDIS";
        format = "I";
    }if(inst >> 22 == 1001001000){
        opcode = "ANDI";
        format = "I";
    }if(inst >> 22 == 1111001000){
        opcode = "ANDIS";
        format = "I";
    }if(inst >> 22 == 1011001000){
        opcode = "ORRI";
        format = "I";
    }if(inst >> 22 == 1101000100){
        opcode = "SUBI";
        format = "I";
    }if(inst >> 22 == 1111000100){
        opcode = "SUBIS";
        format = "I";
    }if(rdShift == 11101010000){//Register
        opcode = "ANDS";
        format = "R";
    }if(rdShift == 10001011000){
        opcode = "ADD";
        format = "R";
    }if(rdShift == 10101011000){
        opcode = "ADDS";
        format = "R";
    }if(rdShift == 10001010000){
        opcode = "AND";
        format = "R";
    }if(rdShift == 11010110000){
        opcode = "BR";
        format = "R";
    }if(rdShift == 11111111110){
        opcode = "DUMP";
        format = "R";
    }if(rdShift == 11001010000){
        opcode = "EOR";
        format = "R";
    }if(rdShift == 00011110011){//The double fs 
        shamt = (inst & 00000000000000001111111111111111) >> 10;
        if(shamt == 000010){
            opcode = "FMULD";
        }if(shamt == 000110){
            opcode = "FDIVD";
        }if(shamt == 001000){
            opcode = "FCMPD";
        }if(shamt == 001010){
            opcode = "FADDD";
        }if(shamt == 001110){
            opcode = "FSUBD";
        }
        format = "R"
    }if(rdShift == 00011110001){//The single fs
        shamt = (inst & 00000000000000001111111111111111) >> 10;
        if(shamt == 000010){
            opcode = "FMULS";
        }if(shamt == 000110){
            opcode = "FDIVS";
        }if(shamt == 001000){
            opcode = "FCMPS";
        }if(shamt == 001010){
            opcode = "FADDS";
        }if(shamt == 001110){
            opcode = "FSUBS";
        }
        format = "R"
    }if(rdShift == 11111111111){
        opcode = "HALT";
        format = "R";
    }if(rdShift == 11111100010){
        opcode = "LDURD";
        format = "R";
    }if(rdShift == 10111100010){
        opcode = "LDURS";
        format = "R";
    }if(rdShift == 11010011011){
        opcode = "LSL";
        format = "R";
    }if(rdShift == 11010011010){
        opcode = "LSR";
        format = "R";
    }if(rdShift == 10011011000){
        opcode = "MUL";
        format = "R";
    }if(rdShift == 10101010000){
        opcode = "ORR";
        format = "R";
    }if(rdShift == 11111111100){
        opcode = "PRNL";
        format = "R";
    }if(rdShift == 11111111101){
        opcode = "PRNT";
        format = "R";
    }if(rdShift == 10011010110){
        shamt = (inst & 00000000000000001111111111111111) >> 10;
        opcode = "SDIV" * (shamt == 000010) + "UDIV" * (shamt == 000011);
        format = "R";
    }if(rdShift == 10011011010){
        opcode = "SMULH";
        format = "R";
    }if(rdShift == 11111100000){
        opcode = "STURD";
        format = "R";
    }if(rdShift == 10111100000){
        opcode = "STURS";
        format = "R";
    }if(rdShift == 11001011000){
        opcode = "SUB";
        format = "R";
    }if(rdShift == 11101011000){
        opcode = "SUBS";
        format = "R";
    }if(rdShift == 10011011110){
        opcode = "UMULH";
        format = "R";
    }if(rdShift == 11111000010){//Data
        opcode = "LOAD";
        format = "D";
    }if(rdShift == 00111000010){
        opcode = "LDURB";
        format = "D";
    }if(rdShift == 01111000010){
        opcode = "LDURH";
        format = "D";
    }if(rdShift == 10111000100){
        opcode = "LDURSW";
        format = "D";
    }if(rdShift == 11111000000){
        opcode = "STUR";
        format = "D";
    }if(rdShift == 00111000000){
        opcode = "STURB";
        format = "D";
    }if(rdShift == 01111000000){
        opcode = "STURH";
        format = "D";
    }if(rdShift == 10111000000){
        opcode = "STURW";
        format = "D";
    }

    switch(format){
        case "B":
            return "1";
            
        case "CB":
            return "2";
            
        case "I":
            return "3";
        
        case "R":
            return "4";
        
        case "D":
            return "5";
        
        case "IW":
            return "6";
        
        else:
            return "ajhhhhhhh";

    }
}