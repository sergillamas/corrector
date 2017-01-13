#include <iostream>
#include <fstream>

#include "freeling.h"
#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/corrector.h"
#include "include/remove_repeated.h"

using namespace std;
using namespace freeling;

std::wstring lowercase(const std::wstring &ostr) {
  std::wstring str(ostr);
  std::transform(str.begin(), str.end(), str.begin(), ::tolower);
  return str;
}

int main(int argc, char* argv[]){

  // set locale to an UTF8 compatible locale
  freeling::util::init_locale(L"default");

  // print usage if config-file missing
  if (argc != 3) {
    wcerr << L"Usage:  tweet_norm config-file golden-standard < input" << endl; 
    exit(1);
  }
  
  // open and read golden standard
  std::wifstream infile(argv[2]);
  if (infile.fail()) {
    std::wcout << L"ERROR: Can't open file " << argv[2] << L" (golden standard)" << std::endl;
    exit(1);
  }
  std::map<std::wstring, std::list<std::pair<std::wstring, std::wstring>>> golden_standard;
  std::wstring sline;
  std::wstring golden_id;
  while (std::getline(infile, sline)) {
    std::wistringstream wiss(sline);
    std::wstring a, b;
    wiss >> a;
    if (wiss >> b) {
      // read pair of corrections
      if (b == L"-") golden_standard[golden_id].push_back(std::make_pair(a, a));
      else golden_standard[golden_id].push_back(std::make_pair(a, b));
    } else {
      // get new id
      golden_id = a;
    }
  }
  infile.close();   
  
  /// path to data files
  wstring path = L"/usr/local/share/freeling/"; // change if using custom installation
  
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
  wstring id;
  
  // read in tweet-norm format (ID user HASH line)
  FILE *results;
  results = fopen("tweet_results.txt", "wb");
  int err, pos, neg;
  err = pos = neg = 0;
  while (getline(wcin, line)) {
    // get id
    wstring out_id;
    unsigned int len = 0;
    wistringstream wiss(line);
    wiss >> out_id;
    len += out_id.length() + 1;
    
    // skip user and hash
    wiss >> id;
    len += id.length() + 1;
    wiss >> id;
    len += id.length() + 1;
    
    // get tweet
    line = line.substr(len);
    
    // skip unavailable tweets
    if (line == L"Not Available") {continue;}
    wcout << out_id << endl;
    
    // get input as list of sentences
    lw = tk.tokenize(line);
    ls = sp.split(sid, lw, true);
    
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
    
    // perform morphosyntactic analysis
    morfo.analyze(ls);
    
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
    
    // normalize text with corrector
    corrector.normalize(ls);
    
    // print corrections
    for (list<freeling::sentence>::iterator s = ls.begin(); s != ls.end(); s++) {
      for (freeling::sentence::iterator w = s->begin(); w != s->end(); w++) {
        if (w->has_alternatives()) {
          for (list<alternative>::iterator a = w->alternatives_begin(); a != w->alternatives_end(); a++) {
            if (a->is_selected()) { // you can also use a->is_selected(i) to get the i-nth best selection (starting from 1)
              // update results
              if (golden_standard.count(out_id) != 1) {
                fwprintf(results, L"ERROR: golden standard does not contain id %s\n", out_id);
              } else {// check if word had to be corrected, set neg otherwise
                bool correct = false;
                std::wstring correct_form;
                for (auto correction : golden_standard[out_id]) {
                  if (correction.first == w->get_form()) {
                    correct = true;
                    correct_form = lowercase(correction.second);
                  }
                }
                
                std::wstring my_correction = lowercase(a->get_form());
                if (!correct) {
                  ++err;
                  fwprintf(results, L"ERR %s %s\n", my_correction);
                } else {
                  if (my_correction == correct_form) {
                    ++pos;
                    fwprintf(results, L"POS %s %s\n", correct_form, my_correction);
                  } else {
                    ++neg;
                    fwprintf(results, L"NEG %s %s\n", correct_form, my_correction);
                  }
                }
              }
                
              // print form
              if (w->get_form() == a->get_form()) {
                wcout << "\t" << w->get_form() << " -" << endl;
              } else {
                wcout << "\t" << w->get_form() << " " << a->get_form() << endl;
              }
            }
          }
        }
      }
    }
    
    // clear temporary lists
    lw.clear(); ls.clear();
  }
  
  fwprintf(results, L"\nERR: %i\nPOS: %i\nNEG: %i\nPRECISION: %f%\n", err, pos, neg, 100.0*((float) pos/((float) (pos + neg + err))));
  fclose(results);
  
  sp.close_session(sid);
}