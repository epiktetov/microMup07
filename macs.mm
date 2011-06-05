/*------------------------------------------------------+----------------------
// МикроМир07      Mac-specific code (Objective-C)      | (c) Epi MG, 2011
//------------------------------------------------------+--------------------*/
#include <Cocoa/Cocoa.h>
#include "mim.h"
#include "twm.h"
#include "vip.h"
#include "macs.h"

void macs_update_all_wspaces(void)
{
  id pool = [[NSAutoreleasePool alloc] init];
  for (wnd *vp = windows; vp; vp = vp->wdnext) {
    NSView *view = (NSView*)( vp->sctw->mf->winId() );
    NSWindow *win = [view window];
    if ([win respondsToSelector:@selector(isOnActiveSpace)])
                        vp->wspace = [win isOnActiveSpace];
    else break; //
  }             // it is VERY unlikely that only some windows can respond to
  [pool drain]; // isOnActiveSpace (which was introduced in 10.6 Snow Leopard)
}
//-----------------------------------------------------------------------------
