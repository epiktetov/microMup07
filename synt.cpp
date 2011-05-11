//------------------------------------------------------+----------------------
// МикроМир07      Syntax checker / text colorizer      | (c) Epi MG, 2011
//------------------------------------------------------+----------------------
#include "mic.h"
#include "ccd.h"
#include "qfs.h"
#include "twm.h"
#include "vip.h"
extern "C" {
#include "le.h"
#include "dq.h"
}
#include "synt.h"
#include <QRegExp>
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
small SyntKnownLang (QString filename)
{
  QRegExp CppFile(".+\\.(c|cpp|cxx|h|hpp|hxx)", Qt::CaseInsensitive);
  if (CppFile.exactMatch(filename)) return 'C';
  else                              return 0;
}
//-----------------------------------------------------------------------------
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
  if (len <= CPP_MAX_WORD_LENGTH && len >= CPP_MIN_WORD_LENGTH) {
    int key = Cpp_hash(tstr, len);
    if (key <= CPP_MAX_HASH_VALUE) {
      const char *word = wordlist[key];
      while (len--)
        if ((char)(*tstr++) != *word++) return false;
                                        return true;
  } }
  return false;
}
static bool Cpp_is_eol_comment_tc (const tchar *tstr, int len)
{
  return (len > 1 && (char)tstr[0] == '/' && (char)tstr[1] == '/');
}
static bool Cpp_is_eol_comment_c (const char *str, int len)
{
  return (len > 1 && str[0] == '/' && str[1] == '/');
}
static bool Cpp_is_begin_comment_tc (const tchar *tstr, int len)
{
  return (len > 1 && (char)tstr[0] == '/' && (char)tstr[1] == '*');
}
static bool Cpp_is_begin_comment_c (const char *str, int len)
{
  return (len > 1 && str[0] == '/' && str[1] == '*');
}
static char Cpp_end_comment[] = "*/";
//-----------------------------------------------------------------------------
char Types[96] = "..`#...`()...#.#" //   ! " # $ % & ' ( ) * + , - . /
                 "0000000000......" // 0 1 2 3 4 5 6 7 8 9 : ; < = > ?
                 ".xxxxxxxxxxxxxxx" // @ A B C D E F G H I J K L M N O
                 "xxxxxxxxxxx(.).x" // P Q R S T U V W X Y Z [ \ ] ^ _
                 "`xxxxxxxxxxxxxxx" // ` a b c d e f g h i j k l m n o
                 "xxxxxxxxxxx(.)."; // p q r s t u v w x y z { | } ~ 
//
inline char Type_tc (tchar tc)
{
  return (0x20 <= tc && tc <= 0x7E) ? Types[tc - 0x20] : '.';
}
inline char Type_c (unsigned char uc)
{
  return (0x20 <= uc && uc <= 0x7E) ? Types[uc - 0x20] : '.';
}
char matchingBrak (char brak) { return (brak < '/') ? (brak^1) : (brak^6); }
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#define SyntMARK_WHEN_KEYWORD(endWord)           \
  if (Cpp_in_word_set(tcp+word0, endWord-word0)) \
    for (i = word0; i < endWord; i++) tcp[i] |= AT_KEYWORD;
//
// Colorize the line from given text (the line assumed to be located in tcbuf),
// may change txt->thisSynts (in which case it empties the entire txt->cldstk):
//
void SyntColorize (txt *text, tchar *tcp, int len)
{
  int newSynts[MAXSYNTBUF], numNews = 1; newSynts[0] = 0;
  char brakt [MAXTXRM]; int brx = 0;
  int  brakp [MAXTXRM]; int brnclo = text->prevSynts[0] & (~AT_COMMENT);
  int  braclo[MAXTXRM]; int brtclo = 0;
  int i, word0 = 0, word1 = 0;
  bool in_le_mode = (Lwnd && Ly == text->txy);
  char mode = '.';
  if (text->prevSynts[0] & AT_COMMENT) mode = 'c';
  else while (word1 < len && tcharIsBlank(tcp[word1])) word1++;
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  for (int N = word1; N < len; N++) {
    if (mode == 'c') {
      while ( N < len && ((char)tcp[N]   != Cpp_end_comment[0] ||
                          (char)tcp[N+1] != Cpp_end_comment[1]) ) N++;
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
      case '#':
        if (Cpp_is_eol_comment_tc(tcp+N, len-N)) {
          while (N < len) tcp[N++] |= AT_COMMENT;
        }
        else if (Cpp_is_begin_comment_tc(tcp+N, len-N)) {
          word0 =  N;           // ^
          mode = 'c'; continue; // will process the comment at the beginning of
        }                       // next loop iteration, as begin_comment cannot
        break;                  // be less than two characters long
      case '`':
        for (i = N, tcp[i++] |= AT_QOPEN; i < len; i++) {
               if ((char)tcp[i] == '\\') i++;
          else if ((tcp[i] & AT_CHAR) == tc) goto quoted;
        }
        tcp[N]     |= (in_le_mode ? AT_MARKOK : AT_ERROR);  return;
quoted: tcp[N = i] |= AT_QCLOSE;
        break;
      case '(':
        if (brx < MAXTXRM) { brakp[brx] = N;
                             brakt[brx] = tc; brx++; } break;
      case ')':
        if (brx) { brx--;
          if (matchingBrak(tc) != brakt[brx]) { // mismatching bracket, makes
             tcp[N]          |= AT_ERROR;       // no sense to continue parse
             tcp[brakp[brx]] |= AT_ERROR;  return;
        } }
        else if (brnclo && (text->prevSynts[brnclo] & 0xFF) == tc) {
           word1 = (text->prevSynts[brnclo] >> 8);
           brnclo--;             // ^
           braclo[brtclo++] = N; // close prev: verify type now, but pos later
        }
        else { tcp[N] |= AT_ERROR; return; }
      }
      mode = '.';
  } }
  if (mode == 'x') SyntMARK_WHEN_KEYWORD(len);
