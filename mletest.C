//----------------------------------------------------------------------------
//
// MLE sample - words and music by Dan Lee
//
//----------------------------------------------------------------------------
//
// Yet another sample program, this one plays with a Multiline Entry Field.
//
// Actions -
//
//    SEARCH: Searches for a string (limited in this sample to 10 chars) in
//       in the MLE.  When the first menu item is selected, a simple dialog
//       box is called to get the search string.  Then the MLE_SEARCH struct
//       is initialized with the string, and then sent to the MLE with the
//       message MLM_SEARCH.  The MLE responds by hilighting the string (if
//       it finds it).  The way the MLE_SEARCH structure is initialized,
//       searches always start from the beginning of the MLE.
//
//    FONTS: Uses WinFontDlg to get a new font, then sets the MLE font by
//       sending the MLM_SETFONT message.  The FONTDLG struct is kept globally
//       and initialized in the WM_CREATE step rather than each time the menu
//       item is selected.  That way changes made to the struct by WinFontDlg
//       are still current the next time the ChangeFont menu item is selected.
//
//    EXPORT: Sets up and uses the MLM_EXPORT message to put the MLE text into
//       a buffer.  I put the MLE format stuff here (rather than right after
//       the MLE is created) to emphasize how critical the text format is in
//       the MLE.  For example, if you use the message MLM_QUERYTEXTLENGTH
//       rather than MLM_QUERYFORMATTEXTLENGTH, the number you get back may
//       NOT reflect the cr-lf chars correctly.  This has bitten a lot of
//       developers trying to use that number to determine how much space to
//       allocate for the export buffer.
//
//       Once the buffer was successfully exported, I put it into a message
//       box to display it (I know - lazy, lazy) rather than increasing the
//       complexity of this sample by putting it into a container or drawing
//       it onto the client space.  I may add a WinFileDlg call (later!) to
//       write the buffer to disk.
//
//----------------------------------------------------------------------------
//
// If I've made any horrible errors in here, or if you find bugs or have
// suggestions, feel free to contact me at 72360,3611.
//
//----------------------------------------------------------------------------

#define  INCL_WINFRAMEMGR
#define  INCL_WINMLE
#define  INCL_WINPOINTERS
#define  INCL_WINSTDDLGS
#define  INCL_WINWINDOWMGR

#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mletest.h"


//-------Function prototypes--------------------------------------------------
INT main( VOID );
MRESULT EXPENTRY ClientProc( HWND, ULONG, MPARAM, MPARAM );
MRESULT EXPENTRY GetSearchString( HWND, ULONG, MPARAM, MPARAM );
VOID InitializeFontDialog( VOID );
//----------------------------------------------------------------------------


//-------Global variables-----------------------------------------------------
FONTDLG fd;
CHAR    szCurrentFont[ FACESIZE ];
CHAR    szClass[] = "MLEClass";
HAB     hab;
HWND    hwndFrame, hwndClient, hwndMLE;
//----------------------------------------------------------------------------


INT main( VOID )
{
   HMQ   hmq;
   QMSG  qmsg;
   ULONG flFrame = FCF_TASKLIST | FCF_TITLEBAR   | FCF_SYSMENU |
                   FCF_MINMAX   | FCF_SIZEBORDER | FCF_MENU    |
                   FCF_SHELLPOSITION;

   hab = WinInitialize( 0 );
   hmq = WinCreateMsgQueue( hab, 0 );

   WinRegisterClass( hab, szClass, ClientProc, CS_SIZEREDRAW, 0 );

   hwndFrame = WinCreateStdWindow( HWND_DESKTOP,
                                   WS_VISIBLE,
                                   &flFrame,
                                   szClass,
                                   "MLE Starter Code",
                                   0,
                                   NULLHANDLE,
                                   ID_RESOURCES,
                                   &hwndClient );

   while( WinGetMsg( hab, &qmsg, (HWND) NULL, 0, 0 ))
       WinDispatchMsg( hab, &qmsg );

   WinDestroyWindow( hwndFrame );
   WinDestroyMsgQueue( hmq );
   WinTerminate( hab );

   return 0;
}

