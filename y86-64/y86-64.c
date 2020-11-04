#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

const int MAX_MEM_SIZE = (1 << 13);

// RESOURCES: Book and slides
// CHECK OUT CCOND IF

// SF (sign flag) = valE < 0
// ZF (zero flag) = valE == 0
// OF (overflow flag)
// ^ Did I take a negative value and a negative value and get a positive value
// valA < 0 && val B < 0 -> valE < 0
// valA > 0 && val B > 0 -> valE > 0
// ^ Have to figure out how to compute this (with if statements, not so hard)
// THEN CALL SETFLAGS (setFlags)

// FROM TABLES: this means SET CC

void fetchStage(int *icode, int *ifun, int *rA, int *rB, wordType *valC,
                wordType *valP) {
  wordType pc = getPC();
  byteType byte = getByteFromMemory(pc);

  // INSTRUCTION ENCODING SLIDE (AND PATTERN)
  *icode = (byte >> 4) & 0xf;
  *ifun = byte & 0xf;

  if (*icode == HALT) {
    *valP = pc + 1;
    setStatus(STAT_HLT);
  }

  if (*icode == NOP) {
    *valP = pc + 1;
  }

  if (*icode == IRMOVQ) {
    // irmovq v, rB

    // FETCH
    // icodc:ifun <- M_1[PC] // instruction byte
    // rA:rB <- M_1[PC + 1] // Input (rA == 0xF in this case)
    // valC <- M_8[PC + 2] // 8 bits on the end constitute the immediate value
    byte = getByteFromMemory(pc + 1);
    *rA = (byte >> 4) & 0xf;
    *rB = byte & 0xf;
    *valC = getWordFromMemory(pc + 2);

    // valP <- PC + 10 // irmovq's "size" is 10 bytes, so we move forward by 10
    // bytes
    *valP = pc + 10;

    // DECODE
    // (nothing because we don't need to _READ_ anything from registers)
    // (i think)

    // EXECUTE
    // valE <- 0 + valC // valC is our "immediate", add 0 so that it goes
    // through the ALU and gets us to valE

    // MEMORY
    // (nothing, memory is untouched)

    // Write back
    // R[rB] <- valE // Update register file, gets valE. I think that we just
    // executed to shove valC through valE
  }

  if (*icode == RRMOVQ) {
    byte = getByteFromMemory(pc + 1);
    *rA = (byte >> 4) & 0xf;
    *rB = byte & 0xf;

    *valP = pc + 2;
  }

  if (*icode == RMMOVQ) {
    byte = getByteFromMemory(pc + 1);
    *rA = (byte >> 4) & 0xf;
    *rB = byte & 0xf;
    *valC = getWordFromMemory(pc + 2);

    *valP = pc + 10;
  }

  if (*icode == OPQ) {
    byte = getByteFromMemory(pc + 1);
    *rA = (byte >> 4) & 0xf;
    *rB = byte & 0xf;

    *valP = pc + 2;
  }

  if (*icode == JXX) {
    byte = getByteFromMemory(pc + 1);

    *valC = getWordFromMemory(pc + 2);
    *valP = pc + 9;
  }
}

// VIN: Added * to icode. This could be a problem if you're looking for one.
void decodeStage(int icode, int rA, int rB, wordType *valA, wordType *valB) {
  if (icode == RRMOVQ) {
    *valA = getRegister(rA);
  }

  if (icode == RMMOVQ) {
    *valA = getRegister(rA);
    *valB = getRegister(rB);
  }

  if (icode == OPQ) {
    *valA = getRegister(rA);
    *valB = getRegister(rB);
  }
}

