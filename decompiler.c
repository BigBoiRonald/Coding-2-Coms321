#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

int main(int argc, char *argv)
{
    uint32_t lineInfo;
    struct stat buffer;
    int *instArr;

    FILE *f = fopen(argv[1], "r");
    if (f == NULL)
    {
        printf("file not found");
        return 1; // error code
    }
    fstat(f, &buffer);
    if (fgets(lineInfo, sizeof(lineInfo), f) == NULL)
    {
        printf("file empty");
        return 1;
    }
    instArr = mmap(NULL, buffer.st_size / 4, PROT_READ | PROT_WRITE, MAP_PRIVATE, f, 0);

    for (int i = 0; i < buffer.st_size / 4; i++)
    {
        instArr[i] = be32toh(instArr[i]);
        decompile(instArr[i], i);
    }

    return 0; // no error
}

void decompile(uint32_t inst, int index)
{
    char *mnemonic;
    char format; // Note: Formal O is for B.cond
    char *label;

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
    // Could add the op address here - I dont think it is used
    uint32_t br_address = inst & 0x3FFFFFF;           // 26 bits
    uint32_t cond_br_address = (inst >> 5) & 0x7FFFF; // 19 bits

    // Creates Label for each line
    label = generateLabel(index);

    if (opcode6 == 0b100101)
    { // Branch
        mnemonic = "BL";
        format = "B";
    }
    if (opcode6 == 0b000101)
    {
        mnemonic = "B";
        format = "B";
    }
    if (opcode8 == 0b10110101)
    { // Cond. Branch
        mnemonic = "CBNZ";
        format = "C";
    }
    if (opcode8 == 0b10110100)
    {
        mnemonic = "CBZ";
        format = "C";
    }
    if (opcode8 == 0b01010100) // B.cond
    {
        mnemonic = branchConditional(rt);
        format = "C"; // Conditional Branch
    }
    if (opcode10 == 0b1101001000)
    { // Immediate
        mnemonic = "EORI";
        format = "I";
    }
    if (opcode10 == 0b1001000100)
    {
        mnemonic = "ADDI";
        format = "I";
    }
    if (opcode10 == 0b1001001000)
    {
        mnemonic = "ANDI";
        format = "I";
    }
    if (opcode10 == 0b1011001000)
    {
        mnemonic = "ORRI";
        format = "I";
    }
    if (opcode10 == 0b1101000100)
    {
        mnemonic = "SUBI";
        format = "I";
    }
    if (opcode10 == 0b1111000100)
    {
        mnemonic = "SUBIS";
        format = "I";
    }
    if (rdShift == 0b10001011000)
    {
        mnemonic = "ADD";
        format = "R";
    }
    if (rdShift == 0b10001010000)
    {
        mnemonic = "AND";
        format = "R";
    }
    if (rdShift == 0b11010110000)
    {
        mnemonic = "BR";
        format = "R";
    }
    if (rdShift == 0b11111111110)
    {
        mnemonic = "DUMP";
        format = "R";
    }
    if (rdShift == 0b11001010000)
    {
        mnemonic = "EOR";
        format = "R";
    }
    if (rdShift == 0b11111111111)
    {
        mnemonic = "HALT";
        format = "R";
    }
    if (rdShift == 0b11010011011)
    {
        mnemonic = "LSL";
        format = "R";
    }
    if (rdShift == 0b11010011010)
    {
        mnemonic = "LSR";
        format = "R";
    }
    if (rdShift == 0b10011011000)
    {
        mnemonic = "MUL";
        format = "R";
    }
    if (rdShift == 0b10101010000)
    {
        mnemonic = "ORR";
        format = "R";
    }
    if (rdShift == 0b11111111100)
    {
        mnemonic = "PRNL";
        format = "R";
    }
    if (rdShift == 0b11111111101)
    {
        mnemonic = "PRNT";
        format = "R";
    }
    if (rdShift == 0b11001011000)
    {
        mnemonic = "SUB";
        format = "R";
    }
    if (rdShift == 0b11101011000)
    {
        mnemonic = "SUBS";
        format = "R";
    }
    if (rdShift == 0b11111000010)
    { // Data
        mnemonic = "LDUR";
        format = "D";
    }
    if (rdShift == 0b11111000000)
    {
        mnemonic = "STUR";
        format = 'D';
    }

    switch (format)
    {
    case 'B':
        return "1";

    case 'C':
        return "2";

    case 'I':
        return "3";

    case 'R':
        return "4";

    case 'D':
        return "5";
    default:
        return "ajhhhhhhh";
    }
}

char *bToD(uint32_t binary, bool isImmediate, int bits, bool telemetry)
{
    int deci = 0;
    char *sDeci;
    char *rtn;
    bool bitVal;
    bool neg = 0;

    if (binary >> (bits - 1) & 0x01 == 1)
    {
        neg = 1;
        binary = ~binary - 1;
    }

    for (int i = 0; i < bits; i++)
    {                                                                               // converter
        bitVal = ((int)(binary % pow(10.0, (double)(bits + 1 - i))) >> (bits - i)); // finds one bit, going from most to least important:
        // ie. 10010: 1 - 1; 2 - 0; 3 - 0; 4 - 1; 5 - 0

        if (telemetry)
        {
            printf("%d : %d", i, bitVal);
        }

        deci += pow(2 * bitVal, bits - 1 - i);
    }

    if (telemetry)
    {
        printf("Value = %d", deci);
    }

    sprintf(*sDeci, "%d", deci); // change to str and return

    if (isImmediate)
    {
        *rtn = "#";
    }
    else
    {
        *rtn = "X";
    }
    strcat(*rtn, *sDeci);

    if (telemetry)
    {
        printf("String value : %s", *rtn);
    }

    return rtn;
}

char[4] branchConditional(uint32_t Rt)
{
    switch (rt)
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

int binaryToIntwithNegative(uint32_t binary, int bitLength)
{
    // number is a branch address
    if (bitLength == 26)
    {
        // Is negative (MSB is 1)
        if (((binary >> 25) & 0x1) == 1)
        {
            binary -= 1;
            binary = ~binary;
        }
    }
    // Number is a conditional branch address
    else if (bitLength == 19)
    {
        // Is negative (MSB is 1)
        if (((binary >> 23) & 0x1) == 1)
        {
            binary -= 1;
            binary = ~binary;
        }
    }
    // D type
    else
    {
        // Is negative (MSB is 1)
        if (((binary >> 20) & 0x1) == 1)
        {
            binary -= 1;
            binary = ~binary;
        }
    }
}