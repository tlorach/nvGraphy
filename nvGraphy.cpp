// nvGraphy.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "nvGraphy.h"
#include "ISvcUI.h"

ISvcFactory         *g_pFact = NULL;
IWindowHandler      *g_pwinHandler = NULL;
IControlString      *g_pCommentEditBox = NULL;
IWindowConsole      *g_pCommentConsole = NULL;
IControlString      *g_pPixelShaderEditBox = NULL;
IWindowContainer    *g_pMainContainer = NULL;
IProgressBar        *g_pProgressBar = NULL;
IWindowConsole      *g_pConsole = NULL;
IWindowConsole      *g_pConsolePShader = NULL;
IWindowLog			*g_pLog = NULL;
IControlCombo       *g_pPredefSearch = NULL;
HWND g_hwnd = NULL;
std::vector<TLDisplay *> g_pDisplays;
std::vector<int> g_apiCall;

int g_StartMouse[2]; // for dragging
int g_curMouse[2];
int g_displaySlectedFirst = -1; //Hacky... sorry
std::string g_pathAnnotations;
bool g_bTightGraphs = true;
bool g_bAutoClearConsole = true;
bool g_bRegExp = false;
bool g_bTagSearch = false;
bool g_bClearTagSearch = false;

#define MAX_LOADSTRING 100

HINSTANCE g_hInst;								// current instance
TCHAR g_szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR g_szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
HWND				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
void                OnSize(HWND hwnd, int w, int h);
bool                saveComments();
void				ComputeDeltaValues();
void				DeleteDeltaGraphs();

void searchString(LPCSTR str, bool bRegExp=false, bool bTagSearch=false, bool bClearTag=false)
{
    if(*str == '\0')
        return;
    for(int i=0; i<(int)g_pDisplays.size(); i++)
    {
        g_pDisplays[i]->searchResults.clear();
    }
    for(int i=0; i<(int)g_pDisplays.size(); i++)
    {
		if(bTagSearch)
	        g_pDisplays[i]->searchAndTag(str, bClearTag);// bRegExp);
		else
	        g_pDisplays[i]->searchHighlight(str);// bRegExp);
        g_pDisplays[i]->update(g_hwnd);
    }
}

void adjustDisplayRatios()
{
    // adjust the ratios...
    int n =0;
    for(int i=0; i<(int)g_pDisplays.size(); i++)
        if(g_pDisplays[i]->percentH > 0) 
            n++;
    for(int i=0; i<(int)g_pDisplays.size(); i++)
        if(g_pDisplays[i]->percentH > 0) 
            g_pDisplays[i]->percentH = 100/n; //100%/N for main displays
	// trigger the update
    RECT r;
    GetClientRect(g_hwnd, &r);
    r.right -= r.left;
    r.bottom -= r.top;
    OnSize(g_hwnd, r.right, r.bottom);
}

class CEventsWnd : public IEventsWnd
{
    void ToolbarChanged(IControlToolbar *pWin, int selecteditem, int prev)
	{
        if(!strcmp(pWin->GetID(), "DELTATB"))
        {
			switch(selecteditem)
			{
			case 0: 
				ComputeDeltaValues();
				break;
			case 1: 
				DeleteDeltaGraphs();
				break;
			}
        }
	}
	// A hacky way to make Folding container match with Display (group of graphs)
	// When one is closed (unchecked), we try to find a Display and close it, too
    void WindowContainerChanged(IWindow *pWin)
    {
        IWindowFolding *pWFold = (IWindowFolding *)pWin->QueryInterface("IWindowFolding");
		// are we in a Folding container ?
		if(pWFold)
        {
            LPCSTR id = pWFold->GetID();
            if(HIWORD(id))
                return;
			// find back the id of the related Display
            int disp = (((int)id)>>8) - 1;
			if(disp >= (int)g_pDisplays.size())
			{
				if(g_pLog) g_pLog->AddMessage("Error>>bad reference to display from the UI...");
				return;
			}
			// Show it or hide it : percentH set to 0 will hide it...
            g_pDisplays[disp]->percentH = pWFold->isUnFolded() ? 1 : 0;

            adjustDisplayRatios();
        }
    }
    void StringChanged(IControlString *pWin)
    {
        int g,f;
        if(!strcmp(pWin->GetID(), "REMSTR"))
        {
		  LPCSTR cstr = pWin->GetString();
          if(g_displaySlectedFirst >=0)
          {
            if(g_pDisplays[g_displaySlectedFirst]->getSelected(&g,&f))
            {
                g_pDisplays[g_displaySlectedFirst]->setCommentForMeasure(g,f, cstr, NULL);
            }
            g_pDisplays[g_displaySlectedFirst]->addAnnotationsFromSelection(cstr, NULL);
            g_pDisplays[g_displaySlectedFirst]->update(g_hwnd);
          }
        }
        else if(!strcmp(pWin->GetID(), "SEARCH"))
        {
            searchString(pWin->GetString(), g_bRegExp, g_bTagSearch, g_bClearTagSearch);
        }
    }
    void ConsoleChanged(IWindowConsole *pWin, LPCSTR newstring, LPCSTR prev) 
	{
        int g,f;
        if(!strcmp(pWin->GetID(), "REMCONSOLE"))
        {
          if(g_displaySlectedFirst >=0)
          {
            if(g_pDisplays[g_displaySlectedFirst]->getSelected(&g,&f))
            {
                g_pDisplays[g_displaySlectedFirst]->setCommentForMeasure(g,f, NULL, "TODO\nTODO"/*pWin->GetString()*/);
            }
            g_pDisplays[g_displaySlectedFirst]->addAnnotationsFromSelection(NULL, "TODO\nTODO"/*pWin->GetString()*/);
            g_pDisplays[g_displaySlectedFirst]->update(g_hwnd);
          }
		}
	};
    void Button(IWindow *pWin, int pressed) 
    {
        if(!strcmp(pWin->GetID(), "DELTA"))
        {
            ComputeDeltaValues();
        }
        if(!strcmp(pWin->GetID(), "DELDELTA"))
        {
            DeleteDeltaGraphs();
        }
        if(!strcmp(pWin->GetID(), "SAVE"))
        {
            saveComments();
        }
        if(!strcmp(pWin->GetID(), "CLEAR"))
        {
            for(int i=0; i<(int)g_pDisplays.size(); i++)
            {
                g_pDisplays[i]->searchHighlight(NULL);
                g_pDisplays[i]->update(g_hwnd);
            }
        }
        //else if(!strcmp(pWin->GetID(), "SHOWSHD"))
        //{
        //    showPixelShader(g_pPixelShaderEditBox->GetString());
        //}
        else if(!strcmp(pWin->GetID(), "SHOWRT")) // Highlight Render targets and textures related
        {
			for(int i=0; i<(int)g_pDisplays.size(); i++)
			{
				g_pDisplays[i]->searchResults.clear();
			}
			for(int i=0; i<(int)g_pDisplays.size();i++)
			{
				int graph, frame;
				TMeasure *pM = g_pDisplays[i]->getSelected(&graph, &frame);
				if(!pM)
					continue;
				//highlightCurrentRT(frame);
			}
			for(int i=0; i<(int)g_pDisplays.size(); i++)
			{
				g_pDisplays[i]->update(g_hwnd);
			}
        }
    }
	//
	// Do a hacky thing so that we show/hide related graphs by finding them from the Checkbox IDs
	//
    void CheckBoxChanged(IControlScalar *pWin, bool &value, bool prev)
    {
        if(!HIWORD(pWin->GetID()))
        {
            g_pDisplays[0]->DrawScene();
            int g = ((int)pWin->GetID() & 0xFF)-1;
            int d = ((int)pWin->GetID() >> 8)-1;
			if(d >= (int)g_pDisplays.size())
				return;
			if(g >= (int)g_pDisplays[d]->Graphs.size())
				return;
			// the Control (pWin) corresponds to the graph
            g_pDisplays[d]->Graphs[g].valid = value;
            g_pDisplays[d]->update(g_hwnd);
        }
    }
    void ComboSelectionChanged(IControlCombo *pWin, unsigned int selectedidx)
    {
        int id = pWin->GetSelectedData();
        IControlString *pStr = g_pwinHandler->GetString("SEARCH");
        switch(id)
        {
        case 0:
            g_bRegExp = false;
            g_pwinHandler->VariableFlush(&g_bRegExp);
            pStr->SetString("depthCompare=1");
            //searchString("depthCompare=1", false);
            break;
        case 1:
            g_bRegExp = true;
            g_pwinHandler->VariableFlush(&g_bRegExp);
            pStr->SetString("depth\\{.+isTex=1");       //BUGGED
            //searchString("depth\\{.+isTex=1", true);
            break;
        // ADD MORE PREDEFINED SEARCH HERE
        }
    }
};
CEventsWnd myEvents;

