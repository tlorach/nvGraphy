#include "stdafx.h"
//
// Load
//
bool loadD3DAPICallsCSVFile_(const char *name, char *fname, int n=0)
{
    char tmpstr[1024];
    //std::basic_ifstream<char> fs(fname);
    std::ifstream fs(fname);
    if(!fs.is_open())
	{
		if(g_pLog) g_pLog->AddMessage("no>>Failure...");
        return false;
	}
    fs.getline(tmpstr, 1023); //Dummy line
    if(*tmpstr == '\0')
	{
		if(g_pLog) g_pLog->AddMessage("no>>Failure...");
        return false;
	}
    //strcpy(tmpstr, fname);
    //char *c = strchr(tmpstr, '.');
    //if(c) *c = '\0';
    //c = strrchr(tmpstr, '\\');
    int graph = g_pDisplays[n]->addGraph(name);//c ? c+1 : tmpstr);
    //UI:
    g_pwinHandler->CreateCtrlCheck((LPCSTR)graph, tmpstr, g_pMainContainer)->SetChecked(true);
    //Fill in the graph
    do {
        int benchmarkable_call_number;
        float frame_time_ms;
        float time_difference;
        std::string call_type;
        int draw_call_number;
        int calls_skipped;
        int calls_executed;
        std::string ps_text;
        std::string vs_text;
        std::string vdecl_text;
        std::string comment;
        fs.getline(tmpstr, 1023, ',');benchmarkable_call_number = atoi(tmpstr);
        fs.getline(tmpstr, 1023, ',');frame_time_ms = (float)atof(tmpstr);
        fs.getline(tmpstr, 1023, ',');time_difference = (float)atof(tmpstr);
        fs.getline(tmpstr, 1023, ',');call_type = tmpstr;
        fs.getline(tmpstr, 1023, ',');draw_call_number = atoi(tmpstr);
        fs.getline(tmpstr, 1023, ',');calls_skipped = atoi(tmpstr);
        fs.getline(tmpstr, 1023, ',');calls_executed = atoi(tmpstr);
        fs.getline(tmpstr, 1023, ',');ps_text = tmpstr;
        fs.getline(tmpstr, 1023, ',');vs_text = tmpstr;
        fs.getline(tmpstr, 1023, '\n'); vdecl_text = tmpstr;
        int offset = (int)vdecl_text.find_first_of(',');
        if(offset > 0) comment = std::string(vdecl_text.c_str() + offset + 1);
        if((!fs.eof()) && (benchmarkable_call_number >= 0))
        {
            g_pDisplays[n]->addMeasure(graph, TMeasure(frame_time_ms, draw_call_number, call_type.c_str(), vs_text.c_str(), ps_text.c_str()), benchmarkable_call_number);
            if(call_type == "Clear")
            {
                g_pDisplays[n]->setCommentForMeasure(graph, benchmarkable_call_number, call_type.c_str(), NULL, RGB(128,160,128));
            }
            /*if((call_type == "Lock")
                ||(call_type == "memcpy"))
            {
                g_pDisplays[n]->setCommentForMeasure(graph, benchmarkable_call_number, NULL, NULL, RGB(128,160,128));
            }*/
            if(!comment.empty())
            {
                int i;
                while((i=(int)comment.find_first_of("|")) >= 0)
                {
                    comment.replace(i, 1,"\n");
                }
                g_pDisplays[n]->Graphs[graph].Measures[benchmarkable_call_number].tooltipComments = comment;
            }
        }
    } while(!fs.eof());
    return true;
}
//
// Load the D3DAPICalls benchmark report:
//
// benchmarkable_call_number,frame_time_ms,time_difference,call_type,draw_call_number,calls_skipped,calls_executed,ps_text,vs_text,vdecl_text
//
bool loadD3DAPICallsCSVFile(const char *name, const char *fname, const std::vector<std::string> &keywordTable, bool bDiffGraph, bool bCollapse)
{
    char tmpstr[1024];
    int graphDiff = -1;

    std::ifstream fs(fname);
    if(!fs.is_open())
	{
		if(g_pLog) g_pLog->AddMessage("no>>Failure...");
        return false;
	}
    // for size of the file so we can display a progress bar
    fs.seekg( -1, std::ios_base::end );
    int bSize = (int)fs.tellg();
    fs.seekg( 0, std::ios_base::beg );
    g_pProgressBar->SetTitle("Loading D3DAPICalls benchmark data...");
	if(g_pLog) g_pLog->AddMessage("Loading D3DAPICalls benchmark data %s", fname);

    fs.getline(tmpstr, 1023); //Dummy line
    if(*tmpstr == '\0')
	{
		if(g_pLog) g_pLog->AddMessage("no>>Failure...");
        return false;
	}
    g_apiCall.clear();

    int disp = (int)g_pDisplays.size();
    IWindowFolding *pWFold;
    if(disp == 0)
    {
        TLDisplay *pDisp = new TLDisplay(g_hwnd);
        pDisp->name = "API Calls Benchmark";
        g_pDisplays.push_back(pDisp);
        //UI:
        pWFold = g_pwinHandler->CreateWindowFolding((LPCSTR)(1<<16), "API Calls Benchmark", g_pMainContainer);
    }
    else
    {
        pWFold = (IWindowFolding *)g_pwinHandler->Get((LPCSTR)(1<<16))->QueryInterface("IWindowFolding");
    }
    // LET's ASSUME that this is always done in display #0 :
    disp = 0;
    int graphTime = g_pDisplays[disp]->addGraph(name);

    //Create a second graph for time difference...
    if(bDiffGraph)
    {
        TLDisplay *pDisp = new TLDisplay(g_hwnd);
        pDisp->name = "API Calls time difference";
        g_pDisplays.push_back(pDisp);
        graphDiff = g_pDisplays[1]->addGraph(name);
    }
    //UI:
    g_pwinHandler->CreateCtrlCheck((LPCSTR)graphTime, name, pWFold)->SetChecked(true);

    //Examples:
    //-1,2.40242,2.40242,first_call_was_skipped,,1999,0
    //0,2.4076,0.005188,Clear,,1998,1
    //Examples2:
    //-1,2.10559,2.10559,first_call_was_skipped,,4147,0,,,
    //0,2.42992,0.32433,Clear,,4146,1,,,
    //2,2.66265,0.215147,Draw,0,4144,3,ps_ea97b1f6b1cbfb27_1,vs_text_1,vdecl_text_1

    //Fill in the graph
    Pattern::registerPattern("int", "\\s*([\\-0-9]*)\\s*");
    Pattern::registerPattern("float", "\\s*([\\-0-9\\.eE]*)\\s*");
    Pattern::registerPattern("text", "\\s*(\\w*)\\s*");
    Pattern *p = Pattern::compile("{int},{float},{float},(\\w+),?{int},?{int},?{int},?{text},?{text},?{text}");
    Matcher *m = p->createMatcher("");
	int benchmarkable_call_number_collapsed = 0;
    do {
        int benchmarkable_call_number;
        float frame_time_ms;
        float time_difference;
        std::string call_type;
        int draw_call_number;
        int calls_skipped;
        int calls_executed;
        std::string ps_text;
        std::string vs_text;
        std::string vdecl_text;
        std::string comment;

        fs.getline(tmpstr, 1023);
        if(fs.eof())
            break;
        m->setString(tmpstr);
        if(m->findFirstMatch())
        {
            benchmarkable_call_number = atoi(m->getGroup(1).c_str());
            frame_time_ms = (float)atof(m->getGroup(2).c_str());
            time_difference = (float)atof(m->getGroup(3).c_str());
            call_type = m->getGroup(4);
            draw_call_number = atoi(m->getGroup(5).c_str());
            calls_skipped = atoi(m->getGroup(6).c_str());
            calls_executed = atoi(m->getGroup(7).c_str());
            ps_text = m->getGroup(8);
            vs_text = m->getGroup(9);
            vdecl_text = m->getGroup(10);
            comment = m->getGroup(11);
			for(int i=0; i< keywordTable.size(); i++)
			{
				if(call_type == keywordTable[i])
				{
					benchmarkable_call_number = -1;
					break;
				}
			}
            if((!fs.eof()) && (benchmarkable_call_number >= 0))
            {
				if(bCollapse)
					benchmarkable_call_number = benchmarkable_call_number_collapsed;
                if(bDiffGraph)
                    g_pDisplays[1]->addMeasure(graphDiff, TMeasure(time_difference, draw_call_number, call_type.c_str(), vs_text.c_str(), ps_text.c_str()), benchmarkable_call_number);
                g_pDisplays[disp]->addMeasure(graphTime, TMeasure(frame_time_ms, draw_call_number, call_type.c_str(), vs_text.c_str(), ps_text.c_str()), benchmarkable_call_number);
                if(call_type == "Clear")
                {
                    g_pDisplays[disp]->setCommentForMeasure(graphTime, benchmarkable_call_number, call_type.c_str(), NULL, RGB(128,160,128));
                }
                if(call_type == "Draw")
                {
                    g_apiCall.push_back(benchmarkable_call_number);
                    //assert((g_apiCall.size()-1) == draw_call_number);
                }
                /*if((call_type == "Lock")
                    ||(call_type == "memcpy"))
                {
                    g_pDisplays[disp]->setCommentForMeasure(graphTime, benchmarkable_call_number, NULL, NULL, RGB(128,160,128));
                }*/
                if(!comment.empty())
                {
                    int i;
                    while((i=(int)comment.find_first_of("|")) >= 0)
                    {
                        comment.replace(i, 1,"\n");
                    }
                    g_pDisplays[disp]->Graphs[graphTime].Measures[benchmarkable_call_number].tooltipComments = comment;
                }
				benchmarkable_call_number_collapsed++;
            }
        }
        else 
        {
            assert(!"ERROR in parsing the line");
			if(g_pLog) g_pLog->AddMessage("error>>Error in parsing a line");
        }
        if((benchmarkable_call_number % 200)==0)
            g_pProgressBar->SetPercent((float)(100*fs.tellg()/bSize));
    } while(!fs.eof());
	if(g_pLog) g_pLog->AddMessage("yes>>Done");
    return true;
}
