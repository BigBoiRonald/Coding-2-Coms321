#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>

char *branchConditional(uint32_t Rt);
void decompile(uint32_t inst, int index);
char *binaryToDecimal(uint32_t binary, int bits, bool negPossible, bool telemetry);
char *generateLabel(int index);
char *branchLabel(int distance, char *currLabel);

int main(int argc, char *argv[])
{
    struct stat buffer;
    uint32_t *instArr;

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1)
    {
        printf("Error opening file");
        return 1; // error code
    }
    fstat(fd, &buffer);
    instArr = mmap(NULL, buffer.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    for (size_t i = 0; i < buffer.st_size / 4; i++)
    {
        uint32_t val = be32toh(instArr[i]);
        decompile(val, (int)i);
    }

    return 0; // no error
}

void decompile(uint32_t inst, int index)
{
    char *mnemonic;
    char format; // Note: Formal O is for B.cond

    // Calculating opcodes
    uint32_t opcode6 = (inst >> 26) & 0x3F;   // & 0b0011 1111 - 6 bits
    uint32_t opcode8 = (inst >> 24) & 0xFF;   // & 0b1111 1111 - 8 bits
    uint32_t opcode10 = (inst >> 22) & 0x3FF; // & 0b0011 1111 1111 - 10 bits
    uint32_t rdShift = (inst >> 21) & 0x7FF;  // & 0b0111 1111 1111 - 11 bits

    // Other fields
    uint32_t rt = inst & 0x1F;            // - Least signification 5 bits - also Rd
    uint32_t rn = (inst >> 5) & 0x1F;     // & 0x0001 1111 - 5 bits
    uint32_t shamt = (inst >> 10) & 0x3F; // & 0x0011 1111 - 6 bits
    uint32_t rm = (inst >> 16) & 0x1F;
    uint32_t alu_immediate = (inst >> 10) & 0xFFF; // 12 bits
    uint32_t dt_address = (inst >> 12) & 0x1FF;    // 9 bits

    // printf("Rt: %d, rn: %d, RM: %d\n", rt, rn, rm);
    // printf("Shamt: %d, immediate: %d, dt_address: %d\n", shamt, alu_immediate, dt_address);

    // Could add the op address here - I dont think it is used
    uint32_t br_address = inst & 0x3FFFFFF;           // 26 bits
    uint32_t cond_br_address = (inst >> 5) & 0x7FFFF; // 19 bits

    // printf("Branch address: %d, conditional Branch address: %d\n", br_address, cond_br_address);

    if (opcode6 == 0b100101)
    { // Branch
        mnemonic = "BL";
        format = 'B';
    }
    if (opcode6 == 0b000101)
    {
        mnemonic = "B";
        format = 'B';
    }
    if (opcode8 == 0b10110101)
    { // Cond. Branch
        mnemonic = "CBNZ";
        format = 'C';
    }
    if (opcode8 == 0b10110100)
    {
        mnemonic = "CBZ";
        format = 'C';
    }
    if (opcode8 == 0b01010100) // B.cond
    {
        mnemonic = branchConditional(rt);
        format = 'O'; // Conditional Branch
    }
    if (opcode10 == 0b1101001000)
    { // Immediate
        mnemonic = "EORI";
        format = 'I';
    }
    if (opcode10 == 0b1001000100)
    {
        mnemonic = "ADDI";
        format = 'I';
    }
    if (opcode10 == 0b1001001000)
    {
        mnemonic = "ANDI";
        format = 'I';
    }
    if (opcode10 == 0b1011001000)
    {
        mnemonic = "ORRI";
        format = 'I';
    }
    if (opcode10 == 0b1101000100)
    {
        mnemonic = "SUBI";
        format = 'I';
    }
    if (opcode10 == 0b1111000100)
    {
        mnemonic = "SUBIS";
        format = 'I';
    }
    if (rdShift == 0b10001011000)
    {
        mnemonic = "ADD";
        format = 'R';
    }
    if (rdShift == 0b10001010000)
    {
        mnemonic = "AND";
        format = 'R';
    }
    if (rdShift == 0b11010110000)
    {
        mnemonic = "BR";
        format = 'L';
    }
    if (rdShift == 0b11111111110)
    {
        mnemonic = "DUMP";
        format = 'S';
    }
    if (rdShift == 0b11001010000)
    {
        mnemonic = "EOR";
        format = 'R';
    }
    if (rdShift == 0b11111111111)
    {
        mnemonic = "HALT";
        format = 'S';
    }
    if (rdShift == 0b11010011011)
    {
        mnemonic = "LSL";
        format = 'R';
    }
    if (rdShift == 0b11010011010)
    {
        mnemonic = "LSR";
        format = 'R';
    }
    if (rdShift == 0b10011011000)
    {
        mnemonic = "MUL";
        format = 'R';
    }
    if (rdShift == 0b10101010000)
    {
        mnemonic = "ORR";
        format = 'R';
    }
    if (rdShift == 0b11111111100)
    {
        mnemonic = "PRNL";
        format = 'S';
    }
    if (rdShift == 0b11111111101)
    {
        mnemonic = "PRNT";
        format = 's';
    }
    if (rdShift == 0b11001011000)
    {
        mnemonic = "SUB";
        format = 'R';
    }
    if (rdShift == 0b11101011000)
    {
        mnemonic = "SUBS";
        format = 'R';
    }
    if (rdShift == 0b11111000010)
    { // Data
        mnemonic = "LDUR";
        format = 'D';
    }
    if (rdShift == 0b11111000000)
    {
        mnemonic = "STUR";
        format = 'D';
    }

    char *label;
    switch (format)
    {
    case 'B':
        label = generateLabel(index);
        printf("%s: %s %s", label, mnemonic, branchLabel(atoi(binaryToDecimal(br_address, 26, true, false)), label));
        break;
    case 'C':
        label = generateLabel(index);
        printf("%s: %s X%d, %s", label, mnemonic, rt, branchLabel(atoi(binaryToDecimal(cond_br_address, 19, true, false)), label));
        break;
    case 'O':
        label = generateLabel(index);
        printf("%s: %s %s", label, mnemonic, branchLabel(atoi(binaryToDecimal(cond_br_address, 19, true, false)), label));
        break;
    case 'I':
        printf("%s: %s X%d, X%d, #%s", generateLabel(index), mnemonic, rt, rn, binaryToDecimal(alu_immediate, 12, true, false));
        break;
    case 'R':
        printf("%s: %s X%d, X%d, X%d", generateLabel(index), mnemonic, rt, rn, rm);
        break;
    case 'L': // for BR
        printf("%s: %s X%d", generateLabel(index), mnemonic, rn);
        break;
    case 'D':
        printf("%s: %s X%d, [X%d, #%s]", generateLabel(index), mnemonic, rt, rn, binaryToDecimal(dt_address, 9, true, false));
        break;
    case 'S': // HALT, DUMP, PRNL
        printf("%s: %s", generateLabel(index), mnemonic);
        break;
    case 's':
        printf("%s: %s X%d", generateLabel(index), mnemonic, rt);
        break;
    default:
        printf("Something broke");
        break;
    }
    printf("\n");
}

