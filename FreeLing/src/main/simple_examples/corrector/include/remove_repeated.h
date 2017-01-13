#include<string>

// Modifies a wstring, removing repeated consecutive characters
// Examples:
//    hoolaaaa   =>   hola
//    VAMOOOOS   =>   VAMOS
//    aaa        =>   a
//    COMOooOo?  =>   COMOoOo?
void remove_repeated(std::wstring &str) {
  unsigned int i = str.length() - 1;
  while (i > 0) {
    if (str[i] == str[i - 1]) {
      str.erase(i - 1, 1);
    }
    i -= 1;
  }
}