//
// Make sure all (already type-matched) closing brackets are positioned correct
// (should not be to the left of adjusted opening bracket position - see below)
//
  for (i = 0; i < brtclo; i++)
    if ((text->prevSynts[brnclo+i+1] >> 8) > braclo[i])
      tcp[braclo[i]] |= AT_ERROR;
//
// Copy not-closed brackets from prevSynt to temporary storage, add new opening
// brackets after that (using first non-space in the line as adjusted position)
// and (only in line-edit mode) colorize all opening non-matched bracket
//
  for (i = 0; i < brnclo; i++)  newSynts[numNews++] = text->prevSynts[i+1];
  for (i = 0; i < brx && numNews < MAXSYNTBUF; i++) {
    newSynts[numNews++] = (word1 << 8) | matchingBrak(brakt[i]);
    if (in_le_mode) tcp[brakp[i]] |= AT_MARKOK;
  }
  if (!in_le_mode) {
    newSynts[0] = ((mode == 'c') ? AT_COMMENT : 0) + (numNews-1);
    len = numNews * sizeof(int);
    if (memcmp(newSynts, text->thisSynts, len) != 0) {
      blkmov(newSynts, text->thisSynts, len);
      DqEmpt(text->cldstk);
      DqAddB(text->cldstk, (char*)newSynts, len); wndop(TW_DWN, text);
} } }
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int SyntParse(txt *text, char *str, int len, int *out)
{
  int brnclo = text->prevSynts[0] & (~AT_COMMENT);
  blkmov(text->prevSynts+1, out+1, sizeof(int)*brnclo);
  char brakt [MAXTXRM];                    int brx = 0;
  int i, word0 = 0, word1 = 0;
  char mode = '.';
  if (text->prevSynts[0] & AT_COMMENT) mode = 'c';
  else while (word1 < len && str[word1] == ' ') word1++;
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  for (int N = word1; N < len; N++) {
    if (mode == 'c') {
      while ( N < len && (str[N]   != Cpp_end_comment[0] ||
                          str[N+1] != Cpp_end_comment[1]) ) N++;
      if (N == len) break;
      N++;
      mode = '.'; continue;
    }
    char c = str[N];
    switch (Type_c(c)) {
    case '#':
           if (Cpp_is_eol_comment_c  (str+N, len-N)) goto end_of_loop;
      else if (Cpp_is_begin_comment_c(str+N, len-N)) {
        word0 =  N;           // ^
        mode = 'c'; continue; // will process the comment at the beginning of
      }                       // next loop iteration, as begin_comment cannot
      break;                  // be less than two characters long
    case '`':
      for (N++; N < len; N++) {  if (str[N] == '\\') N++;
                            else if (str[N] == c ) break; }
      break;
    case '(':
      if (brx < MAXTXRM) brakt[brx++] = str[N];
      break;
    case ')': if (brx) brx--;
         else if (brnclo) brnclo--;
  } }
end_of_loop:
  if (brnclo+brx > MAXSYNTBUF-1) brx = MAXSYNTBUF-brnclo-1;
  *out = ((mode == 'c') ? AT_COMMENT : 0) + (brnclo + brx);
  for (i = 0, out += brnclo; i < brx; i++)
    *(++out) = (word1 << 8) | matchingBrak(brakt[i]);   return brnclo+brx+1;
}
//-----------------------------------------------------------------------------
