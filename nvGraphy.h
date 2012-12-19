/*
# Copyright (c) 2012, Tristan Lorach & NVIDIA CORPORATION. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#  * Neither the name of NVIDIA CORPORATION nor the names of its
#    contributors may be used to endorse or promote products derived 
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
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
