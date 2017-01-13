#include <iostream>
#include <fstream>
#include <algorithm>

#include "freeling.h"
#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/word_vector.h"

using namespace std;
using namespace freeling;

int main(int argc, char* argv[]){

  // set locale to an UTF8 compatible locale
  freeling::util::init_locale(L"default");

  // print usage if config-file missing
  if (argc != 2) {
    wcerr<<L"Usage:  convert_model model-file" << endl; 
    exit(1);
  }
  
  /// path to data files
  wstring path = L"/usr/local/share/freeling/";
  
  // create a word_vector module with the given model
  std::wcout << L"Initializing word_vector module..." << std::flush;
  wstring model_path = freeling::util::string2wstring(argv[1]);
  freeling::word_vector wordVec(model_path, L"??");
  std::wcout << L" DONE" << std::endl;
  
  // warn the user if he's not using a binary model
  if (model_path.substr(model_path.length() - 4) != L".bin") {
    std::wcout << L"Converting model to binary format..." << std::flush;
    std::string::size_type found = model_path.find_last_of(L".");
    wordVec.dump_model(model_path.substr(0, found) + L".bin");
    std::wcout << L" DONE" << std::endl;
  } else {
    std::wcout << L"Converting model to words format..." << std::flush;
    std::string::size_type found = model_path.find_last_of(L".");
    wordVec.dump_model(model_path.substr(0, found) + L".words");
    std::wcout << L" DONE" << std::endl;
  }
}