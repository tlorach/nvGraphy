#include "stdafx.h"
//
// Load a "generic" set of values : You need the frame number first.
// Frame, name1, name2, Name3, ...
// 1, 1.5, 21%,3.1...
// 2, 35%, 1.1,3.0...
// 7, 0.5, 0.1,0.0...
// ...
//
bool loadGeneric(const char *name, const char *fname)
{
    int i = 0, graph = 0;
    char tmpstr[1024];
    //std::basic_ifstream<char> fs(fname);
    std::ifstream fs(fname);
    if(!fs.is_open())
	{
		if(g_pLog) g_pLog->AddMessage("no>>Failure...");
        return false;
	}
    fs.getline(tmpstr, 1023); //line for value names
    if(*tmpstr == '\0')
	{
		if(g_pLog) g_pLog->AddMessage("no>>Failure...");
        return false;
	}
    // for size of the file so we can display a progress bar
    fs.seekg( -1, std::ios_base::end );
    int bSize = fs.tellg();
    fs.seekg( 0, std::ios_base::beg );
    g_pProgressBar->SetTitle("Loading generic csv data...");
	if(g_pLog) g_pLog->AddMessage("warning>>Loading Generic data. No special format recognized...");

	// done outside
    //Matcher *mm = Pattern::compile("\\\\?([0-9a-zA-Z_]+)\\.")->createMatcher(std::string(fname));
    //mm->findFirstMatch();
    //std::string filename = mm->getGroup(1);

    // get the names
	Matcher *mm2 = Pattern::compile("\\s*(([0-9a-zA-Z_\\s#\\[\\]\\*\\(\\)]+))\\s*,*")->createMatcher(tmpstr);
    if(mm2->findFirstMatch())
    {
        IWindowFolding *pWFold;
        //NOTE: we assume all graphs will have the same 'graph' number...
        while(mm2->findNextMatch())
        {
            if(i >= g_pDisplays.size()) 
            {
                TLDisplay *pDisp = new TLDisplay(g_hwnd);
                pDisp->name = mm2->getGroup(1).c_str();
                g_pDisplays.push_back(pDisp);

                //UI:
                pWFold = g_pwinHandler->CreateWindowFolding((LPCSTR)((i+1)<<8), mm2->getGroup(1).c_str(), g_pMainContainer);
            }
            else
            {
                pWFold = (IWindowFolding *)g_pwinHandler->Get((LPCSTR)((i+1)<<8))->QueryInterface("IWindowFolding");
            }
            graph = g_pDisplays[i]->addGraph(name);
            //UI:
            g_pwinHandler->CreateCtrlCheck((LPCSTR)(((i+1)<<8)+graph+1), name, pWFold)->SetChecked(true);
            i++;
        }
    }
    if(i == 0)
	{
		if(g_pLog) g_pLog->AddMessage("no>>Failure...");
        return false;
	}
    //Fill in the graph
    Pattern *p = Pattern::compile("\\s*([\\-0-9\\.]*)(%*)\\s*,*");
    Matcher *m = p->createMatcher("");
    int frame = 0;
    float value;
    do {

        fs.getline(tmpstr, 1023);
        m->setString(tmpstr);
        if(m->findFirstMatch())
        {
            frame = atoi(m->getGroup(1).c_str());
            i = 0;
            while(m->findNextMatch())
            {
                if(i >= g_pDisplays.size())
                    break;
                value = atof(m->getGroup(1).c_str());
                char c = m->getGroup(2).c_str()[0];
                if(m->getGroup(2).c_str()[0] == '%')
                    value /= 100.0;
                g_pDisplays[i]->addMeasure(graph, TMeasure(value, frame, NULL, NULL, NULL), frame);
                i++;
            }
        }
        if((frame % 10)==0)
            g_pProgressBar->SetPercent(100*fs.tellg()/bSize);
    } while(!fs.eof());
	if(g_pLog) g_pLog->AddMessage("yes>>Done");
    return true;
}
