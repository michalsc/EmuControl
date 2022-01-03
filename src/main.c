#include <exec/types.h>
#include <exec/execbase.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <intuition/classes.h>
#include <workbench/startup.h>
#include <graphics/gfxbase.h>
#include <graphics/gfx.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <libraries/mui.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/gadtools.h>
#include <clib/muimaster_protos.h>
#include <clib/alib_protos.h>
#include <utility/tagitem.h>

int main(int);

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

    ret = main(wbmsg ? 1 : 0);

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
struct DosLibrary *     DOSBase;
struct Library *        MUIMasterBase;

#define APPNAME "EmuControl"

static const char version[] __attribute__((used)) = "$VER: " VERSION_STRING;

Object *app;
Object *MainWindow;

void MUIMain()
{
    app = ApplicationObject, 
            MUIA_Application_Title, (ULONG)APPNAME,
            MUIA_Application_Version, (ULONG)version,
            MUIA_Application_Copyright, (ULONG)"(C) 2022 Michal Schulz",
            MUIA_Application_Author, (ULONG)"Michal Schulz",
            MUIA_Application_Description, (ULONG)APPNAME,
            MUIA_Application_Base, (ULONG)"EMUCONTROL",

            SubWindow, MainWindow = WindowObject,
                MUIA_Window_Title, (ULONG)APPNAME,
                
                End,
            End;
    
    if (app)
    {
        DoMethod(MainWindow, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
            (ULONG)app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

        set(MainWindow, MUIA_Window_Open, TRUE);
        DoMethod(app, MUIM_Application_Execute);
        set(MainWindow, MUIA_Window_Open, FALSE);

        MUI_DisposeObject(MainWindow);
    }
}

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

int main(int fromWorkbench)
{
    SysBase = *(struct ExecBase **)4;
    
    IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 37);
    if (IntuitionBase != NULL)
    {
        GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 37);
        if (GfxBase != NULL)
        {
            MUIMasterBase = OpenLibrary("muimaster.library", 0);
            if (MUIMasterBase != NULL)
            {
                MUIMain();
                CloseLibrary(MUIMasterBase);
            }
            else
            {
                GadToolsBase = OpenLibrary("gadtools.library", 37);
                if (GadToolsBase != NULL)
                {
                    GUIMain();
                    CloseLibrary(GadToolsBase);
                }
            }
            CloseLibrary((struct Library *)GfxBase);
        }
        CloseLibrary((struct Library *)IntuitionBase);
    }

    return 0;
}
