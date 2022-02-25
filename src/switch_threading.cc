#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <stack>

struct instruct_t {
  char c;
  char count;
};

bool is_cmd(char c) {
  return c == '>' || c == '<' || c == '+' || c == '-' ||
         c == '.' || c == ',' || c == '[' || c == ']';
}

std::vector<instruct_t> compact(const std::string& source) {
  std::vector<instruct_t> insts;
  size_t i = 0;
  std::stack<size_t> lbracket;
  while (i < source.size()) {
    instruct_t inst;
    inst.c = source[i];
    inst.count = 1;
    i++;
    if (!is_cmd(inst.c)) continue;
    while (i < source.size() && source[i] == inst.c &&
           (inst.c == '>' || inst.c == '<' || inst.c == '+' || inst.c == '-')) {
      i++;
      inst.count++;
    }
    insts.push_back(inst);
  }
  
  for (size_t i = 0; i < insts.size(); i++) {
    if (insts[i].c == '[') {
      lbracket.push(i);
    } else if (insts[i].c == ']') {
      if (lbracket.size() == 0) {
        throw std::runtime_error("no pairing '[' for ']'");
      }
    }
  }
  return insts;
}

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

  std::vector<instruct_t> insts = compact(source);

  size_t lbracket_array[30000] = {0}; size_t* lbracket = lbracket_array;
  char array[30000] = {0}; char *ptr = array;

  size_t pc = 0;
  while (pc < insts.size()) {
    instruct_t inst = insts[pc];
    switch (inst.c) {
      case '>': ptr += inst.count; break;
      case '<': ptr -= inst.count; break;
      case '+': *ptr += inst.count; break;
      case '-': *ptr -= inst.count; break;
      case '.': putchar(*ptr); break;
      case ',': *ptr = getchar(); break;
      case '[':
        if (*ptr == 0) {
          size_t count = 1;
          do {
            pc++;
            if (insts[pc].c == '[')
              count++;
            if (insts[pc].c == ']')
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