MRESULT EXPENTRY ClientProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
   RECTL rcl;
   PCREATESTRUCT pCreateStruct;
   SHORT x, y;

   switch( msg )
   {
      case WM_CREATE:

         //-------------------------------------------------------------------
         // make an MLE
         //-------------------------------------------------------------------
         hwndMLE =
            WinCreateWindow(
                        hwnd,
                        WC_MLE,
                        (PSZ) NULL,
                        MLS_BORDER | MLS_WORDWRAP | MLS_VSCROLL |
                           WS_VISIBLE | WS_TABSTOP,
                        0, 0, 0, 0,      //we'll size this in WM_SIZE
                        hwnd,
                        HWND_TOP,
                        ID_MLE,
                        (PVOID) NULL,
                        (PVOID) NULL );

         //-------------------------------------------------------------------
         //set up font dialog now instead of each time it's called...
         //  ... that way changes made to it by user will stay current
         //-------------------------------------------------------------------
         InitializeFontDialog();

         WinSetFocus( HWND_DESKTOP, hwndMLE );

         return( (MRESULT) 0 );

      case WM_SIZE:
         x = (SHORT) SHORT1FROMMP( mp2 );
         y = (SHORT) SHORT2FROMMP( mp2 );

         //-------------------------------------------------------------------
         // The MLE will cover the entire client area
         //-------------------------------------------------------------------
         WinSetWindowPos( hwndMLE, HWND_TOP,
                          0, 0, x, y,
                          SWP_SIZE | SWP_MOVE | SWP_SHOW );

         break;

      case WM_COMMAND:
         switch( SHORT1FROMMP( mp1 ))
         {
            //----------------------------------------------------------------
            // Search for string
            //----------------------------------------------------------------
            case IDM_SEARCH:
            {
               MLE_SEARCHDATA mlesd;
               CHAR szSearchString[ 10 ];
               BOOL bSearch;

               bSearch = WinDlgBox( HWND_DESKTOP, hwnd,
                                    GetSearchString,
                                    (HMODULE) NULL,
                                    IDDLG_GETSRCHSTRING,
                                    &szSearchString );

               if( bSearch )
               {
                  mlesd.pchFind = szSearchString;
                  mlesd.pchReplace = NULL;         // No find and replace...
                  mlesd.cchFind = 0;               // ...options in this...
                  mlesd.cchReplace = 0;            // ...brain-dead sample.
                  mlesd.iptStart = 0;              // start at beginning
                  mlesd.iptStop = 0xffffffff;      // search entire MLE

                  WinSendMsg( hwndMLE, MLM_SEARCH,
                              MPFROMLONG( MLFSEARCH_SELECTMATCH ),
                              &mlesd );

               }

               break;
            }

            //----------------------------------------------------------------
            // Change font
            //----------------------------------------------------------------
            case IDM_CHANGEFONT:
            {
               fd.hpsScreen = WinGetPS( hwnd );
               WinFontDlg( HWND_DESKTOP, hwnd, &fd );
               WinReleasePS( fd.hpsScreen );

               WinSendMsg( hwndMLE, MLM_SETFONT, &fd.fAttrs, NULL );

               break;
            }

            //----------------------------------------------------------------
            // Export to buffer
            //----------------------------------------------------------------
            case IDM_EXPORTTEXT:
            {
               ULONG ulLenMLE, ulOffsetBegin=0;
               PVOID pvBuffer;

               WinSendMsg( hwndMLE, MLM_FORMAT, MLFIE_CFTEXT, NULL );

               ulLenMLE = (ULONG) WinSendMsg( hwndMLE,
                                              MLM_QUERYFORMATTEXTLENGTH,
                                              0, (MPARAM) 0xffffffff );

               if( ulLenMLE > 0 )
                  DosAllocMem( &pvBuffer, ulLenMLE, PAG_READ | PAG_WRITE | PAG_COMMIT );

               WinSendMsg( hwndMLE, MLM_SETIMPORTEXPORT,
                           MPFROMP( (PBYTE) pvBuffer ),
                           MPFROMLONG( ulLenMLE ));

               WinSendMsg( hwndMLE, MLM_EXPORT,
                           MPFROMP( &ulOffsetBegin ), MPFROMLONG( &ulLenMLE ));

               WinMessageBox( HWND_DESKTOP, HWND_DESKTOP,
                              (PSZ) pvBuffer,
                              "First few lines of export buffer:",
                              0L, MB_OK );

               break;
            }

            //----------------------------------------------------------------
            // Good-bye
            //----------------------------------------------------------------
            case IDM_EXIT:
               WinSendMsg( hwnd, WM_CLOSE, NULL, NULL );
               break;

         }

         break;

   }
   return WinDefWindowProc( hwnd, msg, mp1, mp2 );
}


