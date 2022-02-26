#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <stack>
#include <regex>
#include <unordered_map>

constexpr size_t kValueSize = 8;

struct instruct_t {
  void* label;
  char c;
  union {
    int count;
    instruct_t* next;
    instruct_t* prev;
    char values[kValueSize];
  };
};

inline bool is_cmd(char c) {
  return c == '>' || c == '<' || c == '+' || c == '-' ||
         c == '.' || c == ',' || c == '[' || c == ']';
}

std::vector<instruct_t> compact(
  const std::string& source,
  std::unordered_map<char, void*>& char2label
) {
  std::vector<instruct_t> insts;
  size_t i = 0;
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
      if (c == '<') {
        inst.count = -inst.count;
        inst.c = '>';
      }
      if (c == '-') {
        inst.count = -inst.count;
        inst.c = '+';
      }
      // merge +++--- or >>><<<
      if (insts.size() > 0 && insts[insts.size() - 1].c == inst.c) {
        insts[insts.size() - 1].count += inst.count;
        if (insts[insts.size() - 1].count == 0) {
          insts.pop_back();
        }
      } else {
        insts.push_back(inst);
      }
    } else {
      insts.push_back(inst);
    }
  }
  instruct_t last_inst;
  last_inst.label = char2label[EOF];
  insts.push_back(last_inst);
  return insts;
}

bool optimize(
  std::vector<instruct_t>& insts,
  std::unordered_map<char, void*>& char2label
) {
  bool optimized = false;
  std::vector<instruct_t> new_insts;
  size_t l = 0, r = 0;
  while (l < insts.size() && r < insts.size()) {
    // let l go to the next [
    while (l < insts.size() && insts[l].c != '[') {
      new_insts.push_back(insts[l]);  // This will not push '[' into new_insts
      l++;
    }
    if (l == insts.size()) break;
    assert(insts[l].c == '[');
    // let r go to the next ]
    r = l + 1;
    while (r < insts.size() && insts[r].c != ']' && insts[r].c != '[') {
      r++;
    }
    if (r == insts.size()) {
      throw std::runtime_error("no pairing ']' for '['");
    } else if (insts[r].c == '[') {  // nested '['
      new_insts.push_back(insts[l]);
      l++;
      continue;
    }
    assert(insts[r].c == ']');
    if (r - l >= 2 && insts[l + 1].c == '+' && insts[l + 1].count == -1) {
      // [-]: *ptr = 0;
      if (r - l == 2) {
        instruct_t inst;
        inst.c = 'c';
        inst.label = char2label['c'];
        new_insts.push_back(inst);
        optimized = true;
        l = r + 1;
        continue;
      }
      if (r - l <= kValueSize + 3 && insts[l + 2].c == '>' && insts[r - 1].c == '>') {
        bool flag = true;
        char sum = 0;
        for (size_t i = l + 2; i < r - 1; i++) {
          if (insts[i].c != '+' && insts[i].c != '>') {
            flag = false;
            break;
          }
          if (insts[i].c == '>') {
            sum += insts[i].count;
          }
        }
        sum += insts[r - 1].count;
        if (flag && sum == 0) {
          instruct_t inst;
          char offset = 0;
          size_t value_idx = 0;
          bool is_sum = true;
          for (size_t i = l + 2; i < r - 1; i += 2) {
            assert(insts[i].c == '>' && insts[i + 1].c == '+');
            offset += insts[i].count;
            inst.values[value_idx] = offset;
            inst.values[value_idx + 1] = insts[i + 1].count;
            value_idx += 2;
            if (insts[i + 1].count != 1)
              is_sum = false;
          }
          for (; value_idx < kValueSize; value_idx++) {
            inst.values[value_idx] = 0;
          }
          if (is_sum) {
            if (r - l == 5) {
              inst.c = 'x';
              inst.label = char2label['x'];
            } else {
              inst.c = 's';
              inst.label = char2label['s'];
            }
          } else {
            if (r - l == 5) {
              inst.c = 'y';
              inst.label = char2label['y'];
            } else {
              inst.c = 'm';
              inst.label = char2label['m'];
            }
          }
          new_insts.push_back(inst);
          optimized = true;
          l = r + 1;
          continue;
        }
      }
    }
    if (r - l == 2 && insts[l + 1].c == '>') {
      // [>]
      instruct_t inst;
      inst.c = 'f';
      inst.count = insts[l + 1].count;
      inst.label = char2label['f'];
      new_insts.push_back(inst);
      optimized = true;
      l = r + 1;
      continue;
    }
    while (l < r + 1) {
      new_insts.push_back(insts[l]);
      l++;
    }
    assert(l == r + 1);
  }
  insts = new_insts;
  new_insts.clear();
  return optimized;
}

void link(std::vector<instruct_t>& insts) {
  std::stack<size_t> lbracket;
  for (size_t i = 0; i < insts.size(); i++) {
    if (insts[i].c == '[') {
      lbracket.push(i);
    } else if (insts[i].c == ']') {
      if (lbracket.size() == 0) {
        throw std::runtime_error("no pairing '[' for ']'");
      }
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

move:  // >
  ptr += inst->count; inst++;
  goto *inst->label;
add:  // +
  *ptr += inst->count; inst++;
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
// peephole optimization
clear:
  *ptr = 0; inst++;
  goto *inst->label;
mul:
  for (size_t i = 0; i < kValueSize; i += 2) {
    if (inst->values[i] == 0) break;
    *(ptr + inst->values[i]) += (*ptr) * inst->values[i + 1];
  }
  *ptr = 0; inst++;
  goto *inst->label;
single_mul:
  *(ptr + inst->values[0]) += (*ptr) * inst->values[1];
  *ptr = 0; inst++;
  goto *inst->label;
sum:
  for (size_t i = 0; i < kValueSize; i += 2) {
    if (inst->values[i] == 0) break;
    *(ptr + inst->values[i]) += (*ptr);
  }
  *ptr = 0; inst++;
  goto *inst->label;
single_sum:
  *(ptr + inst->values[0]) += (*ptr);
  *ptr = 0; inst++;
  goto *inst->label;
find_zero:
  while(*ptr) ptr += inst->count; inst++;
  goto *inst->label;

preprocess:
  char2label = {
    {'>', &&move},
    {'<', &&move},
    {'+', &&add},
    {'-', &&add},
    {'.', &&dot},
    {',', &&comma},
    {'[', &&lbrac},
    {']', &&rbrac},
    {EOF, &&end},
    {'c', &&clear},
    {'m', &&mul},
    {'s', &&sum},
    {'x', &&single_sum},
    {'y', &&single_mul},
    {'f', &&find_zero}
  };

  insts = compact(source, char2label);
  optimize(insts, char2label);
  link(insts);

  inst = insts.data();
  goto *inst->label;
}