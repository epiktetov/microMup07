//------------------------------------------------------+----------------------
// МикроМир07      Syntax checker / text colorizer      | (c) Epi MG, 2011-2020
//------------------------------------------------------+----------------------
#include <QString>
#include "mic.h"
#include "ccd.h" // uses ‘KbRadix’ and ‘KbCount’
#include "qfs.h"
#include "twm.h"
#include "vip.h"
extern "C" {
#include "le.h"
#include "dq.h"
#include "tx.h"
}
#include "synt.h"
#include <QRegExp>
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
short SyntKnownLang (QString filename)
{                                         // temporarily Objective-C == C++
  QRegExp ShFile(".+\\.sh");              //
  QRegExp CppFile(".+\\.(c|cpp|cxx|h|hpp|hxx|m|mm)", Qt::CaseInsensitive);
  QRegExp LuaFile(".+\\.(lua|ales)");
  QRegExp PerlFile(".+\\.pl");
  QRegExp PythonFile(".+\\.py");
       if (ShFile.exactMatch(filename))     return CLangSH + CLangDISABLED;
  else if (CppFile.exactMatch(filename))    return CLangCPP;
  else if (LuaFile.exactMatch(filename))    return CLangLUA;
  else if (PerlFile.exactMatch(filename))   return CLangPERL;
  else if (PythonFile.exactMatch(filename)) return CLangPYTH;
  else                                      return CLangNONE;
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
short SyntSniffText (txt *text)
{
  int len;   TxTop(text); if (qTxBottom(text)) return CLangNONE;
  char *tp = TxGetLn(text, &len); if (len < 3) return CLangNONE;
  QString firstLine = QString::fromUtf8(tp, len);
  QRegExp binSpec("#!(?:/usr)?(?:/local)?(?:/bin/)?(\\w+)\\s*(.*)");
  QRegExp bash("(ba)?sh");
  if (!binSpec.exactMatch(firstLine)) return CLangNONE;
  QString  progName = binSpec.cap(1);
       if (progName == "env") progName = binSpec.cap(2).section(' ',0,0);
       if (bash.exactMatch(progName)) return CLangSH + CLangDISABLED;
  else if (progName == "perl")        return CLangPERL;
  else if (progName == "python")      return CLangPYTH;
  else                                return CLangGEN + CLangDISABLED;
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SyntLangOn()
{                             short clang =               CLangNONE;
  if (Ttxt->clang  > CLangDISABLED) clang = Ttxt->clang - CLangDISABLED;
  else if (0 < KbCount && KbCount < CLangMAX)     clang =       KbCount;
  else exc(E_SFAIL);
  if (Ttxt->clustk == NIL)         TxTop(Ttxt); // if Synt was never enabled
  TxEnableSynt(Ttxt,clang); wndop(TW_ALL,Ttxt); // before => rescan from top
}
void SyntLangOff() // (temporarily) disable Syntax checker, do not clear data
{
  if (Ttxt->clang <= CLangNONE || Ttxt->clang > CLangMAX) exc(E_SFAIL);
      Ttxt->clang += CLangDISABLED;                wndop(TW_ALL, Ttxt);
}
int ShowBrakts = 1; // show-brackets mode: 0=no-pos-check, 1=normal, 2=full
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SyntBrakToggle()
{
       if (KbRadix)         ShowBrakts = KbCount;
  else if (ShowBrakts == 1) ShowBrakts = 2;
  else                      ShowBrakts = 1;
  wndop(TW_ALL, Ttxt);
}
//-----------------------------------------------------------------------------
typedef bool (*tchar_uint_fp)(const tchar *, unsigned int);
typedef bool (*tchar_int2_fp)(const tchar *, int, int);
typedef bool (* char_int2_fp)(const char  *, int, int); // pointer,len,pos
struct SyntLang_t {
  tchar_uint_fp in_word_set;
  tchar_int2_fp is_eol_com_tc, is_begin_com_tc;
   char_int2_fp is_eol_com_c,  is_begin_com_c;
  const char *  end_comment;
};
bool no_keywords    (const tchar *, unsigned int) { return false; }
bool no_comments_tc (const tchar *, int,     int) { return false; }
bool no_comments_c  (const  char *, int,     int) { return false; }
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static const char empty[] = "";
static bool pound_eol_com_tc (const tchar *tstr, int,int)
                                               { return (char)tstr[0] == '#'; }
static bool pound_eol_com_c(const char *str, int,int) { return str[0] == '#'; }
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#define BASH_TOTAL_KEYWORDS 25
#define BASH_MIN_WORD_LENGTH 2
#define BASH_MAX_WORD_LENGTH 8
#define BASH_MIN_HASH_VALUE  2
#define BASH_MAX_HASH_VALUE 40

inline unsigned int Bash_hash (const tchar *tstr, unsigned int len)
{
  static unsigned char Bash_char_values[] =
    {
      41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
      41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
      41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
      41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
      41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
      41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
      41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
      41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
      41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
      41, 41, 41, 41, 41, 41, 41, 23, 41,  0,
      41,  0, 20, 41, 15, 10, 41, 41,  0,  5,
       0,  5, 41, 41,  0, 13, 25, 10, 30, 41,
       0, 41, 41, 41, 41, 41, 41, 41, 41, 41,
      41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
      41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
      41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
      41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
      41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
      41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
      41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
      41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
      41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
      41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
      41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
      41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
      41, 41, 41, 41, 41, 41
    };
  unsigned int hval = len;
  switch (hval) {
  default: hval += Bash_char_values[(unsigned char)tstr[3]]; // and FALLTHROUGH
  case 3:
  case 2:  hval += Bash_char_values[(unsigned char)tstr[1]];
  }
  return hval;
}
static bool Bash_in_word_set (const tchar *tstr, unsigned int len)
{
  static const char * wordlist[] =
    {
      "", "", "in", "", "else", "unset", "select", "do", "for", "done", "",
      "export", "fi", "", "time", "until", "return", "esac", "function",
      "then", "while", "", "if", "umask", "elif", "", "", "case", "break",
      "exit", "", "", "", "", "eval", "", "", "", "continue", "", "shift"
    };
  if (BASH_MIN_WORD_LENGTH <= len && len <= BASH_MAX_WORD_LENGTH) {
    int key = Bash_hash(tstr, len);
    if (key <= BASH_MAX_HASH_VALUE) {
      const char *word = wordlist[key];
      while (len--)
        if ((char)(*tstr++) != *word++) return false;
                                        return true;
  } }
  return false;
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#define LUA_TOTAL_KEYWORDS 21
#define LUA_MIN_WORD_LENGTH 2
#define LUA_MAX_WORD_LENGTH 8
#define LUA_MIN_HASH_VALUE 2
#define LUA_MAX_HASH_VALUE 28

inline unsigned int Lua_hash (const tchar *tstr, unsigned int len)
{
  static unsigned char Lua_char_values[] =
    {
      29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
      29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
      29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
      29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
      29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
      29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
      29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
      29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
      29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
      29, 29, 29, 29, 29, 29, 29, 10, 10, 29,
      15,  5,  5, 29, 29,  0, 29, 10,  0, 29,
       0, 10, 29, 29,  0, 29, 15, 15, 29,  0,
      29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
      29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
      29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
      29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
      29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
      29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
      29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
      29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
      29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
      29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
      29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
      29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
      29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
      29, 29, 29, 29, 29, 29
    };
  return len + Lua_char_values[(unsigned char)tstr[len - 1]]
             + Lua_char_values[(unsigned char)tstr[0]];
}
static bool Lua_in_word_set (const tchar *tstr, unsigned int len)
{
  static const char * wordlist[] =
    {
      "", "", "in", "nil", "", "local", "return", "if", "for", "", "while",
      "", "or", "function", "else", "false", "elseif", "", "not", "then",
      "until", "repeat", "", "end", "true", "break", "", "do", "and"
    };
  if (LUA_MIN_WORD_LENGTH <= len && len <= LUA_MAX_WORD_LENGTH) {
    int key = Lua_hash(tstr, len);
    if (key <= LUA_MAX_HASH_VALUE) {
      const char *word = wordlist[key];
      while (len--)
        if ((char)(*tstr++) != *word++) return false;
                                        return true;
  } }
  return false;
}
static bool Lua_is_eol_com_tc (const tchar *tstr, int len, int)
{
  return (len > 1 && (char)tstr[0] == '-' && (char)tstr[1] == '-'
                  &&             (len < 3 || (char)tstr[2] != '['));
}
static bool Lua_is_eol_com_c (const char *str, int len, int)
{
  return (len > 1 && str[0] == '-' && str[1] == '-'
                  &&      (len < 3 || str[2] != '['));
}
static bool Lua_is_begin_com_tc (const tchar *tstr, int len, int)
{
  return (len > 3 && (char)tstr[0] == '-' && (char)tstr[1] == '-'
                  && (char)tstr[2] == '[' && (char)tstr[3] == '[');
}
static bool Lua_is_begin_com_c (const char *str, int len, int)
{
  return (len > 1 && str[0] == '/' && str[1] == '*'
                  && str[2] == '[' && str[3] == '[');
}
static const char Lua_end_comment[] = "]]";
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#define CPP_TOTAL_KEYWORDS  68
#define CPP_MIN_WORD_LENGTH  2
#define CPP_MAX_WORD_LENGTH 16
#define CPP_MIN_HASH_VALUE   2
#define CPP_MAX_HASH_VALUE 145

inline unsigned int Cpp_hash (const tchar *tstr, unsigned int len)
{
  static unsigned char Cpp_char_values[] =
    {
      146, 146, 146, 146, 146, 146, 146, 146, 146, 146,
      146, 146, 146, 146, 146, 146, 146, 146, 146, 146,
      146, 146, 146, 146, 146, 146, 146, 146, 146, 146,
      146, 146, 146, 146, 146, 146, 146, 146, 146, 146,
      146, 146, 146, 146, 146, 146, 146, 146, 146, 146,
      146, 146, 146, 146, 146, 146, 146, 146, 146, 146,
      146, 146, 146, 146, 146, 146, 146, 146, 146, 146,
      146, 146, 146, 146, 146, 146, 146, 146, 146, 146,
      146, 146, 146, 146, 146, 146, 146, 146, 146, 146,
      146, 146, 146, 146, 146, 146, 146,  75,  50,  75,
       15,  10,  20,  55,  20,   5,  15, 146,  15,  65,
        5,   0,  40,  40,  30,  25,   5,   5,  85,  75,
       45,   0,  10,   0, 146, 146, 146, 146, 146, 146,
      146, 146, 146, 146, 146, 146, 146, 146, 146, 146,
      146, 146, 146, 146, 146, 146, 146, 146, 146, 146,
      146, 146, 146, 146, 146, 146, 146, 146, 146, 146,
      146, 146, 146, 146, 146, 146, 146, 146, 146, 146,
      146, 146, 146, 146, 146, 146, 146, 146, 146, 146,
      146, 146, 146, 146, 146, 146, 146, 146, 146, 146,
      146, 146, 146, 146, 146, 146, 146, 146, 146, 146,
      146, 146, 146, 146, 146, 146, 146, 146, 146, 146,
      146, 146, 146, 146, 146, 146, 146, 146, 146, 146,
      146, 146, 146, 146, 146, 146, 146, 146, 146, 146,
      146, 146, 146, 146, 146, 146, 146, 146, 146, 146,
      146, 146, 146, 146, 146, 146, 146, 146, 146, 146,
      146, 146, 146, 146, 146, 146, 146
    };
  unsigned int hval = len;
  if (hval != 2) hval += Cpp_char_values[(unsigned char)tstr[2]+1];
                 hval += Cpp_char_values[(unsigned char)tstr[1]  ];
  return hval;
}
static bool Cpp_in_word_set (const tchar *tstr, unsigned int len)
{
  static const char *wordlist[] =
    {
      "", "", "do", "", "long", "const", "", "", "continue", "goto",
      "const_cast", "sizeof", "dynamic_cast", "int", "auto", "", "",
      "mutable", "unsigned", "void", "endif", "return", "if", "template",
      "else", "union", "ifndef", "include", "for", "", "", "signed", "",
      "asm", "elif", "ifdef", "struct", "virtual", "register", "this",
      "while", "reinterpret_cast", "", "try", "bool", "using", "typeid",
      "typedef", "typename", "", "throw", "friend", "private", "", "",
      "break", "extern", "", "new", "", "float", "static", "", "", "",
      "short", "static_cast", "", "operator", "", "class", "define",
      "default", "volatile", "char", "", "inline", "", "", "protected", "",
      "delete", "", "", "case", "catch", "public", "wchar_t", "",
      "namespace", "", "double", "", "explicit", "enum", "", "switch",
      "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
      "", "", "", "", "", "true", "", "", "", "", "", "", "", "", "", "",
      "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "false"
    };
  if (CPP_MIN_WORD_LENGTH <= len && len <= CPP_MAX_WORD_LENGTH) {
    int key = Cpp_hash(tstr, len);
    if (key <= CPP_MAX_HASH_VALUE) {
      const char *word = wordlist[key];
      while (len--)
        if ((char)(*tstr++) != *word++) return false;
                                        return true;
  } }
  return false;
}
static bool Cpp_is_eol_com_tc (const tchar *tstr, int len, int)
{
  return (len > 1 && (char)tstr[0] == '/' && (char)tstr[1] == '/');
}
static bool Cpp_is_eol_com_c (const char *str, int len, int)
{
  return (len > 1 && str[0] == '/' && str[1] == '/');
}
static bool Cpp_is_begin_com_tc (const tchar *tstr, int len, int)
{
  return (len > 1 && (char)tstr[0] == '/' && (char)tstr[1] == '*');
}
static bool Cpp_is_begin_com_c (const char *str, int len, int)
{
  return (len > 1 && str[0] == '/' && str[1] == '*');
}
static const char Cpp_end_comment[] = "*/";
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#define PERL_TOTAL_KEYWORDS 30
#define PERL_MIN_WORD_LENGTH 2
#define PERL_MAX_WORD_LENGTH 8
#define PERL_MIN_HASH_VALUE 2
#define PERL_MAX_HASH_VALUE 38

inline unsigned int Perl_hash (const tchar *tstr, unsigned int len)
{
  static unsigned char Perl_char_values[] =
    {
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 25, 25,  0,
      10,  0, 15,  0, 39, 15, 39, 39, 15, 21,
      25,  6,  5, 25,  0,  0,  5,  0, 39, 10,
       0, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39
    };
  unsigned int hval = len;
  switch (hval) {
  default: hval += Perl_char_values[(unsigned char)tstr[2]]; // and FALLTHROUGH
  case 2:
  case 1:  hval += Perl_char_values[(unsigned char)tstr[0]];
  }
  return hval;
}
static bool Perl_in_word_set (const tchar *tstr, unsigned int len)
{
  static const char * wordlist[] =
    {
      "", "", "eq", "use", "else", "elsif", "strict", "unshift", "or", "goto",
      "until", "return", "do", "die", "redo", "undef", "", "if", "for", "last",
      "shift", "unless", "foreach", "my", "", "print", "printf", "ne", "sub",
      "next", "while", "", "require", "continue", "", "", "", "", "and"
    };
  if (PERL_MIN_WORD_LENGTH <= len && len <= PERL_MAX_WORD_LENGTH) {
    int key = Perl_hash(tstr, len);
    if (key <= PERL_MAX_HASH_VALUE) {
      const char *word = wordlist[key];
      while (len--)
        if ((char)(*tstr++) != *word++) return false;
                                        return true;
  } }
  return false;
}
static bool Perl_eol_com_tc (const tchar *tstr, int, int pos)
{
  return (char)tstr[0] == '#' && (pos == 0 || (char)tstr[-1] == ' ');
}
static bool Perl_eol_com_c (const char *str, int, int pos)
{
  return str[0] == '#' && (pos == 0 || str[-1] == ' ');
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#define PYTHON_TOTAL_KEYWORDS 32
#define PYTHON_MIN_WORD_LENGTH 2
#define PYTHON_MAX_WORD_LENGTH 8
#define PYTHON_MIN_HASH_VALUE 2
#define PYTHON_MAX_HASH_VALUE 46

inline unsigned int Python_hash (const tchar *tstr, unsigned int len)
{
  static unsigned char Python_char_values[] =
    {
      47, 47, 47, 47, 47, 47, 47, 47, 47, 47,
      47, 47, 47, 47, 47, 47, 47, 47, 47, 47,
      47, 47, 47, 47, 47, 47, 47, 47, 47, 47,
      47, 47, 47, 47, 47, 47, 47, 47, 47, 47,
      47, 47, 47, 47, 47, 47, 47, 47, 47, 47,
      47, 47, 47, 47, 47, 47, 47, 47, 47, 47,
      47, 47, 47, 47, 47, 47, 47, 47, 47, 47,
      47, 47, 47, 47, 47, 47, 47, 47, 25, 47,
      47, 47, 47, 47, 47, 47, 47, 47, 47, 47,
      47, 47, 47, 47, 47, 47, 47, 15, 10, 15,
       5,  5,  0,  0, 20,  0, 47,  5, 25,  0,
      10,  5,  0, 47,  0, 15,  0, 47, 47,  5,
      47, 15, 47, 47, 47, 47, 47, 47, 47, 47,
      47, 47, 47, 47, 47, 47, 47, 47, 47, 47,
      47, 47, 47, 47, 47, 47, 47, 47, 47, 47,
      47, 47, 47, 47, 47, 47, 47, 47, 47, 47,
      47, 47, 47, 47, 47, 47, 47, 47, 47, 47,
      47, 47, 47, 47, 47, 47, 47, 47, 47, 47,
      47, 47, 47, 47, 47, 47, 47, 47, 47, 47,
      47, 47, 47, 47, 47, 47, 47, 47, 47, 47,
      47, 47, 47, 47, 47, 47, 47, 47, 47, 47,
      47, 47, 47, 47, 47, 47, 47, 47, 47, 47,
      47, 47, 47, 47, 47, 47, 47, 47, 47, 47,
      47, 47, 47, 47, 47, 47, 47, 47, 47, 47,
      47, 47, 47, 47, 47, 47, 47, 47, 47, 47,
      47, 47, 47, 47, 47, 47
    };
  return len + Python_char_values[(unsigned char)tstr[len-1]]
             + Python_char_values[(unsigned char)tstr[0]];
}
static bool Python_in_word_set (const tchar *tstr, unsigned int len)
{
  static const char * wordlist[] =
    {
      "", "", "if", "for", "from", "print", "import", "or", "def", "elif",
      "raise", "except", "in", "not", "else", "while", "return", "is", "try",
      "pass", "break", "assert", "finally", "and", "exec", "yield", "", "",
      "continue", "with", "", "global", "as", "del", "None", "class",
      "", "", "", "", "", "", "", "", "", "", "lambda"
    };
  if (PYTHON_MIN_WORD_LENGTH <= len && len <= PYTHON_MAX_WORD_LENGTH) {
    int key = Python_hash(tstr, len);
    if (key <= PYTHON_MAX_HASH_VALUE) {
      const char *word = wordlist[key];
      while (len--)
        if ((char)(*tstr++) != *word++) return false;
                                        return true;
  } }
  return false;
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
SyntLang_t SyntLangs[] =
{
  { &no_keywords, &no_comments_tc, &no_comments_tc,
                  &no_comments_c,  &no_comments_c, empty }, // CLangNONE

  { &no_keywords, &pound_eol_com_tc, &no_comments_tc,
                  &pound_eol_com_c,  &no_comments_c, empty }, // CLangGEN

  { &Bash_in_word_set, &pound_eol_com_tc, &no_comments_tc,
                       &pound_eol_com_c,  &no_comments_c, empty }, // CLangSH

  { &Cpp_in_word_set,
    &Cpp_is_eol_com_tc, &Cpp_is_begin_com_tc,
    &Cpp_is_eol_com_c,  &Cpp_is_begin_com_c, Cpp_end_comment }, // CLangCPP

  { &Lua_in_word_set,
    &Lua_is_eol_com_tc, &Lua_is_begin_com_tc,
    &Lua_is_eol_com_c,  &Lua_is_begin_com_c, Lua_end_comment }, // CLangLUA

  { &Perl_in_word_set, &Perl_eol_com_tc, &no_comments_tc,
                       &Perl_eol_com_c,  &no_comments_c, empty }, // CLangPERL

  { &Python_in_word_set,
       &Perl_eol_com_tc, &no_comments_tc,
       &Perl_eol_com_c,  &no_comments_c, empty }, // CLangPYTH (same comments
};                                                //             as for Perl)
//-----------------------------------------------------------------------------
void printSynt(int *Synt)
{
  int len = Synt[0] & AT_CHAR;
  if (len > MAXSYNTBUF) fprintf(stderr, "0x%X", Synt[0]);
  else {
    fprintf(stderr, "[%s%d]", (Synt[0] & AT_COMMENT) ? "#" : "", len);
    for (int i = 1; i <= len; i++)
      fprintf(stderr, ",%d%c", (Synt[i] >> 8), (char)(Synt[i] & 0xFF));
} }
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
char Types[96] = "..`#x..`()..;#.#" //   ! " # $ % & ' ( ) * + , - . /
                 "0000000000;;...." // 0 1 2 3 4 5 6 7 8 9 : ; < = > ?
                 ".xxxxxxxxxxxxxxx" // @ A B C D E F G H I J K L M N O
                "xxxxxxxxxxx(\\).x" // P Q R S T U V W X Y Z [ \ ] ^ _
                 "`xxxxxxxxxxxxxxx" // ` a b c d e f g h i j k l m n o
                 "xxxxxxxxxxx(.)."; // p q r s t u v w x y z { | } ~ 
//
char matchingBrak (char brak) { return (brak < '/') ? (brak^1) : (brak^6); }
inline char Type_c (unsigned char uc)
{
  return (0x20 <= uc && uc <= 0x7E) ? Types[uc - 0x20] : '.';
}
inline char Type_tc (tchar tc)
{
  return (0x20 <= tc && tc <= 0x7E) ? Types[tc - 0x20] : '.';
}
char SyntType (tchar tc) { return Type_tc(tc); } // used by leNword() file le.c
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#define SyntMARK_WHEN_KEYWORD(endWord)          \
  if (L->in_word_set(tcp+word0, endWord-word0)) \
    for (i = word0; i < endWord; i++) tcp[i] |= AT_KEYWORD;
//-----------------------------------------------------------------------------
// Colorize the line from given text (the line assumed to be located in tcbuf),
// may change txt->thisSynts (in which case it empties the entire txt->cldstk):
//
void SyntColorize (txt *text, tchar *tcp, int& len)
{
  if (text->clang > CLangMAX)      return;
  SyntLang_t *L = &SyntLangs[text->clang];
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  int newSynts[MAXSYNTBUF], numNews = 1;    newSynts[0] = 0;
  char brakt [MAXBRAC]; int brnclo = text->prevSynts[0] & AT_CHAR;
  int  brakp [MAXBRAC]; int brx = 0; if (brnclo > MAXSYNTBUF) brnclo = 0;
  int  braclo[MAXBRAC]; int brtclo = 0;
  int i, word0 = 0, word1 = 0, badSynt = 0;
  bool in_le_mode = (Lwnd && Ly == text->txy);
  char mode = (text->prevSynts[0] & AT_COMMENT) ? 'c' : '.';
  if (mode == '.') while (word1 < len && tcharIsBlank(tcp[word1])) word1++;
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  for (int N = word1; N < len; N++) {
    if (mode == 'c') {
      while ( N < len && ((char)tcp[N]   != L->end_comment[0] ||
                          (char)tcp[N+1] != L->end_comment[1]) ) N++;
  // NOTE:
  // assuming end-of-comment sequence is always 2-chars long like '/*' for C++,
  // ']]' for Lua, not applicable to shell, Perl and Python (multi-line comment
  // of the latter are not supported since no easy way to tell open/close ones)
  // P.S. reading one character past length is Ok (there will be a space there)
  //
      for (i = word0; i <= N; i++) tcp[i] |= AT_COMMENT;
      if (N == len) break;
      N++;
      mode = '.'; continue;
    }
    tchar tc = tcp[N] & AT_CHAR;
    char typ = Type_tc(tc);
    if (typ != mode) {
           if (typ  == 'x') { word0 = N; mode = typ; continue; }
      else if (mode == 'x') {        if (typ == '0') continue;
                                     SyntMARK_WHEN_KEYWORD(N); }
      switch (typ) {
      case '\\': N++; break;
      case '#':
        if (L->is_eol_com_tc(tcp+N, len-N, N)) {
          while (N < len) tcp[N++] |= AT_COMMENT;
        }
        else if (L->is_begin_com_tc(tcp+N, len-N, N)) {
          word0 =  N;           // ^
          mode = 'c'; continue; // will process the comment at the beginning of
        }                       // next loop iteration, as begin_comment cannot
        break;                  // be less than two characters long
      case '`':
        for (i = N+1; i < len; i++) {
               if ((char)tcp[i] == '\\') i++;
          else if ((tcp[i] & AT_CHAR) == tc) goto quoted;
        }
        tcp[N]     |= (in_le_mode ? AT_MARKOK : AT_ERROR); break;
quoted: tcp[N]     |= AT_QOPEN;         //
        tcp[N = i] |= AT_QCLOSE; break; // either mark "good" quotes or bad one
      case '(':
        if (brx < MAXBRAC) { brakp[brx] = N;
                             brakt[brx] = tc; brx++; } break;
      case ')':
        if (brx) { brx--;
          if (matchingBrak(tc) != brakt[brx]) { // mismatching bracket -- show
             tcp[N]          |= AT_ERROR;       //          only the 1st error
             tcp[brakp[brx]] |= AT_ERROR; goto end_of_loop;
        } }
        else if (brnclo) { bool matched = false;
          while (brnclo) {
            if (matchingBrak(tc) == (text->prevSynts[brnclo] & 0xFF)) {
              word1 = (text->prevSynts[brnclo] >> 8);
              brnclo--;              // ^                   |
              braclo[brtclo++] =  N; // to cover this case: |if (two-line
              matched = true; break; //                     |    condition) {
            }
            else if (badSynt == 0) { badSynt = text->prevSynts[brnclo--];
                                     tcp[N] |= AT_ERROR;        continue; }
            else break;
          }
          if (!matched) tcp[N] |= AT_ERROR;
        }
        else { tcp[N] |= AT_ERROR; return; }
      }
      mode = '.';
  } }
end_of_loop:
  if (mode == 'x') SyntMARK_WHEN_KEYWORD(len);
//
// Make sure all already type-matched closing brackets are positioned correctly
// (should not be to the left of adjusted opening bracket position)
//
  if (ShowBrakts) for (i = 0; i < brtclo; i++)
                    if ((text->prevSynts[brnclo+i+1] >> 8) > braclo[i])
                      tcp[braclo[i]] |= AT_ERROR;
//
// Move Synt pair of the first (hopefully, single) mismatched bracket in front
// of the list, trying to avoid multiple error marks and provide more relevant
// info for the end-of-file unclosed brackets lineup
//
  if (badSynt) newSynts[numNews++] = badSynt;
//
// Copy not-closed brackets from prevSynt to temporary storage, add new opening
// brackets after that (using first non-space in the line as adjusted position)
// and (only in line-edit mode) colorize all opening non-matched bracket
//
  for (i = 0; i < brnclo; i++)  newSynts[numNews++] = text->prevSynts[i+1];
  for (i = 0; i < brx && numNews < MAXSYNTBUF; i++) {
    newSynts[numNews++] =  (word1 << 8) | brakt[i];
    if (in_le_mode || ShowBrakts > 1) tcp[brakp[i]] |= AT_MARKOK;
  }
// Some artificial intelligence to add an earlier indication of missing closing
// bracket - mark as error when first non-closed bracket in current line starts
// (as measured by 'word1') without indent from the last element in prevSynts,
// but not when the latter is curly bracket and the former is not:
//
//       int SyntParse(txt *text,          extern "C" {
// mark> {            ^                ok> void vipRepaint(wnd *vp,... int x0,
//         ...        not closed                                       int y1);
//       }                                 }
//
  if (ShowBrakts && brx && brnclo
                 && word1 <= (text->prevSynts[brnclo] >>   8)
                 && !(       (text->prevSynts[brnclo] & 0xFF) == '{' &&
                                                     brakt[0] != '{' ))
    tcp[brakp[0]] |= AT_ERROR;
//
// Unless in line-editing mode, check if post-line Synts has been changed, and,
// if they did, update the top element on the CL stack and force recalculation:
//
  if (!in_le_mode) {
    newSynts[0] = ((mode == 'c') ? AT_COMMENT : 0) + (numNews-1);
    int syntlen = numNews * sizeof(int);
    if (memcmp(newSynts, text->thisSynts, syntlen) != 0) {
      blkmov  (newSynts, text->thisSynts, syntlen);
      DqEmpt(text->cldstk);                           // emptying CL stack will
      DqAddB(text->cldstk, (char*)newSynts, syntlen); // force re-parse of text
      wndop(TW_DWN, text);            TxBottom(text); // below, make sure final
} } }                                                 // Synts are updated too
//-----------------------------------------------------------------------------
int SyntParse(txt *text, char *str, int len, int *out) // NOTE: 'out' points to
{                                                      //  text->this/prevSynts
  if (text->clang > CLangMAX) { *out = 0xD1; return 1; }
  SyntLang_t *L = &SyntLangs[text->clang];
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  char brakt [MAXBRAC]; int brnclo = text->prevSynts[0] & AT_CHAR;
  int i, word0 = 0, word1 = 0, badSynt = 0, haveBad = 0,  brx = 0;
  int *pout = out;            if (brnclo > MAXSYNTBUF) brnclo = 0;
  char mode = (text->prevSynts[0] & AT_COMMENT) ? 'c' : '.';
  if (mode == '.') while (word1 < len && str[word1] == ' ') word1++;
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  for (int N = word1; N < len; N++) {
    if (mode == 'c') {
      while ( N < len && (str[N]   != L->end_comment[0] ||
                          str[N+1] != L->end_comment[1]) ) N++;
      if (N == len) break;
      N++;
      mode = '.'; continue;
    }
    char c = str[N];
    switch (Type_c(c)) {
    case '\\': N++; break;
    case '#':
           if (L->is_eol_com_c  (str+N, len-N, N)) goto end_of_loop;
      else if (L->is_begin_com_c(str+N, len-N, N)) {
        word0 =  N;           // ^
        mode = 'c'; continue; // will process the comment at the beginning of
      }                       // next loop iteration, as begin_comment cannot
      break;                  // be less than two characters long
    case '`':
      for (N++; N < len; N++) {  if (str[N] == '\\') N++;
                            else if (str[N] == c ) break; }
      break;
    case '(':
      if (brx < MAXBRAC) brakt[brx++] = str[N];
      break;
    case ')': if (brx) brx--;
      else
      if      (brnclo) { bool matched = false;
        while (brnclo) {
          if (matchingBrak(c) == (text->prevSynts[brnclo] & 0xFF)) {
            brnclo--;
            matched = true; break;
          }
          else if (!haveBad) { haveBad++;
                               badSynt = text->prevSynts[brnclo--]; }
          else break;  }}
  } }
end_of_loop:
  if (brnclo+brx+haveBad > MAXSYNTBUF-1) brx = MAXSYNTBUF-brnclo-haveBad-1;
  *pout++ = ((mode == 'c') ? AT_COMMENT : 0) +  ( brnclo + brx + haveBad );
  pout = (int*)blkmov(text->prevSynts+1, pout+haveBad, sizeof(int)*brnclo);
  for (i = 0; i < brx; i++) *(pout++) = (word1 << 8) | brakt[i];
  if (haveBad) out[1] = badSynt;    return brnclo+brx+haveBad+1;
}
//-----------------------------------------------------------------------------