void executeStage(int icode, int ifun, wordType valA, wordType valB,
                  wordType valC, wordType *valE, bool *Cnd) {
  if (icode == IRMOVQ) {
    // 0 + not strictly necessary
    *valE = 0 + valC;
  }

  if (icode == RRMOVQ) {
    *valE = 0 + valA;
  }

  if (icode == RMMOVQ) {
    *valE = valB + valA;
  }

  if (icode == OPQ) {
    if (ifun == ADD) {
      *valE = valB + valA;
    } else if (ifun == SUB) {
      *valE = valB - valA;
    } else if (ifun == AND) {
      *valE = valB & valA;
    } else if (ifun == XOR) {
      *valE = valB ^ valA;
    }

    bool overflow = ((valA < 0) == (valB < 0)) && ((*valE < 0) != (valA < 0));
    setFlags(*valE < 0, *valE == 0, overflow);
  }

  if (icode == JXX) {
    *Cnd = Cond(ifun);
  }
}

void memoryStage(int icode, wordType valA, wordType valP, wordType valE,
                 wordType *valM) {
  if (icode == RMMOVQ) {
    setWordInMemory(valE, valA);
  }
}

void writebackStage(int icode, wordType rA, wordType rB, wordType valE,
                    wordType valM) {
  if (icode == IRMOVQ || icode == RRMOVQ || icode == OPQ) {
    setRegister(rB, valE);
  }
}

void pcUpdateStage(int icode, wordType valC, wordType valP, bool Cnd,
                   wordType valM) {
  if (icode == NOP || icode == IRMOVQ || icode == RRMOVQ || icode == RMMOVQ ||
      icode == OPQ) {
    setPC(valP);
  }

  if (icode == JXX) {
    setPC(Cnd ? valC : valP);
  }
}

void stepMachine(int stepMode) {
  /* FETCH STAGE */
  int icode = 0, ifun = 0;
  int rA = 0, rB = 0;
  wordType valC = 0;
  wordType valP = 0;

  /* DECODE STAGE */
  wordType valA = 0;
  wordType valB = 0;

  /* EXECUTE STAGE */
  wordType valE = 0;
  bool Cnd = 0;

  /* MEMORY STAGE */
  wordType valM = 0;

  fetchStage(&icode, &ifun, &rA, &rB, &valC, &valP);
  applyStageStepMode(stepMode, "Fetch", icode, ifun, rA, rB, valC, valP, valA,
                     valB, valE, Cnd, valM);

  decodeStage(icode, rA, rB, &valA, &valB);
  applyStageStepMode(stepMode, "Decode", icode, ifun, rA, rB, valC, valP, valA,
                     valB, valE, Cnd, valM);

  executeStage(icode, ifun, valA, valB, valC, &valE, &Cnd);
  applyStageStepMode(stepMode, "Execute", icode, ifun, rA, rB, valC, valP, valA,
                     valB, valE, Cnd, valM);

  memoryStage(icode, valA, valP, valE, &valM);
  applyStageStepMode(stepMode, "Memory", icode, ifun, rA, rB, valC, valP, valA,
                     valB, valE, Cnd, valM);

  writebackStage(icode, rA, rB, valE, valM);
  applyStageStepMode(stepMode, "Writeback", icode, ifun, rA, rB, valC, valP,
                     valA, valB, valE, Cnd, valM);

  pcUpdateStage(icode, valC, valP, Cnd, valM);
  applyStageStepMode(stepMode, "PC update", icode, ifun, rA, rB, valC, valP,
                     valA, valB, valE, Cnd, valM);

  incrementCycleCounter();
}

/**
 * main
 * */
int main(int argc, char **argv) {
  int stepMode = 0;
  FILE *input = parseCommandLine(argc, argv, &stepMode);

  initializeMemory(MAX_MEM_SIZE);
  initializeRegisters();
  loadMemory(input);

  applyStepMode(stepMode);
  while (getStatus() != STAT_HLT) {
    stepMachine(stepMode);
    applyStepMode(stepMode);
#ifdef DEBUG
    printMachineState();
    printf("\n");
#endif
  }
  printMachineState();
  return 0;
}
