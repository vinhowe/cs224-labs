#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRUE 1
#define FALSE 0
#define BAD_NUMBER_ARGS 1
int BYTES_PER_LINE_HEX = 16;
int BYTES_PER_LINE_BINARY = 6;

/**
 * Parses the command line.
 *
 * argc: the number of items on the command line (and length of the
 *       argv array) including the executable
 * argv: the array of arguments as strings (char* array)
 * bits: the integer value is set to TRUE if bits output indicated
 *       outherwise FALSE for hex output
 *
 * returns the input file pointer (FILE*)
 **/
FILE *parseCommandLine(int argc, char **argv, int *bits) {
  if (argc > 2) {
    printf("Usage: %s [-b|-bits]\n", argv[0]);
    exit(BAD_NUMBER_ARGS);
  }

  if (argc == 2 &&
      (strcmp(argv[1], "-b") == 0 || strcmp(argv[1], "-bits") == 0)) {
    *bits = TRUE;
  } else {
    *bits = FALSE;
  }

  return stdin;
}

/**
 * Writes data to stdout in hexadecimal.
 *
 * See myxxd.md for details.
 *
 * data: an array of no more than 16 characters
 * size: the size of the array
 **/
void printDataAsHex(unsigned char *data, size_t size) {
  for (int i = 0; i < BYTES_PER_LINE_HEX; i++) {
    if (i < BYTES_PER_LINE_HEX - 1 && !(i % 2)) {
        printf(" ");
    }

    if (i < (int)size) {
      printf("%02x", *(data + i));
    } else {
      printf("%*c", 2, ' ');
    }
  }
}

/**
 * Writes data to stdout as characters.
 *
 * See myxxd.md for details.
 *
 * data: an array of no more than 16 characters
 * size: the size of the array
 **/
void printDataAsChars(unsigned char *data, size_t size) {
  for (int i = 0; i < (int) size; i++) {
    unsigned char currentChar = *(data + i);

    if (currentChar > 126 || currentChar < 32) {
        currentChar = '.';
    }

    printf("%c", currentChar);
  }
}

void readAndPrintInputAsHex(FILE *input) {
  unsigned char data[BYTES_PER_LINE_HEX];
  int numBytesRead = fread(data, 1, BYTES_PER_LINE_HEX, input);
  unsigned int offset = 0;
  while (numBytesRead != 0) {
    printf("%08x:", offset);
    offset += numBytesRead;
    printDataAsHex(data, numBytesRead);
    printf("  ");
    printDataAsChars(data, numBytesRead);
    printf("\n");
    numBytesRead = fread(data, 1, BYTES_PER_LINE_HEX, input);
  }
}

/**
 * Writes data to stdout in binary.
 *
 * See myxxd.md for details.
 *
 * data: an array of no more than 6 characters
 * size: the size of the array
 **/
void printDataAsBinary(unsigned char *data, size_t size) {
  for (int i = 0; i < BYTES_PER_LINE_BINARY; i++) {
    printf(" ");

    if (i < (int)size) {
      char byteString[8];
      unsigned char currentChar = *(data + i);

      for (int i = 7; i >= 0; i--) {
        byteString[i] = currentChar % 2 == 1 ? '1' : '0';
        currentChar /= 2;
      }

      printf("%s", byteString);
    } else {
      // 8 spaces where a byte would go
      printf("%*c", 8, ' ');
    }
  }
}

/**
 * Bits output for xxd.
 *
 * See myxxd.md for details.
 *
 * input: input stream
 **/
void readAndPrintInputAsBits(FILE *input) {
  unsigned char data[BYTES_PER_LINE_BINARY];
  int numBytesRead = fread(data, 1, BYTES_PER_LINE_BINARY, input);
  unsigned int offset = 0;
  while (numBytesRead != 0) {
    printf("%08x:", offset);
    offset += numBytesRead;
    printDataAsBinary(data, numBytesRead);
    printf("  ");
    printDataAsChars(data, numBytesRead);
    printf("\n");
    numBytesRead = fread(data, 1, BYTES_PER_LINE_BINARY, input);
  }
}

int main(int argc, char **argv) {
  int bits = FALSE;
  FILE *input = parseCommandLine(argc, argv, &bits);

  if (bits == FALSE) {
    readAndPrintInputAsHex(input);
  } else {
    readAndPrintInputAsBits(input);
  }
  return 0;
}