//
// Load comments
//
void loadComments(const char * fname)
{
    char tmpstr[1024];

/*    // for size of the file so we can display a progress bar
    fs.seekg( -1, std::ios_base::end );
    int bSize = fs.tellg();
    fs.seekg( 0, std::ios_base::beg );
    g_pProgressBar->SetTitle("Loading comments...");
*/
    for(int d=0; d<(int)g_pDisplays.size(); d++)
        for(int g=0; g<(int)g_pDisplays[d]->Graphs.size(); g++)
        {
            std::string fullname;
            fullname = g_pathAnnotations;
            fullname += "annotations_";
            fullname += g_pDisplays[d]->Graphs[g].name;
            fullname += ".txt";
            std::ifstream fs(fullname.c_str());
FILE *ffffk = fopen(fullname.c_str(), "r");
if(ffffk) fclose(ffffk);
if(!ffffk)
    continue;
            if(!fs.is_open()) //Why the heck this F... fucntion doesn't work, hey ? 
                continue;
            if(fs.eof())
                continue;
            do {
                int frame, x1, x2;
                bool valid;
                std::string userComment;
                std::string userComment2;

                fs.getline(tmpstr, 1023, ','); 
                if(!strcmp(tmpstr, "range"))
                {
                    fs.getline(tmpstr, 1023, ','); x1 = atoi(tmpstr);
                    fs.getline(tmpstr, 1023, ','); x2 = atoi(tmpstr);
                    fs.getline(tmpstr, 1023, '\n'); userComment = tmpstr;
					int i = userComment.find_first_of(',');
					if(i>0)
					{
						userComment2 = userComment.substr(i+1,userComment.length());
						userComment2 = Pattern::compile("(\\\\n)")->replace(userComment2, "\n");
						userComment = userComment.substr(0,i);
					}
                    g_pDisplays[d]->addAnnotationsFromSelection(userComment.c_str(), userComment2.c_str(), x1, x2);
                }
                else if(!strcmp(tmpstr, "frame"))
                {
                    fs.getline(tmpstr, 1023, ','); frame = atoi(tmpstr);
                    fs.getline(tmpstr, 1023, ','); valid = atoi(tmpstr) ? true :  false;
                    fs.getline(tmpstr, 1023, '\n'); userComment = tmpstr;
					int i = userComment.find_first_of(',');
					if(i>0)
					{
						userComment2 = userComment.substr(i+1,userComment.length());
						userComment2 = Pattern::compile("(\\\\n)")->replace(userComment2, "\n");
						userComment = userComment.substr(0,i);
					}
                    if(frame >= (int)g_pDisplays[d]->Graphs[g].Measures.size())
                        continue;
                    g_pDisplays[d]->Graphs[g].Measures[frame].valid = valid;
                    if((!userComment.empty())||(!userComment2.empty()))
                        g_pDisplays[d]->setCommentForMeasure(g, frame, userComment.c_str(), userComment2.c_str());
                }
                //g_pProgressBar->SetPercent(100*fs.tellg()/bSize);
            } while(!fs.eof());
        }
}

/*------------------------------------------------------------------------------------------------
Load the files
  ------------------------------------------------------------------------------------------------*/
