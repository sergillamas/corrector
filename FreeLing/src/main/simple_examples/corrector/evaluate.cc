#include <iostream>
#include <fstream>

#include "freeling.h"
#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/corrector.h"
#include "include/remove_repeated.h"

using namespace std;
using namespace freeling;

int main(int argc, char* argv[]){

  // set locale to an UTF8 compatible locale
  freeling::util::init_locale(L"default");

  // print usage if config-file missing
  if (argc != 2) {
    wcerr << L"Usage:  evaluate config-file" << endl; 
    exit(1);
  }
  
  /// path to data files
  wstring path = L"/usr/local/share/freeling/";
  
  // create a noisy text normalization module with the given config file
  wstring cfgFile = freeling::util::string2wstring(argv[1]);
  std::wcout << L"Initializing corrector module..." << std::flush;
  freeling::corrector corrector(cfgFile);
  std::wcout << L" DONE" << std::endl;
  
  // set the language
  const wstring lang = corrector.get_language();
  
  // accept twitter text (this detects @tags, #hashtags, :D smilies, url's, etc.)
  // set to false to use the default tokenizer and usermap
  bool is_twitter = true;
  
  // create modules and analyzer
  std::wcout << L"Initializing analyzer and other modules";
  if (is_twitter) std::wcout << L" (using twitter tokenizer and usermap)";
  std::wcout << L"..." << std::flush;
  wstring tokenizer_file;
  if (is_twitter) tokenizer_file = path + lang + L"/twitter/tokenizer.dat";
  else tokenizer_file = path + lang + L"/tokenizer.dat";
  freeling::tokenizer tk(tokenizer_file); 
  freeling::splitter sp(path + lang + L"/splitter.dat");
  freeling::splitter::session_id sid = sp.open_session();
  
  // morphological analysis module and options
  maco_options opt(lang);
  wstring mapfile;
  if (is_twitter) mapfile = path+lang+L"/twitter/usermap.dat";
  else mapfile = L"";
  opt.UserMapFile=mapfile;
  opt.LocutionsFile=path+lang+L"/locucions.dat"; opt.AffixFile=path+lang+L"/afixos.dat";
  opt.ProbabilityFile=path+lang+L"/probabilitats.dat"; opt.DictionaryFile=path+lang+L"/dicc.src";
  opt.NPdataFile=path+lang+L"/np.dat"; opt.PunctuationFile=path+L"/common/punct.dat"; 
  maco morfo(opt);
  morfo.set_active_options (is_twitter, // UserMap
                            true,    // NumbersDetection,
                            true,    //  PunctuationDetection,
                            true,    //  DatesDetection,
                            true,    //  DictionarySearch,
                            true,    //  AffixAnalysis,
                            false,   //  CompoundAnalysis,
                            true,    //  RetokContractions,
                            true,    //  MultiwordsDetection,
                            true,    //  NERecognition,
                            false,   //  QuantitiesDetection,
                            false);  //  ProbabilityAssignment
  
  // create alternative porposers
  freeling::alternatives alts_ort(path + lang + L"/alternatives-key.dat");
  freeling::alternatives alts_phon(path + lang + L"/alternatives-phon.dat");
  std::wcout << L" DONE" << endl;
  
  // process input
  wstring line;
  list<freeling::word> lw;
  list<freeling::sentence> ls;
  std::vector<corrector_evaluation_t> corrections_vec;
  std::wcout << L"Write EXIT when you want to close the program and get the results. " << endl;
  std::wcout << L"Write a sentence and the corrections in the format (incorrect_word corrected_word, inco..): " << endl;
  while (getline(wcin, line)) {
    if (line == L"EXIT") break;
    
    // get input as list of sentences
    lw = tk.tokenize(line);
    ls = sp.split(sid, lw, true);
    
    // perform morphosyntactic analysis
    morfo.analyze(ls);
    
    // clean words with too many repetitions that would hang the alternatives module otherwise
    for (list<freeling::sentence>::iterator s = ls.begin(); s != ls.end(); s++) {
      for (freeling::sentence::iterator w = s->begin(); w != s->end(); w++) {
        std::wstring no_repeated = std::wstring(w->get_lc_form());
        remove_repeated(no_repeated);
        if (no_repeated.length() + 3 < w->get_lc_form().length()) {
          w->set_form(no_repeated);
        }
      }
    }

    // propose alternative forms
    alts_ort.analyze(ls);
    alts_phon.analyze(ls);
    
    // for each word, if the word has to be corrected and has consecutive repeated 
    // characters, add that new form and its alternatives
    for (list<freeling::sentence>::iterator s = ls.begin(); s != ls.end(); s++) {
      for (freeling::sentence::iterator w = s->begin(); w != s->end(); w++) {
        if (w->has_alternatives()) { // if word has to be corrected
          std::wstring no_repeated = std::wstring(w->get_lc_form());
          remove_repeated(no_repeated);
          if (no_repeated != w->get_lc_form()) {
            // get similar words to the new form
            list<std::pair<std::wstring, int>> alts;
            alts_ort.get_similar_words(no_repeated, alts);
            alts_phon.get_similar_words(no_repeated, alts);
            
            // add forms
            alts.push_back(std::pair<std::wstring, int>(no_repeated, 30));
            for (auto alt = alts.begin(); alt != alts.end(); alt++) {
              w->add_alternative(alt->first, alt->second + 30);
            }
            
            // remove repeated alternatives (sort + unique)
            w->get_alternatives().sort(
                          [](const freeling::alternative &left,
                            const freeling::alternative &right){
                            if (left.get_form() == right.get_form()) {return left.get_distance() < right.get_distance();}
                            else {return left.get_form().compare(right.get_form()) < 0;}});
            w->get_alternatives().unique(
                          [](const freeling::alternative &left,
                            const freeling::alternative &right){
                            return left.get_form() == right.get_form();});
            w->get_alternatives().sort(
                          [](const freeling::alternative &left,
                            const freeling::alternative &right){
                            return left.get_distance() < right.get_distance();});
          }
        }
      }
    }
    
    // get corrections
    std::list<std::pair<std::wstring, std::wstring>> corrections;
    std::pair<std::wstring, std::wstring> correction_pair;
    unsigned int words = 0;
    wstring corrections_line;
    getline(wcin, corrections_line);
    wistringstream wiss(corrections_line);
    wstring word;
    
    while (wiss >> word) {
      if (word != L",") {
        if (words % 2 == 0) { //even
          correction_pair.first  = word;
        } else { //odd
          correction_pair.second = word;
          std::pair<std::wstring, std::wstring> pair_copy(correction_pair);
          corrections.push_back(pair_copy);
        }
        ++words;
      }
    }
    
    // evaluate text with corrector
    corrector_evaluation_t correction;
    correction = corrector.evaluate(ls, corrections);
    corrections_vec.push_back(correction);
    
    // clear temporary lists
    lw.clear(); ls.clear();
  }
  
  // print results
  wcout << endl;
  corrector.print_evaluation_results(corrections_vec);
  wcout << endl;
}