//---------------------------------------------------------------------------
// Initialize the fontdlg structure for WinFontDlg
//---------------------------------------------------------------------------
VOID InitializeFontDialog( VOID )
{
   memset( &fd, 0, sizeof( FONTDLG ));

   szCurrentFont[0] = '\0';
   fd.cbSize = sizeof( FONTDLG );

   fd.fl = FNTS_CENTER | FNTS_INITFROMFATTRS;

   strcpy( fd.fAttrs.szFacename, "System Proportional" );

   fd.fAttrs.fsType = 0;
   fd.fAttrs.usRecordLength = sizeof( FATTRS );
   fd.fAttrs.fsSelection = 0;
   fd.fAttrs.lMatch = 0;
   fd.fAttrs.idRegistry = 0;
   fd.fAttrs.usCodePage = 850;
   fd.fAttrs.lMaxBaselineExt = 13;
   fd.fAttrs.lAveCharWidth = 8;
   fd.fAttrs.fsFontUse = FATTR_FONTUSE_NOMIX;

   fd.fl = FNTS_CENTER | FNTS_INITFROMFATTRS;
   fd.pszFamilyname = szCurrentFont;
   fd.usFamilyBufLen = FACESIZE;
   fd.clrFore = CLR_NEUTRAL;
   fd.clrBack = CLR_DEFAULT;

   return;
}


//---------------------------------------------------------------------------
// Dialog box to get search string
//---------------------------------------------------------------------------
MRESULT EXPENTRY GetSearchString( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
   static PSZ pszSearch;

   switch (msg)
   {
      case  WM_INITDLG :

         //------------------------------------------------------------------
         // Get search string buffer passed in pCreateParams field
         //------------------------------------------------------------------
         pszSearch = (PSZ) PVOIDFROMMP( mp2 );

         //------------------------------------------------------------------
         // 10 character search string
         //------------------------------------------------------------------
         WinSendDlgItemMsg( hwnd, IDD_SEARCHSTRING, EM_SETTEXTLIMIT,
                            MPFROM2SHORT( 11, 0 ),       //+1 for terminator
                            NULL );

         WinSetFocus( HWND_DESKTOP,
                      WinWindowFromID( hwnd, IDD_SEARCHSTRING ));

         return (MPARAM) TRUE;

      case WM_COMMAND :

         switch( SHORT1FROMMP( mp1 ))
         {
            //---------------------------------------------------------------
            // Okay - dialog box returns TRUE
            //---------------------------------------------------------------
            case IDD_BTN_OK:

               WinQueryDlgItemText( hwnd, IDD_SEARCHSTRING,
                                    11,
                                    pszSearch );

               WinDismissDlg( hwnd, TRUE );
               return;

            //---------------------------------------------------------------
            // Cancel - dialog box returns FALSE
            //---------------------------------------------------------------
            case IDD_BTN_CANCEL :

               WinDismissDlg( hwnd, FALSE );
               return;
         }

   }
   return  WinDefDlgProc(hwnd, msg, mp1, mp2);
}