bool loadFile(LPCSTR name, LPCSTR fname, const std::vector<std::string> &keywordTable, bool bCollapse=false, int n=0)
{
    char tmpstr[1024];
    std::ifstream fs(fname);
    if(!fs.is_open())
    {
        MessageBox(NULL, "Error Loading the file", fname, MB_OK);
        return false;
    }
	//
	// Choose how to parse the file
	//
	fs.getline(tmpstr, 1023); //Dummy line
    if(!strncmp(tmpstr, "Frame, Time", 11))
        loadFRAPSms(name, fname);
    else if(!strncmp(tmpstr, "FPS", 3))
        loadFRAPSfps(name, fname);
    else if(!strncmp(tmpstr, "Draw Call #, GPU Time", 21))
        loadPerfHUDReport(name, fname);
    else if(!strncmp(tmpstr, "benchmarkable_OGL_call", 22))
		loadOGLAPICallsCSVFile(name, fname, keywordTable, n ? true : false, bCollapse);
    else if(!strncmp(tmpstr, "benchmarkable_call_number", 25))
		loadD3DAPICallsCSVFile(name, fname, keywordTable, n ? true : false, bCollapse);
    else 
        loadGeneric(name, fname);
    fs.close();
    return true;
}
//
// Compute Delta Values for the 2 first activated curves
// simply creates a new graph...
//
void DeleteDeltaGraphs()
{
	IWindowFolding *pContainer2  = NULL;
	std::vector<TLDisplay *>::iterator iD = g_pDisplays.begin();
	int i=0;
	while(iD != g_pDisplays.end())
	{
		if((*iD)->name == std::string("Delta"))
		{
			pContainer2 = (IWindowFolding*)g_pwinHandler->GetContainer((LPCSTR)((i+1)<<8))->QueryInterface("IWindowFolding");
			if(pContainer2) 
				// I have a bug, here!!!!!
				pContainer2->Destroy();
			g_pDisplays.erase(iD);
			// Compute the distribution over the window
            adjustDisplayRatios();
			return;
		}
		++i;
		++iD;
	}
}
//
// Compute Delta Values for the 2 first activated curves
// simply creates a new graph...
//
void ComputeDeltaValues()
{
    for(int n=0; n<(int)g_pDisplays.size(); n++)
	{
		// pass any Display that is not visible (checked)
		if(g_pDisplays[n]->percentH == 0)
			continue;
		// we need 2 graphs
		int j = 0;
		int goffsets[2] = {-1,-1};
		for(int i=0; i<(int)g_pDisplays[n]->Graphs.size(); i++)
		{
			if(g_pDisplays[n]->Graphs[i].valid)
				goffsets[j++] = i;
			if(j > 1)
				break;
		}
		if(j < 2)
			continue;
		// we have 2 graphs...
		TLDisplay *pDisp = g_pDisplays[n];
		// create the new display for Delta
		TLDisplay *pDispDelta = NULL;
		int dispnum = 0;
		IWindowFolding *pContainer2  = NULL;
		for(int i=0; i<(int)g_pDisplays.size(); i++)
			if(g_pDisplays[i]->name == std::string("Delta"))
			{
				pDispDelta = g_pDisplays[i];
				dispnum = i;
				pContainer2 = (IWindowFolding*)g_pwinHandler->GetContainer((LPCSTR)((i+1)<<8))->QueryInterface("IWindowFolding");
			}
		// Display not found. Let's create it (with the UI)
		if(pDispDelta == NULL)
		{
			pDispDelta = new TLDisplay(g_hwnd);
			pDispDelta->name = "Delta";
			dispnum = (int)g_pDisplays.size();
			g_pDisplays.push_back(pDispDelta);
			IWindow* pWin = (IWindowFolding*)g_pwinHandler->GetContainer((LPCSTR)((int)g_pDisplays.size()<<8));
			if(pWin)
			{
				if(g_pLog) g_pLog->AddMessage("warning>>found a left-over in UI...");
				pContainer2 = (IWindowFolding*)pWin->QueryInterface("IWindowFolding");
			}
			if(!pContainer2)
				pContainer2 = g_pwinHandler->CreateWindowFolding((LPCSTR)((dispnum+1)<<8), "Delta", g_pMainContainer);
		}
		std::string graphname = pDisp->Graphs[goffsets[0]].name + std::string("-") + pDisp->Graphs[goffsets[1]].name;
		int graph = -1;
		for(int i=0; i<(int)pDispDelta->Graphs.size(); i++)
			if(pDispDelta->Graphs[i].name == graphname)
			{
				if(g_pLog) g_pLog->AddMessage("warning>>Delta Graph %s already created...", graphname.c_str());
				return; // we already created this graph
			}
		if(graph < 0)
			graph = pDispDelta->addGraph(graphname.c_str());
		//UI:
		g_pwinHandler->CreateCtrlCheck((LPCSTR)0, graphname.c_str(), pContainer2)->SetChecked(true);
		// populate the graph
		STLMeasures &m1 = pDisp->Graphs[goffsets[0]].Measures;
		STLMeasures &m2 = pDisp->Graphs[goffsets[1]].Measures;
		assert(m1.size() == m2.size());
		int sz = (int)m1.size(); if(sz > (int)m2.size()) sz = (int)m2.size();
		float avegap = 0.0;
		for(int i=0; i<sz; i++)
		{
			pDispDelta->addMeasure(graph,TMeasure(m2[i].timing - m1[i].timing, 0, NULL, NULL, NULL), i);
			avegap += m2[i].timing - m1[i].timing;
		}
		pDispDelta->ScaleToRange();
		pDispDelta->ScaleX = pDisp->ScaleX;
		pDispDelta->xoffs = pDisp->xoffs;
		pDispDelta->FramePos = pDisp->FramePos;
		pDispDelta->shifting = pDisp->shifting;
		pContainer2->UnFold(true);
		break;
	}
}

//
// Save file
//
bool saveComments()
{
    FILE *fp;
    std::string pathname1(g_pathAnnotations);
    pathname1 += "annotations_";
    for(int n=0; n<(int)g_pDisplays.size(); n++)
        for(int g=0 ; g<(int)g_pDisplays[n]->Graphs.size(); g++)
        {
            if(g_pDisplays[n]->rangeAnnotations.size() == 0)
            {
                int annotsNum = 0;
                for(int m=0; m<(int)g_pDisplays[n]->Graphs[g].Measures.size(); m++)
                {
                    TMeasure &measure = g_pDisplays[n]->Graphs[g].Measures[m];
                    if((measure.modified)||(!measure.userComment.empty()))
                        annotsNum++;
                }
                if(annotsNum == 0)
                    continue;
            }
            std::string fullname;
            fullname = pathname1 + g_pDisplays[n]->Graphs[g].name + ".txt";
            fp = fopen(fullname.c_str(), "w");
            if(!fp)
                continue;
            for(int m=0; m<(int)g_pDisplays[n]->Graphs[g].Measures.size(); m++)
            {
                TMeasure &measure = g_pDisplays[n]->Graphs[g].Measures[m];
                if((measure.modified)||(!measure.userComment.empty()))
                {
                    int nextline = measure.userComment.find_first_of('\n') + 1;
                    fprintf(fp, "frame,%d,%d,%s", m, measure.valid ? 1 : 0, measure.valid ? measure.userComment.c_str() + nextline : "truncated");
					if(!measure.userCommentAddons.empty())
					{
						std::string tmpstr(measure.userCommentAddons);
						tmpstr = Pattern::compile("(\\n)")->replace(tmpstr, "\\\\n");
						fprintf(fp, ",%s", tmpstr.c_str());
					}
					fprintf(fp, "\n");
                }
            }
            for(int i=0; i< (int)g_pDisplays[n]->rangeAnnotations.size(); i++)
            {
                TLDisplay::RangeAnnotation &item = g_pDisplays[n]->rangeAnnotations[i];
                fprintf(fp, "range,%d,%d,%s", item.start, item.end, item.text.c_str());
				if(!item.text2.empty())
				{
					std::string tmpstr(item.text2);
					tmpstr = Pattern::compile("(\\n)")->replace(tmpstr, "\\\\n");
					fprintf(fp, ",%s", tmpstr.c_str());
				}
				fprintf(fp, "\n");
            }
            fclose(fp);
        }
    return true;
}

