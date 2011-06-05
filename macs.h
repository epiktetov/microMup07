/*------------------------------------------------------+----------------------
// МикроМир07      Mac-specific code (Objective-C)      | (c) Epi MG, 2011
//------------------------------------------------------+--------------------*/
#ifndef MACS_H_INCLUDED
#define MACS_H_INCLUDED
#ifdef Q_OS_MAC
  void macs_update_all_wspaces(void);
#else
# define macs_update_all_wspaces()
#endif
/*---------------------------------------------------------------------------*/
#endif                                                    /* MACS_H_INCLUDED */
