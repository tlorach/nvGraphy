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
#include "stdafx.h"
//
// Load
//
bool loadFRAPSms(const char *name, const char *fname, int n)
{
    float prevtime = 0.0f;
    char tmpstr[1024];
    //std::basic_ifstream<char> fs(fname);
    std::ifstream fs(fname);
    if(!fs.is_open())
	{
		if(g_pLog) g_pLog->AddMessage("no>>Failure...");
        return false;
	}
    // for size of the file so we can display a progress bar
    fs.seekg( -1, std::ios_base::end );
    int bSize = fs.tellg();
    fs.seekg( 0, std::ios_base::beg );
    g_pProgressBar->SetTitle("Loading FRAPS-ms data...");
	if(g_pLog) g_pLog->AddMessage("Loading FRAPS-ms data from %s", fname);

    strcpy(tmpstr, fname);
    char *c = strchr(tmpstr, '.');
    if(c) *c = '\0';
    c = strrchr(tmpstr, '\\');

    int disp = g_pDisplays.size();
    IWindowFolding *pWFold;
    if(disp == 0)
    {
        TLDisplay *pDisp = new TLDisplay(g_hwnd);
        pDisp->name = "FRAPS (Ms)";
        g_pDisplays.push_back(pDisp);
        //UI:
        pWFold = g_pwinHandler->CreateWindowFolding((LPCSTR)((disp+1)<<8), "FRAPS ms", g_pMainContainer);
    }
    else
    {
        pWFold = (IWindowFolding *)g_pwinHandler->Get((LPCSTR)((disp+1)<<8))->QueryInterface("IWindowFolding");
		disp--;
    }

    int graph = g_pDisplays[disp]->addGraph(c ? c+1 : tmpstr);
    //UI:
    g_pwinHandler->CreateCtrlCheck((LPCSTR)(((disp+1)<<8)+graph+1), c ? c+1 : tmpstr, pWFold)->SetChecked(true);

    fs.getline(tmpstr, 1023); //Dummy line
    if(*tmpstr == '\0')
	{
		if(g_pLog) g_pLog->AddMessage("no>>Failure...");
        return false;
	}
    do {
        int frame;
        float time;
        fs.getline(tmpstr, 1023, ',');
        frame = atoi(tmpstr);
        fs.getline(tmpstr, 1023, '\n');
        time = atof(tmpstr);
        if((!fs.eof()) && (frame >= 0))
        {
            g_pDisplays[n]->addMeasure(graph, TMeasure(time - prevtime, frame, NULL, NULL, NULL), frame);
        }
        prevtime = time;
        if((frame % 10)==0)
            g_pProgressBar->SetPercent(100*fs.tellg()/bSize);
    } while(!fs.eof());
	if(g_pLog) g_pLog->AddMessage("yes>>Done");
    return true;
}
bool loadFRAPSfps(const char *name, const char *fname)
{
    char tmpstr[1024];
    //std::basic_ifstream<char> fs(fname);
    std::ifstream fs(fname);
    if(!fs.is_open())
	{
		if(g_pLog) g_pLog->AddMessage("no>>Failure...");
        return false;
	}
    // for size of the file so we can display a progress bar
    fs.seekg( -1, std::ios_base::end );
    int bSize = fs.tellg();
    fs.seekg( 0, std::ios_base::beg );
    g_pProgressBar->SetTitle("Loading FRAPS-fps data...");
	if(g_pLog) g_pLog->AddMessage("Loading FRAPS-fps data from %s", fname);

    //strcpy(tmpstr, name);
    //char *c = strchr(tmpstr, '.');
    //if(c) *c = '\0';
    //c = strrchr(tmpstr, '\\');

    int disp = g_pDisplays.size();
    IWindowFolding *pWFold;
    // Note:Now let's have one single display for FRAPS fps reports
    // and put all the curves in the same one... better to compare.
    //g_pDisplays.push_back(new TLDisplay(g_hwnd));
    if(disp == 0)
    {
        TLDisplay *pDisp = new TLDisplay(g_hwnd);
        pDisp->name = "FRAPS (FPS)";
        g_pDisplays.push_back(pDisp);
        //UI:
        pWFold = g_pwinHandler->CreateWindowFolding((LPCSTR)((disp+1)<<8), "FRAPS fps", g_pMainContainer);
    }
    else
    {
        // Hack for now: in this case of FRAPS, always use the first display
        disp = 0;
        pWFold = (IWindowFolding *)g_pwinHandler->Get((LPCSTR)((disp+1)<<8))->QueryInterface("IWindowFolding");
    }

    int graph = g_pDisplays[disp]->addGraph(name);//c ? c+1 : tmpstr);
    //UI:
    g_pwinHandler->CreateCtrlCheck((LPCSTR)(((disp+1)<<8)+graph+1), /*c ? c+1 : tmpstr*/name, pWFold)->SetChecked(true);

    fs.getline(tmpstr, 1023); //Dummy line
    if(*tmpstr == '\0')
	{
		if(g_pLog) g_pLog->AddMessage("no>>Failure...");
        return false;
	}
    do {
        int fps;
        fs.getline(tmpstr, 1023, '\n');fps = atoi(tmpstr);
        if(!fs.eof())
        {
            g_pDisplays[disp]->addMeasure(graph, TMeasure(fps, 0, NULL, NULL, NULL));
        }
        g_pProgressBar->SetPercent(100*fs.tellg()/bSize);
    } while(!fs.eof());
	if(g_pLog) g_pLog->AddMessage("yes>>Done");
	return true;
}