typedef struct SFiles {
	std::string fullPath;
	std::string filename;
	std::string lastFolder;
} TFiles;
void parse_file(std::vector<TFiles> &files, std::string &grp, std::string &commonFilename)
{
	SFiles f;

	grp = Pattern::compile("(\")")->replace(grp, "");
	int p = grp.find_last_of('\\');
	// No \ : folder or name alone
	if(p < 0) 
	{
		if(!commonFilename.empty()) // we know that grp is a folder
		{
			f.lastFolder = grp;
			f.filename = commonFilename;
			f.fullPath = grp + std::string("\\") + f.filename;
		} else if( ((int)grp.find(std::string(".txt"))>=0)
				|| ((int)grp.find(std::string(".csv"))>=0)
				|| ((int)grp.find(std::string(".xls"))>=0))
		{
			f.filename = grp.substr(0,grp.length()-4);
			f.fullPath = grp;
		} else {
			f.lastFolder = grp;
			f.filename = std::string("incremental_call_time_log.csv");
			f.fullPath = grp + std::string("\\") + f.filename;
		}
	// found a '\'
	} else {
		std::string lastgrp = grp.substr(p+1,grp.length()-1);
		std::string beforelastgrp = grp.substr(0,p);
		p = beforelastgrp.find_last_of('\\');
		if(p >= 0)
			 beforelastgrp = beforelastgrp.substr(p+1,grp.length()-1);
		if(!commonFilename.empty()) // we know that grp is a folder
		{
			f.lastFolder = lastgrp;
			f.filename = commonFilename;
			f.fullPath = grp + std::string("\\") + commonFilename;
		} else if( ((int)lastgrp.find(std::string(".txt"))>0) 
				|| ((int)lastgrp.find(std::string(".csv"))>0)
				|| ((int)lastgrp.find(std::string(".xls"))>0))
		{
			int a = lastgrp.find(std::string(".txt"));
			f.lastFolder = beforelastgrp;
			f.filename = lastgrp.substr(0,lastgrp.length()-4);
			f.fullPath = grp;
		} else {
			f.lastFolder = lastgrp;
			f.filename = std::string("incremental_call_time_log.csv");
			f.fullPath = grp + std::string("\\") + f.filename;
		}
	}

	files.push_back(f);

}
/*************************************************************************/ /**
 ** Cardiogram-like graphic.
 ** \todo create methods to display this timeline...
 ** <LI> ///italics///
 ** <LI> '''bold'''
 ** <LI> ___underline___
 ** <LI> col{FF0080} : color FF0080
 ** <LI> size{12} : font size 12
 ** <LI> n : return.
 **/ /*************************************************************************/ 
