#include <exception>
#include <iostream>
#include <string>

using namespace std;

#include "machine_object.h"

int main(int argc, char ** argv)
{
  try {
    if(argc != 3) {
      cerr << "usage: mobjroundtrip <input.o> <output.o>\n";
      return 1;
    }
    machine_object::write_object_file(argv[2], machine_object::parse_object_file(argv[1]));
    return 0;
  } catch(const exception & ex) {
    cerr << "mobjroundtrip: " << ex.what() << "\n";
    return 1;
  }
}
