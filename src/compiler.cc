#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

std::string space(size_t ident) {
  return std::string(ident * 2, ' ');
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

  size_t lbracket_array[30000] = {0}; size_t* lbracket = lbracket_array;
  char array[30000] = {0}; char *ptr = array;

  std::string ccode = "#include <stdio.h>\n"
                      "\n"
                      "int main() {\n";
  size_t indent = 1;
  ccode += space(indent) + "char array[30000] = {0}; char *ptr = array;\n";
  for (size_t i = 0; i < source.size(); i++) {
    switch (source[i]) {
      case '>': ccode += space(indent) + "++ptr;\n"; break;
      case '<': ccode += space(indent) + "--ptr;\n"; break;
      case '+': ccode += space(indent) + "++*ptr;\n"; break;
      case '-': ccode += space(indent) + "--*ptr;\n"; break;
      case '.': ccode += space(indent) + "putchar(*ptr);\n"; break;
      case ',': ccode += space(indent) + "*ptr = getchar();\n"; break;
      case '[': ccode += space(indent) + "while (*ptr) {\n"; indent++; break;
      case ']': indent--; ccode += space(indent) + "}\n"; break;
      default:
        break;
    }
  }
  ccode += "}\n";
  std::cout << ccode;
}