void displayHelp(bool onLog=false)
{
	if(!g_pConsole)
		return;
	if(!onLog)
	{
		g_pConsole->Clear();
		g_pConsole->SetVisible();
		g_pConsole->SetZPos();
		g_pConsole->SetSize(650,300);
		g_pConsole->Printf("'''HELP'''\n\n"
		"'''nvGraphy.exe <file1> <file2> ...''' : display graphs\n"
		"'''nvGraphy.exe <folder1> <folder2> ...''' : display graphs incremental_call_time_log.csv in folders\n"
		"'''nvGraphy.exe <folder1> <folder2> -n bmark.csv''' : display graphs bmark.csv located in the folders\n"
		"'''nvGraphy.exe incremental_call.csv -ic{memcpy Lock Map} -r CallRanges.txt'''\n"
		"Ignores some line of APIC benchmark containing memcpy, Lock or Map, then collapse all:\n"
		"\n___Options:___\n"
		"'''-i{...}''' : ignore frames having the keywords. But keep the original benchmarkable_call_number\n"
		"'''-ic{}''' : ignore frames having the keywords and collapse the graph to avoid gaps : recompute benchmarkable_call_number\n"
		"'''-r <perfHUD_comments.txt>''' : loads the perfHUD tags\n"
		"'''-a <path>''' : path where to load/save annotations\n"
		"'''-n <filename>''' : when given a list of folders, load files named <filename>\n"
		"'''-c''' : collapse data on x axis\n"
		"\n___UI:___\n"
		"'''right mouse''' : horizontal move\n"
		"'''Ctrl + right mouse''' : horizontal and vertical move\n"
		"'''Ctrl + Shift + right mouse''' : vertical move\n"
		"'''wheel''' : scale X axis\n"
		"'''Ctrl + wheel''' : scale Y axis\n"
		"'''Shift + right click''' : temporarily shift one of the curve (to make it match with another, for example)\n"
		"'''Left mouse button''' : select a range or select a point on a curve\n"
		"'''Del''' : delete a point of a curve (good if the point was irrelevant, for example)\n"
		"'''z''' : zoom to selection. If no selection, zoom to full view\n"
		"\n___File formats:___\n"
		"'''APIC Calls csv''' frame benchmark reports\n"
		"'''FRAPS''' log files\n"
		"'''nvPerfHUD''' csv exported file\n"
		"'''generic''' csv files...\n"
		);
	} else if(g_pLog) {
		g_pLog->AddMessage("-----------------");
		g_pLog->AddMessage("right mouse : horizontal move");
		g_pLog->AddMessage("Ctrl + right mouse : horizontal and vertical move");
		g_pLog->AddMessage("Ctrl + Shift + right mouse : vertical move");
		g_pLog->AddMessage("wheel : scale X axis");
		g_pLog->AddMessage("Ctrl + wheel : scale Y axis");
		g_pLog->AddMessage("Shift + right click : temporarily shift one of the curve (to make it match with another, for example)");
		g_pLog->AddMessage("Left mouse button : select a range or select a point on a curve");
		g_pLog->AddMessage("Del : delete a point of a curve (good if the point was irrelevant, for example)");
		g_pLog->AddMessage("z : zoom to selection. If no selection, zoom to full view");
		g_pLog->AddMessage("nvGraphy.exe -h or help menu for more");
	}
}
int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	std::string cmdLine(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, g_szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_NVGRAPHY, g_szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

    HWND hwnd;
	// Perform application initialization:
	if (!(hwnd=InitInstance(hInstance, nCmdShow)))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_NVGRAPHY));
	//
	// options
	//
	// -a <path> : for annotations
	Pattern *pat = Pattern::compile("-a\\s+\"*([0-9a-zA-Z_\\-\\.\\\\:/]+)\"*");
	Matcher *mm2 = pat->createMatcher(cmdLine);
    if(mm2->findFirstMatch())
    {
		g_pathAnnotations = mm2->getGroup(1) + std::string("\\");
		// remove this option from the cmdline
		cmdLine = pat->replace(cmdLine, "");
	}
	//
	// options
	//
	// -h : help
	pat = Pattern::compile("[-/][\\?h]");
	mm2 = pat->createMatcher(cmdLine);
    if(mm2->findFirstMatch())
    {
		displayHelp();
		// remove this option from the cmdline
		cmdLine = pat->replace(cmdLine, "");
	}
	// -r <PerfHud_CallRanges.txt> : for annotations exported from PerfHUD
	pat = Pattern::compile("-r\\s+\"*([0-9a-zA-Z_\\-\\.\\\\:/]+)\"*");
	mm2 = pat->createMatcher(cmdLine);
	std::string fnamePHAnnotations;
    if(mm2->findFirstMatch())
    {
		fnamePHAnnotations = mm2->getGroup(1);
		// remove this option from the cmdline
		cmdLine = pat->replace(cmdLine, "");
	}
	// -n <filename> : when all have the same filename, you can write it once here
	// then give only the folder names
	pat = Pattern::compile("-n\\s+\"*([0-9a-zA-Z_\\-\\.]+)\"*");
	mm2 = pat->createMatcher(cmdLine);
	std::string commonFilename;
    if(mm2->findFirstMatch())
    {
		commonFilename = mm2->getGroup(1);
		// remove this option from the cmdline
		cmdLine = pat->replace(cmdLine, "");
	}

	// -i{keyword1 keyword2 ...} : ask to ignore in framebenchmark some events that APIC dumped in the csv file
	// -i{memcpy Lock Map}
	// then give only the folder names
	bool bCollapse = false;
	pat = Pattern::compile("-i(c?)\\{([^\\}]+)\\}");
	mm2 = pat->createMatcher(cmdLine);
	std::string keywords;
	std::vector<std::string> keywordTable;
    if(mm2->findFirstMatch())
    {
		bCollapse = mm2->getGroup(1) == "c";
		keywords = mm2->getGroup(2);
		// remove this option from the cmdline
		cmdLine = pat->replace(cmdLine, "");
		pat = Pattern::compile("([^\\s]+)");
		mm2 = pat->createMatcher(keywords);
		if(mm2->findFirstMatch())
		{
			keywordTable.push_back(mm2->getGroup(1));
			while(mm2->findNextMatch())
				keywordTable.push_back(mm2->getGroup(1));
		}
	}

	// -c : collapse data on x axis
	mm2 = Pattern::compile("-c")->createMatcher(cmdLine);
    if(mm2->findFirstMatch())
    {
		bCollapse = true;
		// remove this option from the cmdline
		cmdLine = pat->replace(cmdLine, "");
	}

	//
	// load files
	//
	std::vector<TFiles> files;
	bool bIdenticalFNames = false;
	//
	// get the names in a table
	// check if filenames are the same. If so, take the last folders as the names
	//
	mm2 = Pattern::compile("\"([^\"]+)\"")->createMatcher(cmdLine);
	std::string cmdlineWithoutQuotes;
    if(!mm2->findFirstMatch())
	{
		cmdlineWithoutQuotes = Pattern::compile("\"([^\"]+)\"")->replace(cmdLine, "");
		mm2 = Pattern::compile("[\\\\:/a-zA-Z_\\-0-9\\.\\+]+")->createMatcher(cmdlineWithoutQuotes);
	}
    if(mm2->findFirstMatch())
    {
		std::string grp = mm2->getGroup();
		parse_file(files, grp, commonFilename);
        while(mm2->findNextMatch())
		{
			std::string grp = mm2->getGroup();
			parse_file(files, grp, commonFilename);
		}
	}
	//
	// Check if the files names are similar
	//
	if(files.size() >= 2)
	{
		if(files[0].filename == files[1].filename)
			bIdenticalFNames = true;
	}
	//
	// Load the files. Use folder or filename for naming the graphs
	//
	for(int i=0; i<(int)files.size(); i++)
	{
		loadFile(
			bIdenticalFNames ? files[i].lastFolder.c_str() : files[i].filename.c_str(), 
			files[i].fullPath.c_str(), keywordTable, bCollapse);
	}
	//
	// grab the g_pathAnnotations
	// Let's take the first folder in the list...
	//
	if(g_pathAnnotations.empty() && !files.empty())
    {
        g_pathAnnotations = files[0].fullPath;
        int p = g_pathAnnotations.find_last_of('\\');
        if(p>=0) 
        {
			g_pathAnnotations = g_pathAnnotations.substr(0,p+1) + std::string("\\");
        }
        else
            g_pathAnnotations.clear();
    }
	/*To remove... else {
		MessageBox(hwnd, "Bad arguments : append some csv files", cmdLine, MB_OK);
		UISERVICE_UNLOAD();
		return 0;
    }*/
    g_pProgressBar->SetVisible(0);

    adjustDisplayRatios();

    std::string fullname(g_pathAnnotations);
    fullname += "APICalls_Comments.txt";
    loadComments(fullname.c_str());
	// Load annotations from PerfHUD (Ignacio's cooking)
	if(!fnamePHAnnotations.empty())
		loadPHAnnotations(fnamePHAnnotations.c_str());

    RECT r;
    GetClientRect(hwnd, &r);
    r.right -= r.left;
    r.bottom -= r.top;
    OnSize(hwnd, r.right, r.bottom);
    for(int i=0; i<(int)g_pDisplays.size(); i++)
        g_pDisplays[i]->ScaleToRange();

	displayHelp(true);

    // Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	g_pwinHandler->DestroyAll();
    UISERVICE_UNLOAD(g_pFact, g_pwinHandler);
	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_NVGRAPHY));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_NVGRAPHY);
	wcex.lpszClassName	= g_szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
