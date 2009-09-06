/*------------------------------------------------------+----------------------
// МикроМир07  Command Codes Definition / transcoding   | (c) Epi MG, 2006-2007
//------------------------------------------------------+--------------------*/
#ifdef MAKE_TRANS_TABLE
# define k_BEGIN        static micom trans1[] = {
# define k_(e,v,k,a)      { e, k, a },
# define a_(e,  k,a)      { e, k, a },
# define k_END_OF_TABLE };
#else
# ifdef CCD_H_INCLUDED
#  define k_BEGIN
#  define k_(e,v,k,a)
#  define a_(e,  k,a)
#  define k_END_OF_TABLE
# else
#  define k_BEGIN        enum micom_enum {
#  define k_(e,v,k,a)      e = v,
#  define a_(e,  k,a)
#  define k_END_OF_TABLE };
# endif
#endif
#include "micro.keys"
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
#undef k_BEGIN
#undef k_
#undef a_
#undef k_END_OF_TABLE
/*---------------------------------------------------------------------------*/
#ifndef CCD_H_INCLUDED
#define CCD_H_INCLUDED
struct micom {
  enum micom_enum ev; /* keyboard event                    */
  int kcode;          /* key code as reported by Qt        */
  int attr;           /* attributes KxBLK / KxTMP / KxSEL  */
};
struct micom *key2mimCmd(int qtKode); /* NIL if not a valid microMir command */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
extern int KbCode;                    /* Запрос клавиатуры: код запроса (ev) */
extern int KbCount;                   /*  повторитель, если не вводился то 1 */
extern int KbRadix;                   /*  основание,   если не вводился то 0 */
typedef struct {
  enum micom_enum mi_ev;  /* код команды */
  void  (*cfunc)(void);   /* подпрограмма, выполняющая команду */
  int attr;               /* атрибуты команды (CA_xxx below)   */
} comdesc;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
#define CA_BLOCK   001 /* block command (работает по другому если есть блок) */
#define CA_LEARG   002 /* работает иначе внутри LenterARG                    */
#define CA_CHANGE  004 /* изменяет состояние текста/строки                   */
#define CA_EXT     010 /* добавление строки текста в конец                   */
#define CA_RPT     020 /* сама обрабатывает аргумент (в т.ч. не повторяема)  */
#define CA_NEND   0100 /* указатель не в конце  строки/текста                */
#define CA_NBEG   0200 /* указатель не в начале строки/текста                */
#define CA_LCUT   0400 /* работает с текстом запомненных строк               */
                       /* Keycode attributes:                                */
#define KxTS 0x1000000 /*  move starts/extends "temporary" selection         */
#define KxBLK 0x400000 /*  keeps "permanent" selection                       */
#define KxTMP 0x800000 /*  keeps "temporary" selection                       */
#define KxSEL 0xc00000 /*  keeps any selection                               */
/*---------------------------------------------------------------------------*/
#endif                                                     /* CCD_H_INCLUDED */
