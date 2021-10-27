#include <exec/types.h>
#include <exec/execbase.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <workbench/startup.h>
#include <graphics/gfxbase.h>
#include <graphics/gfx.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/gadtools.h>
#include <utility/tagitem.h>

int main(void);

/* Startup code including workbench message support */
int _start()
{
    struct ExecBase *SysBase = *(struct ExecBase **)4;
    struct Process *p = NULL;
    struct WBStartup *wbmsg = NULL;
    int ret = 0;

    p = (struct Process *)SysBase->ThisTask;

    if (p->pr_CLI == 0)
    {
        WaitPort(&p->pr_MsgPort);
        wbmsg = (struct WBStartup *)GetMsg(&p->pr_MsgPort);
    }

    ret = main();

    if (wbmsg)
    {
        Forbid();
        ReplyMsg((struct Message *)wbmsg);
    }

    return ret;
}

struct ExecBase *       SysBase;
struct IntuitionBase *  IntuitionBase;
struct GfxBase *        GfxBase;
struct Library *        GadToolsBase;

struct TextAttr Topaz8 = { "topaz.font", 8, 0, 0, };

void GUIMain()
{
    struct TextFont *   font;
    struct Screen *     pubscreen;
    APTR                vi;

    font = OpenFont(&Topaz8);
    
    if (font != NULL)
    {
        pubscreen = LockPubScreen(NULL);

        if (pubscreen)
        {
            vi = GetVisualInfo(pubscreen, TAG_DONE);

            if (vi != NULL)
            {

                FreeVisualInfo(vi);
            }

            UnlockPubScreen(NULL, pubscreen);
        }

        CloseFont(font);
    }
}

int main()
{
    SysBase = *(struct ExecBase **)4;
    
    IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 37);
    if (IntuitionBase != NULL)
    {
        GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 37);
        if (GfxBase != NULL)
        {
            GadToolsBase = OpenLibrary("gadtools.library", 37);
            if (GadToolsBase != NULL)
            {
                GUIMain();
                CloseLibrary(GadToolsBase);
            }
            CloseLibrary((struct Library *)GfxBase);
        }
        CloseLibrary((struct Library *)IntuitionBase);
    }

    return 0;
}
