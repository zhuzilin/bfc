#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

int main(int argc, char* argv[]) {
  if (argc < 2) {
    printf("no filename provided.");
    return -1;
  }
  std::string filename(argv[1]);
  std::ifstream file(filename);
  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string source = buffer.str();

  size_t lbracket_array[30000] = {0}; size_t* lbracket = lbracket_array;
  char array[30000] = {0}; char *ptr = array;

  size_t pc = 0;
  while (pc < source.size()) {
    switch (source[pc]) {
      case '>': ++ptr; break;
      case '<': --ptr; break;
      case '+': ++*ptr; break;
      case '-': --*ptr; break;
      case '.': putchar(*ptr); break;
      case ',': *ptr = getchar(); break;
      case '[':
        if (*ptr == 0) {
          size_t count = 1;
          do {
            pc++;
            if (source[pc] == '[')
              count++;
            if (source[pc] == ']')
              count--;
          } while (count);
        } else {
          lbracket++;
          *lbracket = pc;
        }
        break;
      case ']':
        if (*ptr == 0) {
          lbracket--;
        } else {
          pc = *lbracket;
        }
        break;
      default:
        break;
    }
    pc++;
  }
}