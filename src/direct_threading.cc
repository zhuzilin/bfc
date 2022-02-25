#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <stack>
#include <unordered_map>

struct instruct_t {
  void* label;
  char c;
  union {
    char count;
    instruct_t* next;
    instruct_t* prev;
  };
};

bool is_cmd(char c) {
  return c == '>' || c == '<' || c == '+' || c == '-' ||
         c == '.' || c == ',' || c == '[' || c == ']';
}

std::vector<instruct_t> compact(
  const std::string& source,
  std::unordered_map<char, void*>& char2label
) {
  std::vector<instruct_t> insts;
  size_t i = 0;
  std::stack<size_t> lbracket;
  while (i < source.size()) {
    char c = source[i];
    i++;
    if (!is_cmd(c)) continue;
    instruct_t inst;
    inst.label = char2label[c];
    inst.c = c;
    if (c == '>' || c == '<' || c == '+' || c == '-') {
      inst.count = 1;
      while (i < source.size() && source[i] == c) {
        i++;
        inst.count++;
      }
    }
    insts.push_back(inst);
  }
  instruct_t last_inst;
  last_inst.label = char2label[EOF];
  insts.push_back(last_inst);
  return insts;
}

void link(std::vector<instruct_t>& insts) {
  std::stack<size_t> lbracket;
  for (size_t i = 0; i < insts.size(); i++) {
    if (insts[i].c == '[') {
      lbracket.push(i);
    } else if (insts[i].c == ']') {
      size_t j = lbracket.top();
      lbracket.pop();
      insts[j].next = insts.data() + i + 1;
      insts[i].prev = insts.data() + j + 1;
    }
  }
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

  size_t lbracket_array[30000] = {0};
  size_t* lbracket = lbracket_array;

  char array[30000] = {0};
  char *ptr = array;

  std::vector<instruct_t> insts;
  instruct_t* inst;

  std::unordered_map<char, void*> char2label;

  goto preprocess;

move_right:  // >
  ptr += inst->count; inst++;
  goto *inst->label;
move_left:   // <
  ptr -= inst->count; inst++;
  goto *inst->label;
add:  // +
  *ptr += inst->count; inst++;
  goto *inst->label;
sub:  // -
  *ptr -= inst->count; inst++;
  goto *inst->label;
dot:  // .
  putchar(*ptr); inst++;
  goto *inst->label;
comma:  // ,
  *ptr = getchar(); inst++;
  goto *inst->label;
lbrac:  // [
  if (*ptr) { inst++; } else { inst = inst->next; }
  goto *inst->label;
rbrac:  // ]
  if (*ptr) { inst = inst->prev; } else { inst++; }
  goto *inst->label;
end:
  return 0;

preprocess:
  char2label = {
    {'>', &&move_right},
    {'<', &&move_left},
    {'+', &&add},
    {'-', &&sub},
    {'.', &&dot},
    {',', &&comma},
    {'[', &&lbrac},
    {']', &&rbrac},
    {EOF, &&end},
  };

  insts = compact(source, char2label);
  link(insts);

  inst = insts.data();
  goto *inst->label;
}