HWND InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hwnd;

   g_hInst = hInstance; // Store instance handle in our global variable

   hwnd = CreateWindow(g_szWindowClass, g_szTitle, WS_OVERLAPPEDWINDOW,
      280, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hwnd)
   {
      return FALSE;
   }
   g_hwnd = hwnd;

    UISERVICE_LOAD(g_pFact, g_pwinHandler);
	if(g_pFact)
	{
		LPCSTR desc;
		LPCSTR revision;
		g_pFact->GetModuleInfos(desc, revision);
		if(strcmp(revision, WINDOWS_VERSION))
		{
			printf("Warning : UI dll " UIDLLNAME " has a wrong version (%s). Need " WINDOWS_VERSION, revision);
			UISERVICE_UNLOAD(g_pFact, g_pwinHandler);
			g_pwinHandler = NULL;
			g_pFact = NULL;
		}
	}
    //if(g_pFact) g_pwinHandler = (IWindowHandler*)g_pFact->GetSingletonOf("windowhandler");
    if(g_pwinHandler)
    {
        g_pwinHandler->Register(&myEvents);
        g_pProgressBar = g_pwinHandler->CreateWindowProgressBar("PROG", "Loading data", 0);
        g_pProgressBar->SetVisible();
        g_pMainContainer = g_pwinHandler->CreateWindowContainer("MAIN", "Graphy", NULL);
		//
		//
		//
		IWindowFolding* pSpecialsFold = g_pwinHandler->CreateWindowFolding("SPECIALSWIN","Specials", g_pMainContainer);
		{
			g_pwinHandler->VariableBind(
				g_pwinHandler->CreateCtrlCheck("TIGHT", "Tight graphs", pSpecialsFold),
				&g_bTightGraphs);
			g_pwinHandler->VariableBind(
				g_pwinHandler->CreateCtrlCheck("CLEARCON", "clear State-output", pSpecialsFold),
				&g_bAutoClearConsole);
			g_pPixelShaderEditBox = g_pwinHandler->CreateCtrlString("PIXSHD", "Pix. Shader", pSpecialsFold);
			g_pwinHandler->CreateCtrlButton("SHOWSHD","Show PShader", pSpecialsFold);
			g_pwinHandler->CreateCtrlButton("SHOWRT","Highlight RT", pSpecialsFold);
			pSpecialsFold->UnFold(0);
		}
		IWindowFolding* pAnnotsFold = g_pwinHandler->CreateWindowFolding("REMWIN","Annotations", g_pMainContainer);
		{
			g_pCommentEditBox = g_pwinHandler->CreateCtrlString("REMSTR", "Comments", pAnnotsFold);
			IWindowFolding* pLogFold = g_pwinHandler->CreateWindowFolding("BLOGWIN","More comments", pAnnotsFold);
			{
				g_pCommentConsole = g_pwinHandler->CreateWindowConsole("REMCONSOLE", "Notes", pLogFold);
				g_pCommentConsole->SetSize(200, 200);
				pLogFold->UnFold(0);
			}
			g_pwinHandler->CreateCtrlButton("SAVE","Save annotations", pAnnotsFold);
		}
		IWindowFolding* pSearchFold = g_pwinHandler->CreateWindowFolding("SEARCHWIN","Search Window", g_pMainContainer);
		{
			g_pPredefSearch = g_pwinHandler->CreateCtrlCombo("COMBSEARCH", "Predefined search", pSearchFold);
			g_pPredefSearch->AddItem("Tex depthcompare",0);
			g_pPredefSearch->AddItem("ZBuf as Tex",1);
            // ADD MORE PREDEFINED SEARCH HERE

			g_pwinHandler->CreateCtrlString("SEARCH", "Search", pSearchFold);
			g_pwinHandler->VariableBind(
				g_pwinHandler->CreateCtrlCheck("USEREGEXP", "Use RegExp", pSearchFold),
				&g_bRegExp);
			g_pwinHandler->CreateCtrlButton("CLEAR","Clear search", pSearchFold);
			g_pwinHandler->VariableBind(
				g_pwinHandler->CreateCtrlCheck("SEARCHTAG", "Tag Search", pSearchFold),
				&g_bTagSearch);
			g_pwinHandler->VariableBind(
				g_pwinHandler->CreateCtrlCheck("CLRSEARCHTAG", "Clear Tag Search", pSearchFold),
				&g_bClearTagSearch);
		}

		g_pMainContainer->SetVisible();
        g_pMainContainer->SetLocation(0,0);
        g_pMainContainer->SetSize(280, 800);
        g_pConsole = g_pwinHandler->CreateWindowConsole("CONSOLE", "State", NULL);
        RECT r;
        GetWindowRect(hwnd, &r);
        g_pConsole->SetLocation(r.left + 200,0);
        g_pConsole->SetSize(800, 900);
        //g_pConsole->SetVisible();

		IWindowFolding *pLogFolding = g_pwinHandler->CreateWindowFolding("LOGFOLDING", "Logging", g_pMainContainer);
        g_pLog = g_pwinHandler->CreateWindowLog("Log", "State", pLogFolding);
        g_pLog->SetSize(100, 200);
        g_pLog->SetVisible();

		// To ask for a Delta curve between 2 activated curves
		IControlToolbar* pDeltaContainer = g_pwinHandler->CreateCtrlToolbar("DELTATB", "Deltas", g_pMainContainer);
		{
			pDeltaContainer->AddItem("Compute Delta", NULL);
			pDeltaContainer->AddItem("Delete Delta", NULL);
		}

        g_pConsolePShader = g_pwinHandler->CreateWindowConsole("CONSOLEPS", "Pixel Shader", NULL);
        g_pConsolePShader->SetLocation(r.left + 400,0);
        g_pConsolePShader->SetSize(800, 900);
        //g_pConsole->SetVisible();
    }    

   ShowWindow(hwnd, nCmdShow);
   UpdateWindow(hwnd);

   return hwnd;
}


