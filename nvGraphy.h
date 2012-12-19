#pragma once

#include "resource.h"
#include "display.h"
#include "ISvcUI.h"

extern std::vector<TLDisplay *> g_pDisplays;
extern std::vector<int> g_apiCall; // give the apic benchmark call number from nvPerfHUD drawcall #

extern HWND g_hwnd;

extern std::string g_pathAnnotations;

extern ISvcFactory         *g_pFact;
extern IWindowHandler      *g_pwinHandler;
extern IControlString      *g_pCommentEditBox;
extern IWindowConsole      *g_pCommentConsole;
extern IControlString      *g_pPixelShaderEditBox;
extern IWindowContainer    *g_pMainContainer;
extern IProgressBar        *g_pProgressBar;
extern IWindowConsole      *g_pConsole;
extern IWindowConsole      *g_pConsolePShader;
extern IControlCombo       *g_pPredefSearch;
extern IWindowLog		   *g_pLog;

//extern void showPixelShader(LPCSTR name);

//
// Load functions
//
extern bool loadGeneric(const char *name, const char *fname);
extern bool loadFRAPSms(const char *name, const char *fname, int n=0);
extern bool loadFRAPSfps(const char *name, const char *fname);
extern bool loadD3DAPICallsCSVFile(const char *name, const char *fname, const std::vector<std::string> &keywordTable, bool bDiffGraph=false, bool bCollapse=true);
extern bool loadOGLAPICallsCSVFile(const char *name, const char *fname, const std::vector<std::string> &keywordTable, bool bDiffGraph=false, bool bCollapse=true);
extern bool loadPerfHUDReport(const char *name, const char *fname);
extern void loadPHAnnotations(const char * fname, int d = 0, int g = 0);
