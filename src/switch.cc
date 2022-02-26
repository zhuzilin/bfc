#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <stack>

struct instruct_t {
  char c;
  union {
    int count;
    size_t next;
    size_t prev;
  };
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
    char c = source[i];
    i++;
    if (!is_cmd(c)) continue;
    instruct_t inst;
    inst.c = c;
    if (c == '>' || c == '<' || c == '+' || c == '-') {
      inst.count = 1;
      while (i < source.size() && source[i] == c) {
        i++;
        inst.count++;
      }
      if (c == '<') {
        inst.count = -inst.count;
        inst.c = '>';
      }
      if (c == '-') {
        inst.count = -inst.count;
        inst.c = '+';
      }
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
      size_t j = lbracket.top();
      lbracket.pop();
      insts[j].next = i + 1;
      insts[i].prev = j + 1;
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
      case '>': ptr += inst.count; pc++; break;
      case '+': *ptr += inst.count; pc++; break;
      case '.': putchar(*ptr); pc++; break;
      case ',': *ptr = getchar(); pc++; break;
      case '[':
        if (*ptr) { pc++; } else { pc = inst.next; }
        break;
      case ']':
        if (*ptr) { pc = inst.prev; } else { pc++; }
        break;
      default:
        break;
    }
  }
}