void OnCreate(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LPCREATESTRUCT lpc = (LPCREATESTRUCT)lParam;
    TLDisplay::createStaticResources(lpc->hInstance);
}
void OnDestroy(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    TLDisplay::releaseStaticResources();
    for(int i=0; i<(int)g_pDisplays.size(); i++)
    {
        delete g_pDisplays[i];
    }
    g_pDisplays.clear();
}
void OnPaint(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT Paint;

	HDC dc = BeginPaint(hwnd, &Paint);

	int w = Paint.rcPaint.right-Paint.rcPaint.left;
	int h = Paint.rcPaint.bottom-Paint.rcPaint.top;
	if(w >= Paint.rcPaint.right)
		w = Paint.rcPaint.right-1;
	if((h >= Paint.rcPaint.bottom)&&((int)g_pDisplays.size() > 0))
		h = (Paint.rcPaint.bottom-1)/(int)g_pDisplays.size();
    for(int i=0; i<(int)g_pDisplays.size(); i++)
    {
        g_pDisplays[i]->blitBitmap(dc, Paint.rcPaint);
    }
	EndPaint(hwnd, &Paint);
}
void OnMouseMove(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//
	// Left button
	//
    g_curMouse[0] = LOWORD(lParam);
    g_curMouse[1] = HIWORD(lParam);
	if((wParam & MK_RBUTTON))
	{
        for(int i=0; i<(int)g_pDisplays.size(); i++)
        {
            /* To remove...
			bool sowhat = wParam&MK_SHIFT ? false : g_bTightGraphs;
            if(!sowhat && !g_pDisplays[i]->inBitmap(g_StartMouse[0], g_StartMouse[1]))
                continue;*/

			if(wParam&MK_SHIFT) // Vertical shift of the selected curve...
			{
				g_pDisplays[i]->ShiftPickedGraph(0, HIWORD(lParam) - g_StartMouse[1]);
			} else { // normal pan
                bool sowhat = wParam&MK_CONTROL ? false : g_bTightGraphs;
                if(!sowhat && !g_pDisplays[i]->inBitmap(g_curMouse[0], g_curMouse[1]))
                    g_pDisplays[i]->SetPosFromMouse(LOWORD(lParam) - g_StartMouse[0], 0);
				else
					g_pDisplays[i]->SetPosFromMouse(LOWORD(lParam) - g_StartMouse[0], wParam&MK_CONTROL ? HIWORD(lParam) - g_StartMouse[1] : 0);
			}
            g_pDisplays[i]->update(hwnd);
        }
	    SetCursor(TLDisplay::hcursor_handdn);
	}
	else if(wParam & MK_LBUTTON)
	{
        for(int i=0; i<(int)g_pDisplays.size(); i++)
        {
            if(!g_bTightGraphs && !g_pDisplays[i]->inBitmap(LOWORD(lParam), HIWORD(lParam)))
                continue;
		    g_pDisplays[i]->SetXSelectionEnd(LOWORD(lParam) - g_StartMouse[0]);
            if(g_curMouse[0] < 20)
            {
    		    g_pDisplays[i]->ScrollPos(4, 0);
            }
            if(g_curMouse[0] > g_pDisplays[i]->bmpW-20)
            {
    		    g_pDisplays[i]->ScrollPos(-4, 0);
            }
            g_pDisplays[i]->update(hwnd);
        }
		SetCursor(TLDisplay::hcursor_arrow);
	}
	//
	// right button
	//
	/*else if(wParam & MK_RBUTTON)
	{
        for(int i=0; i<(int)g_pDisplays.size(); i++)
        {
            if(!g_bTightGraphs && !g_pDisplays[i]->inBitmap(g_StartMouse[0], g_StartMouse[1]))
                continue;
		    g_pDisplays[i]->SetScaleFromMouse(
                ((float)(LOWORD(lParam) - g_StartMouse[0]))/(float)g_pDisplays[i]->bmpW,
                ((float)(HIWORD(lParam) - g_StartMouse[1]))/(float)g_pDisplays[i]->bmpH
                );
		    //SetPosFromMouse(0, HIWORD(lParam) - g_StartMouse[1]);
            g_pDisplays[i]->update(hwnd);
        }
		SetCursor(TLDisplay::hcursor_handzoom);
	}*/
	//
	//====> Do the Picking just for cursor & highlight
	//
	//else
	{
	    SetCursor(TLDisplay::hcursor_arrow);
        for(int i=0; i<(int)g_pDisplays.size(); i++)
        {
//            if(!g_bTightGraphs && !g_pDisplays[i]->inBitmap(LOWORD(lParam), HIWORD(lParam)))
//                continue;
		    if(g_pDisplays[i]->PickScene(LOWORD(lParam), HIWORD(lParam), PICK_HIGHLIGHT))
		    {
                g_pDisplays[i]->update(hwnd);
                SetCursor(TLDisplay::hcursor_arrow);
		    }
        }
	}
}
void OnLButtonDown(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //
    // Pan view
    //
    if(wParam & MK_SHIFT)
    {
	    g_StartMouse[0] = LOWORD(lParam);
	    g_StartMouse[1] = HIWORD(lParam);
        SetCursor(TLDisplay::hcursor_hand);
    }
    //
    // Start a selection
    //
    else 
    {
        SetCursor(TLDisplay::hcursor_arrow);//TODO: create a cursor for this.
        g_displaySlectedFirst = -1;
        //bool bNoSelection = true;
        for(int i=0; i<(int)g_pDisplays.size(); i++)
        {
            g_pDisplays[i]->ClearSelection(); // safe to do this, first...
            if(g_pDisplays[i]->inBitmap(LOWORD(lParam), HIWORD(lParam)))
                g_displaySlectedFirst = i; // Hack... sorry
            if(!g_bTightGraphs && !g_pDisplays[i]->inBitmap(LOWORD(lParam), HIWORD(lParam)))
                continue;
		    g_pDisplays[i]->SetXSelectionStart(LOWORD(lParam));
            //
            // potential selection
            //
		    if(g_pDisplays[i]->PickScene(LOWORD(lParam), HIWORD(lParam), PICK_SELECT))
		    {
                //
                // Actions on selection
                //
                TMeasure *pM = g_pDisplays[i]->getSelected();
                if(pM && pM->stateID >= 0)
                {
                    if(g_bAutoClearConsole)
                        g_pConsole->Clear();
                    g_pConsole->Printf("\\n___State %d for Drawcall %d:___\\n", pM->stateID, pM->drawcall);
                    std::string decoratedString;
                    //decorateState(g_states[pM->stateID].stateStr, decoratedString);
                    g_pConsole->Printf(decoratedString.c_str());
                    g_pConsole->SetVisible();
                }
                //bNoSelection = false;
            } 
            g_pDisplays[i]->update(hwnd);
        }
        /*if(bNoSelection) 
        {
            g_pConsole->SetVisible(0);
            g_pConsolePShader->SetVisible(0);
        }*/
    }
}
void OnRButtonDown(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	g_StartMouse[0] = LOWORD(lParam);
	g_StartMouse[1] = HIWORD(lParam);
	SetCursor(TLDisplay::hcursor_handzoom);

	if(wParam&MK_SHIFT) // Vertical shift of the selected curve...
	{
        for(int i=0; i<(int)g_pDisplays.size(); i++)
        {
			g_pDisplays[i]->LockPickedGraph(true);
		}
	}
}
void OnLButtonUp(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    for(int i=0; i<(int)g_pDisplays.size(); i++)
    {
        if(!g_bTightGraphs && !g_pDisplays[i]->inBitmap(g_StartMouse[0], g_StartMouse[1]))
            continue;
    	g_pDisplays[i]->AdjustOffsetFromMouse();
    }
	SetCursor(TLDisplay::hcursor_arrow);
	g_StartMouse[0] = 0;
	g_StartMouse[1] = 0;
}
void OnRButtonUp(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	g_StartMouse[0] = 0;
	g_StartMouse[1] = 0;
    for(int i=0; i<(int)g_pDisplays.size(); i++)
    {
    	g_pDisplays[i]->AdjustOffsetFromMouse();
		g_pDisplays[i]->LockPickedGraph(false);
		g_pDisplays[i]->ShiftPickedGraph(0,0);
    }
	SetCursor(TLDisplay::hcursor_hand);
}
void OnKeyDown(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  for(int i=0; i<(int)g_pDisplays.size(); i++)
  {
    if(!g_bTightGraphs && !g_pDisplays[i]->inBitmap(g_curMouse[0], g_curMouse[1]))
        continue;
    switch(wParam)
    {
    case ' ':
        break;
    case 46: //del
        g_pDisplays[i]->smoothSelectedValue();
        g_pDisplays[i]->deleteSelectedRangeAnnotation();
        break;
    case 90: //'z'
        g_pDisplays[i]->ScaleToSelection();
        break;
	case 'X':
        g_pDisplays[i]->ScaleToRange();
		break;
	case 'R': // Render target highlight
		if(i > 0)
			break;
		g_pDisplays[i]->searchResults.clear();
		{
			int graph, frame;
			TMeasure *pM = g_pDisplays[i]->getSelected(&graph, &frame);
			if(!pM)
				continue;
			//highlightCurrentRT(frame);
		}
		g_pDisplays[i]->update(g_hwnd);
		break;
    case 37: //left
		g_pDisplays[i]->ScrollPos(10, 0);
    	g_pDisplays[i]->AdjustOffsetFromMouse();
        break;
    case 38: //top
		g_pDisplays[i]->ScrollPos(0, 10);
    	g_pDisplays[i]->AdjustOffsetFromMouse();
        break;
    case 39: //right
		g_pDisplays[i]->ScrollPos(-10, 0);
    	g_pDisplays[i]->AdjustOffsetFromMouse();
        break;
    case 40: //bottom
		g_pDisplays[i]->ScrollPos(0, -10);
    	g_pDisplays[i]->AdjustOffsetFromMouse();
        break;
    }
    g_pDisplays[i]->update(hwnd);
  }

  switch(wParam)
    {
    case 'S': //save
        saveComments();
        break;
    }
  //UpdateWindow(hwnd);
}
void OnSize(HWND hwnd, int w, int h)
{
    int y = 0;
    for(int i=0; i<(int)g_pDisplays.size(); i++)
    {
        g_pDisplays[i]->bmpY = y;
        int h2 = (h * g_pDisplays[i]->percentH) / 100;
        g_pDisplays[i]->ResizeBitmap(w, h2);
        g_pDisplays[i]->update(hwnd);
        y += h2;
    }
}
//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;

	switch (message)
	{
	case WM_MOUSEWHEEL:
        {
            short zdelta = HIWORD(wParam);
            float zscale = ((float)zdelta)/1000.0f;
            for(int i=0; i<(int)g_pDisplays.size(); i++)
            {
                bool sowhat = wParam&MK_CONTROL ? false : g_bTightGraphs;
                if(!sowhat && !g_pDisplays[i]->inBitmap(g_curMouse[0], g_curMouse[1]))
                    continue;
                g_pDisplays[i]->SetScaleFromMouseWheel(
                    wParam&MK_CONTROL ? 0.0f : zscale, 
                    wParam&MK_CONTROL ? zscale : 0.0f,
                    g_curMouse[0],//LOWORD(lParam),
                    g_curMouse[1]);//HIWORD(lParam));
                g_pDisplays[i]->update(hwnd);
            }
        }
        break;
	case WM_CREATE:
        OnCreate(hwnd, message, wParam, lParam);
        break;
	case WM_DESTROY:
        OnDestroy(hwnd, message, wParam, lParam);
		PostQuitMessage(0);
        break;
    case WM_SIZE:
        OnSize(hwnd, LOWORD(lParam), HIWORD(lParam));
        break;
    case WM_MOUSEMOVE:
        OnMouseMove(hwnd, message, wParam, lParam);
        break;
    case WM_LBUTTONDOWN:
        OnLButtonDown(hwnd, message, wParam, lParam);
        break;
    case WM_LBUTTONUP:
        OnLButtonUp(hwnd, message, wParam, lParam);
        break;
    case WM_RBUTTONDOWN:
        OnRButtonDown(hwnd, message, wParam, lParam);
        break;
    case WM_RBUTTONUP:
        OnRButtonUp(hwnd, message, wParam, lParam);
        break;
	case WM_KEYDOWN:
        OnKeyDown(hwnd, message, wParam, lParam);
        break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			displayHelp();
			//DialogBox(g_hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hwnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hwnd);
			break;
		default:
			return DefWindowProc(hwnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
        OnPaint(hwnd, message, wParam, lParam);
		break;
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
	return 0;
}
// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