char *binaryToDecimal(uint32_t binary, int bits, bool negPossible, bool telemetry)
{
    static char sDeci[32];
    int32_t deci;
    if (negPossible)
    {
        // Need to sign extend binary
        deci = ((int32_t)(binary << (32 - bits)) >> (32 - bits));
    }
    else
    {
        deci = ((int32_t)binary);
    }
    sprintf(sDeci, "%d", deci); // change to str and return

    if (telemetry)
    {
        printf("Deci %d, binary %d\n", deci, binary);
        printf("String value : %s", sDeci);
        printf("sDeci: %s\n", sDeci);
    }
    return sDeci;
}

char *branchConditional(uint32_t Rt)
{
    switch (Rt)
    {
    case 0b00000:
        return "B.EQ";
    case 0b00001:
        return "B.NE";
    case 0b00010:
        return "B.HS";
    case 0b00011:
        return "B.LO";
    case 0b00100:
        return "B.MI";
    case 0b00101:
        return "B.PL";
    case 0b00110:
        return "B.VS";
    case 0b00111:
        return "B.VC";
    case 0b01000:
        return "B.HI";
    case 0b01001:
        return "B.LS";
    case 0b01010:
        return "B.GE";
    case 0b01011:
        return "B.LT";
    case 0b01100:
        return "B.GT";
    case 0b01101:
        return "B.LE";
    default:
        return "B.DF";
    }
}

char *generateLabel(int index)
{
    static char label[5];
    sprintf(label, "L%03d", index);
    return label;
}

char *branchLabel(int distance, char *currLabel)
{
    static char resultLabel[5];
    int currIndex;

    // Gets the number value of the label
    sscanf(currLabel, "L%d", &currIndex);
    // Calculates the target label index
    int targetIndex = currIndex + distance;
    // Returns the resulting label
    sprintf(resultLabel, "L%03d", targetIndex);
    return resultLabel;
}