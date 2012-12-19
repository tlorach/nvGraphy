#include "stdafx.h"
#define NUM_BASE_VALUES 12
#define NUM_EXP_EVENTS 11
//
// Load the API Calls benchmark report:
//
//
bool loadOGLAPICallsCSVFile(const char *name, const char *fname, const std::vector<std::string> &keywordTable, bool bDiffGraph, bool bCollapse)
{
	int graphSignal, graphRawSignal, firstGraphId, firstGraphRawId;
	int firstGraphIdOffset, firstGraphRawIdOffset;
    char tmpstr[1024];
	std::vector< std::string > itemNames;
	std::vector< bool > itemIsPercent;
    int graphDiff = -1;
	bool bHasSignals = true;
	bool bHasAnalysis = true;
	int numSignals = 0, startIdxSignals = 0;

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
    g_pProgressBar->SetTitle("Loading API Calls benchmark data...");
	if(g_pLog) g_pLog->AddMessage("Loading API Calls benchmark data %s", fname);

    fs.getline(tmpstr, 1023); //title line
    if(*tmpstr == '\0')
	{
		if(g_pLog) g_pLog->AddMessage("no>>Failure...");
        return false;
	}
    g_apiCall.clear();
	// Let's parse the title line : some names will be needed for the unknown signals
	Pattern *patNames = Pattern::compile("([0-9/\\[\\]\\-\\(\\)\\%\\.=:\\|_\\w\\s]+)");
    Matcher *matchNames = patNames->createMatcher(tmpstr);
	int nNames = 0;
	if(matchNames->findFirstMatch())
	{
		do {
			std::string name = matchNames->getGroup(1);
			itemNames.push_back(name);
			itemIsPercent.push_back(strstr(name.c_str(), "%") ? true : false);
			if((int)itemNames.size() == NUM_BASE_VALUES+1)
				if(strcmp("GPU Bottleneck", itemNames[NUM_BASE_VALUES].c_str()))
					bHasAnalysis = false;
		} while(matchNames->findNextMatch());
	}
	if((int)itemNames.size() == NUM_BASE_VALUES)
	{
		bHasSignals = false;
		bHasAnalysis = false;
	}
	else if(bHasAnalysis && (int)itemNames.size() == (NUM_BASE_VALUES+NUM_EXP_EVENTS) )
		bHasSignals = false;
	else 
	{
		startIdxSignals = (bHasAnalysis ? NUM_BASE_VALUES+NUM_EXP_EVENTS : NUM_BASE_VALUES);
		numSignals = (int)itemNames.size() - startIdxSignals;
	}

    IWindowFolding *pWFold, *pWFoldElts, *pWFoldEltsTotal, *pWFoldSignals, *pWFoldRawSignals, *pWFoldSignals2, *pWFoldRawSignals2;
    // FOR NOW: LET's ASSUME that this is always done in display #0 :
    int disp = 0;
	int dispElts = disp + 1;
	int dispEltsTotal = disp + 2;
	int dispSignals = disp + 3;
	int dispRawSignals = disp + 4;
    if((int)g_pDisplays.size() == 0)
    {
        TLDisplay *pDisp = new TLDisplay(g_hwnd);
        pDisp->name = "OGL API Calls Benchmark";
        g_pDisplays.push_back(pDisp);
        TLDisplay *pDispElts = new TLDisplay(g_hwnd);
        pDispElts->name = "OGL Num Elts";
        g_pDisplays.push_back(pDispElts);
        TLDisplay *pDispEltsTotal = new TLDisplay(g_hwnd);
        pDispElts->name = "OGL Total Num Elts";
        g_pDisplays.push_back(pDispEltsTotal);
        TLDisplay *pDispSignals = new TLDisplay(g_hwnd);
        pDispSignals->name = "Instrumentation%";
        g_pDisplays.push_back(pDispSignals);
        TLDisplay *pDispRawSignals = new TLDisplay(g_hwnd);
        pDispSignals->name = "Instrumentation";
        g_pDisplays.push_back(pDispRawSignals);
        //UI:
        (pWFold			= g_pwinHandler->CreateWindowFolding((LPCSTR)(1<<8), "API Calls Benchmark", g_pMainContainer))->UnFold();
        (pWFoldElts		= g_pwinHandler->CreateWindowFolding((LPCSTR)(2<<8), "API Calls Elements", g_pMainContainer))->UnFold();
        (pWFoldEltsTotal= g_pwinHandler->CreateWindowFolding((LPCSTR)(3<<8), "API Calls Elements Total", g_pMainContainer))->UnFold();
        (pWFoldSignals	= g_pwinHandler->CreateWindowFolding((LPCSTR)(4<<8), "Signals", g_pMainContainer))->UnFold();
        (pWFoldRawSignals	= g_pwinHandler->CreateWindowFolding((LPCSTR)(5<<8), "RawSignals", g_pMainContainer))->UnFold();

		firstGraphIdOffset = 0;
		firstGraphRawIdOffset = 0;
    }
    else
    {
        pWFold			= (IWindowFolding *)g_pwinHandler->Get((LPCSTR)(1<<8))->QueryInterface("IWindowFolding");
        pWFoldElts		= (IWindowFolding *)g_pwinHandler->Get((LPCSTR)(2<<8))->QueryInterface("IWindowFolding");
        pWFoldEltsTotal	= (IWindowFolding *)g_pwinHandler->Get((LPCSTR)(3<<8))->QueryInterface("IWindowFolding");
        pWFoldSignals	= (IWindowFolding *)g_pwinHandler->Get((LPCSTR)(4<<8))->QueryInterface("IWindowFolding");
        pWFoldRawSignals	= (IWindowFolding *)g_pwinHandler->Get((LPCSTR)(5<<8))->QueryInterface("IWindowFolding");

		firstGraphIdOffset = (int)g_pDisplays[dispSignals]->Graphs.size();
		firstGraphRawIdOffset = (int)g_pDisplays[dispRawSignals]->Graphs.size();
    }
    int graphTime		= g_pDisplays[disp]->addGraph(name);
    g_pwinHandler->CreateCtrlCheck((LPCSTR)(((disp+1)<<8) + graphTime + 1), name, pWFold)->SetChecked(true);
    int graphElts		= g_pDisplays[dispElts]->addGraph("OGL Elements");
    int graphGLCalls	= g_pDisplays[dispElts]->addGraph("GL calls");
    int graphPoints		= g_pDisplays[dispElts]->addGraph("Points");
    int graphLines		= g_pDisplays[dispElts]->addGraph("Lines");
    int graphTris		= g_pDisplays[dispElts]->addGraph("Triangles");
    int graphQuads		= g_pDisplays[dispElts]->addGraph("Quads");
    g_pwinHandler->CreateCtrlCheck((LPCSTR)(((dispElts+1)<<8) + graphElts + 1), "OGL Elements", pWFoldElts)->SetChecked(true);
    g_pwinHandler->CreateCtrlCheck((LPCSTR)(((dispElts+1)<<8) + graphGLCalls + 1), "GL API Calls", pWFoldElts)->SetChecked(true);
    g_pwinHandler->CreateCtrlCheck((LPCSTR)(((dispElts+1)<<8) + graphPoints + 1), "Points", pWFoldElts)->SetChecked(true);
    g_pwinHandler->CreateCtrlCheck((LPCSTR)(((dispElts+1)<<8) + graphLines + 1), "Lines", pWFoldElts)->SetChecked(true);
    g_pwinHandler->CreateCtrlCheck((LPCSTR)(((dispElts+1)<<8) + graphTris + 1), "Triangles", pWFoldElts)->SetChecked(true);
    g_pwinHandler->CreateCtrlCheck((LPCSTR)(((dispElts+1)<<8) + graphQuads + 1), "Quads", pWFoldElts)->SetChecked(true);

    int graphEltsTotal	= g_pDisplays[dispEltsTotal]->addGraph("OGL Total Elements");
    int graphGLCallsTotal = g_pDisplays[dispEltsTotal]->addGraph("GL calls Total");
    int graphPointsTotal  = g_pDisplays[dispEltsTotal]->addGraph("Points");
    int graphLinesTotal	= g_pDisplays[dispEltsTotal]->addGraph("Lines");
    int graphTrisTotal	= g_pDisplays[dispEltsTotal]->addGraph("Triangles");
    int graphQuadsTotal	= g_pDisplays[dispEltsTotal]->addGraph("Quads");
    g_pwinHandler->CreateCtrlCheck((LPCSTR)(((dispEltsTotal+1)<<8) + graphEltsTotal + 1), "OGL Elements Total", pWFoldElts)->SetChecked(true);
    g_pwinHandler->CreateCtrlCheck((LPCSTR)(((dispEltsTotal+1)<<8) + graphGLCallsTotal + 1), "GL API Calls Total", pWFoldElts)->SetChecked(true);
    g_pwinHandler->CreateCtrlCheck((LPCSTR)(((dispEltsTotal+1)<<8) + graphPointsTotal + 1), "Points Total", pWFoldElts)->SetChecked(true);
    g_pwinHandler->CreateCtrlCheck((LPCSTR)(((dispEltsTotal+1)<<8) + graphLinesTotal + 1), "Lines Total", pWFoldElts)->SetChecked(true);
    g_pwinHandler->CreateCtrlCheck((LPCSTR)(((dispEltsTotal+1)<<8) + graphTrisTotal + 1), "Triangles Total", pWFoldElts)->SetChecked(true);
    g_pwinHandler->CreateCtrlCheck((LPCSTR)(((dispEltsTotal+1)<<8) + graphQuadsTotal + 1), "Quads Total", pWFoldElts)->SetChecked(true);

	pWFoldSignals2	= g_pwinHandler->CreateWindowFolding((LPCSTR)((5+firstGraphIdOffset)<<8), name, pWFoldSignals);
    pWFoldRawSignals2	= g_pwinHandler->CreateWindowFolding((LPCSTR)((6+firstGraphRawIdOffset)<<8), name, pWFoldRawSignals);

	firstGraphId = (int)g_pDisplays[dispSignals]->Graphs.size();
	firstGraphRawId = (int)g_pDisplays[dispRawSignals]->Graphs.size();
	graphSignal = firstGraphId;
	graphRawSignal = firstGraphRawId;
	#define CREATEGRAPH(name, d, g, p)\
	{ g = g_pDisplays[d]->addGraph(name);\
	g_pwinHandler->CreateCtrlCheck((LPCSTR)(((d +1)<<8) + 1 + g++), name, p)->SetChecked(true); }
	if(bHasAnalysis)
		for(int i=0; i<10; i++)
		{
			std::string n = itemNames[NUM_BASE_VALUES+1 + i];
			CREATEGRAPH(n.c_str(), dispSignals, graphSignal, pWFoldSignals2); // 12 : after the "GPU Bottleneck"
		}
	if(bHasSignals)
	{
		for(int i=0; i<numSignals; i++)
		{
			std::string n = itemNames[startIdxSignals + i];
			if(itemIsPercent[startIdxSignals + i])
				CREATEGRAPH(n.c_str(), dispSignals, graphSignal, pWFoldSignals2)
			else
				CREATEGRAPH(n.c_str(), dispRawSignals, graphRawSignal, pWFoldRawSignals2)
		}
	}
    //Create a 3rd graph for time difference...
    //if(bDiffGraph)
    //{
    //    TLDisplay *pDisp = new TLDisplay(g_hwnd);
    //    pDisp->name = "API Calls time difference";
    //    g_pDisplays.push_back(pDisp);
    //    graphDiff = (int)g_pDisplays[g_pDisplays.size()-1]->addGraph(name);
    //}

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
    Pattern::registerPattern("text", "\\s*(.*)\\s*");
	//Pattern *p = Pattern::compile("{int},{float},{float},(\\w*),(\\w*),{int},{int},?{int},?{int},?{int},?{text}");
	// Now we added some PerfSDK signals :
	// there can be an undertermined # of signals to read. And we don't know them
	//Pattern *pInst = Pattern::compile(",?{float}");
	// ,GPU Bottleneck,IDX Bottleneck, IDX SOL,GEOM Bottleneck,GEOM SOL,SHD Bottleneck,SHD SOL,FB Bottleneck,FB SOL,ROP Bottleneck,ROP SOL
	//Pattern *pInst = Pattern::compile(",?([\\w\\s]*),?{int},?{int},?{int},?{int},?{int},?{int},?{int},?{int},?{int},?{int},?{text}"); // we still need to gather the comments at the end
    //Matcher *m = p->createMatcher("");
    //Matcher *mInst = pInst->createMatcher("");
	int benchmarkable_call_number_collapsed = 0;
	int prevFBO = 0;
	int nEltsTotal = 0;
	int nGLCalls_total = 0;
	unsigned int nPointsTotal=0, nLinesTotal=0, nTrisTotal=0, nQuadsTotal=0;
    do {
        int benchmarkable_call_number;
        int nGLCalls;
        float frame_time_ms;
        std::string call_type;
        std::string prim_type;
        unsigned int nElts, curFBO;
		unsigned int nPoints, nLines, nTris, nQuads;
        std::string comment;
		// PerfKit Signals
        int IDX_Bottleneck
			,IDX_SOL
			,GEOM_Bottleneck
			,GEOM_SOL
			,SHD_Bottleneck
			,SHD_SOL
			,FB_Bottleneck
			,FB_SOL
			,ROP_Bottleneck
			,ROP_SOL;
        std::string GPU_Bottleneck;

        fs.getline(tmpstr, 1023);
        if(fs.eof())
            break;
        matchNames->setString(tmpstr);
        if(matchNames->findFirstMatch())
		{
			// We know in advance what to read...
			benchmarkable_call_number = atoi(matchNames->getGroup(1).c_str()); matchNames->findNextMatch();
			nGLCalls  = atoi(matchNames->getGroup(1).c_str()); matchNames->findNextMatch();
			nGLCalls_total += nGLCalls;
			frame_time_ms	= (float)atof(matchNames->getGroup(1).c_str()); matchNames->findNextMatch();
			call_type		= matchNames->getGroup(1); matchNames->findNextMatch();
			prim_type		= matchNames->getGroup(1); matchNames->findNextMatch();
			nElts			= atoi(matchNames->getGroup(1).c_str()); matchNames->findNextMatch();
			nEltsTotal		+= nElts;
			nPoints			= atoi(matchNames->getGroup(1).c_str()); matchNames->findNextMatch();
			nPointsTotal	+= nPoints;
			nLines			= atoi(matchNames->getGroup(1).c_str()); matchNames->findNextMatch();
			nLinesTotal		+= nLines;
			nTris			= atoi(matchNames->getGroup(1).c_str()); matchNames->findNextMatch();
			nTrisTotal		+= nTris;
			nQuads			= atoi(matchNames->getGroup(1).c_str()); matchNames->findNextMatch();
			nQuadsTotal		+= nQuads;
			curFBO			= atoi(matchNames->getGroup(1).c_str()); matchNames->findNextMatch();
			comment			= matchNames->getGroup(1); matchNames->findNextMatch();
			// If we have the bottleneck analysis
			if(bHasAnalysis)
			{
				GPU_Bottleneck	= matchNames->getGroup(1); matchNames->findNextMatch();
				IDX_Bottleneck	= atoi(matchNames->getGroup(1).c_str()); matchNames->findNextMatch();
				IDX_SOL			= atoi(matchNames->getGroup(1).c_str()); matchNames->findNextMatch();
				GEOM_Bottleneck	= atoi(matchNames->getGroup(1).c_str()); matchNames->findNextMatch();
				GEOM_SOL		= atoi(matchNames->getGroup(1).c_str()); matchNames->findNextMatch();
				SHD_Bottleneck	= atoi(matchNames->getGroup(1).c_str()); matchNames->findNextMatch();
				SHD_SOL			= atoi(matchNames->getGroup(1).c_str()); matchNames->findNextMatch();
				FB_Bottleneck	= atoi(matchNames->getGroup(1).c_str()); matchNames->findNextMatch();
				FB_SOL			= atoi(matchNames->getGroup(1).c_str()); matchNames->findNextMatch();
				ROP_Bottleneck	= atoi(matchNames->getGroup(1).c_str()); matchNames->findNextMatch();
				ROP_SOL			= atoi(matchNames->getGroup(1).c_str()); matchNames->findNextMatch();
				comment += std::string("|Bottleneck: ") + GPU_Bottleneck;
			}
			for(int i=0; i< (int)keywordTable.size(); i++)
			{
				if(call_type == keywordTable[i])
				{
					benchmarkable_call_number = -1;
					break;
				}
			}
            if((!fs.eof()) && (benchmarkable_call_number >= 0))// && (frame_time_ms != 0.0f))
            {
				char eltsStr[100];
				eltsStr[0] = '\0';
				if(!prim_type.empty() && (prim_type != std::string(" ")))
					sprintf_s(eltsStr,100,"(%s, %d)",prim_type.c_str(), nElts);
				else if(nElts > 0)
					sprintf_s(eltsStr,100,"(%d)",nElts);
				call_type += std::string(eltsStr);
				if(bCollapse)
					benchmarkable_call_number = benchmarkable_call_number_collapsed;
                //if(bDiffGraph)
                //    g_pDisplays[1]->addMeasure(graphDiff, TMeasure(time_difference, draw_call_number, call_type.c_str(),curFBO), benchmarkable_call_number,curFBO);
                g_pDisplays[disp]->addMeasure(graphTime, TMeasure(frame_time_ms, benchmarkable_call_number, call_type.c_str(),curFBO), benchmarkable_call_number,(float)curFBO);

				g_pDisplays[dispElts]->addMeasure(graphElts, TMeasure((float)nElts, benchmarkable_call_number, call_type.c_str(),curFBO), benchmarkable_call_number);
                g_pDisplays[dispElts]->addMeasure(graphGLCalls, TMeasure((float)nGLCalls, benchmarkable_call_number, call_type.c_str(),curFBO), benchmarkable_call_number);
                g_pDisplays[dispElts]->addMeasure(graphPoints, TMeasure((float)nPoints, benchmarkable_call_number, call_type.c_str(),curFBO), benchmarkable_call_number);
                g_pDisplays[dispElts]->addMeasure(graphLines, TMeasure((float)nLines, benchmarkable_call_number, call_type.c_str(),curFBO), benchmarkable_call_number);
                g_pDisplays[dispElts]->addMeasure(graphTris, TMeasure((float)nTris, benchmarkable_call_number, call_type.c_str(),curFBO), benchmarkable_call_number);
                g_pDisplays[dispElts]->addMeasure(graphQuads, TMeasure((float)nQuads, benchmarkable_call_number, call_type.c_str(),curFBO), benchmarkable_call_number);

				g_pDisplays[dispEltsTotal]->addMeasure(graphEltsTotal, TMeasure((float)nEltsTotal, benchmarkable_call_number, call_type.c_str(),curFBO), benchmarkable_call_number);
                g_pDisplays[dispEltsTotal]->addMeasure(graphGLCallsTotal, TMeasure((float)nGLCalls_total, benchmarkable_call_number, call_type.c_str(),curFBO), benchmarkable_call_number);
                g_pDisplays[dispEltsTotal]->addMeasure(graphPointsTotal, TMeasure((float)nPointsTotal, benchmarkable_call_number, call_type.c_str(),curFBO), benchmarkable_call_number);
                g_pDisplays[dispEltsTotal]->addMeasure(graphLinesTotal, TMeasure((float)nLinesTotal, benchmarkable_call_number, call_type.c_str(),curFBO), benchmarkable_call_number);
                g_pDisplays[dispEltsTotal]->addMeasure(graphTrisTotal, TMeasure((float)nTrisTotal, benchmarkable_call_number, call_type.c_str(),curFBO), benchmarkable_call_number);
                g_pDisplays[dispEltsTotal]->addMeasure(graphQuadsTotal, TMeasure((float)nQuadsTotal, benchmarkable_call_number, call_type.c_str(),curFBO), benchmarkable_call_number);

				int id = firstGraphId  - firstGraphIdOffset;
				int idRaw = firstGraphRawId - firstGraphRawIdOffset;
				if(bHasAnalysis)
				{
					g_pDisplays[dispSignals]->addMeasure(id++, TMeasure((float)IDX_Bottleneck, benchmarkable_call_number, call_type.c_str(),curFBO), benchmarkable_call_number);
					g_pDisplays[dispSignals]->addMeasure(id++, TMeasure((float)IDX_SOL, benchmarkable_call_number, call_type.c_str(),curFBO), benchmarkable_call_number);
					g_pDisplays[dispSignals]->addMeasure(id++, TMeasure((float)GEOM_Bottleneck, benchmarkable_call_number, call_type.c_str(),curFBO), benchmarkable_call_number);
					g_pDisplays[dispSignals]->addMeasure(id++, TMeasure((float)GEOM_SOL, benchmarkable_call_number, call_type.c_str(),curFBO), benchmarkable_call_number);
					g_pDisplays[dispSignals]->addMeasure(id++, TMeasure((float)SHD_Bottleneck, benchmarkable_call_number, call_type.c_str(),curFBO), benchmarkable_call_number);
					g_pDisplays[dispSignals]->addMeasure(id++, TMeasure((float)SHD_SOL, benchmarkable_call_number, call_type.c_str(),curFBO), benchmarkable_call_number);
					g_pDisplays[dispSignals]->addMeasure(id++, TMeasure((float)FB_Bottleneck, benchmarkable_call_number, call_type.c_str(),curFBO), benchmarkable_call_number);
					g_pDisplays[dispSignals]->addMeasure(id++, TMeasure((float)FB_SOL, benchmarkable_call_number, call_type.c_str(),curFBO), benchmarkable_call_number);
					g_pDisplays[dispSignals]->addMeasure(id++, TMeasure((float)ROP_Bottleneck, benchmarkable_call_number, call_type.c_str(),curFBO), benchmarkable_call_number);
					g_pDisplays[dispSignals]->addMeasure(id++, TMeasure((float)ROP_SOL, benchmarkable_call_number, call_type.c_str(),curFBO), benchmarkable_call_number);
				}
				// Now take care of other signals that may have been added
				if(bHasSignals) do {
					float f	= (float)atof(matchNames->getGroup(1).c_str());
					if(id > ((int)g_pDisplays[dispSignals]->Graphs.size()))
					{
						if(g_pLog) g_pLog->AddMessage("error>>Error Too many data in the line !");
						break;
					}
					if(idRaw > ((int)g_pDisplays[dispRawSignals]->Graphs.size()))
					{
						if(g_pLog) g_pLog->AddMessage("error>>Error Too many data in the line !");
						break;
					}
					if(itemIsPercent[(bHasAnalysis?1:0)+NUM_BASE_VALUES+id+idRaw])
						g_pDisplays[dispSignals]->addMeasure(id++, TMeasure((float)f, benchmarkable_call_number, call_type.c_str(),curFBO), benchmarkable_call_number);
					else
						g_pDisplays[dispRawSignals]->addMeasure(idRaw++, TMeasure((float)f, benchmarkable_call_number, call_type.c_str(),curFBO), benchmarkable_call_number);
				} while(matchNames->findNextMatch());

				char FBONum[10];
				FBONum[0] = '\0';
                if(prevFBO != curFBO)
                {
					sprintf_s(FBONum, 10, "FBO=%d", curFBO);
                    g_pDisplays[disp]->setCommentForMeasure(graphTime, benchmarkable_call_number, FBONum, NULL, RGB(128,160,128));
                }
                if(!strncmp(call_type.c_str(), "BlitFramebuffer", 15))
                {
					char blitStr[60];
					sprintf_s(blitStr, 60, "%s to FBO #%d", call_type.c_str(), curFBO);
                    g_pDisplays[disp]->setCommentForMeasure(graphTime, benchmarkable_call_number, blitStr, NULL, RGB(128,160,128));
                }
				prevFBO = curFBO;
                //if(call_type == "Draw")
                //{
                //    g_apiCall.push_back(benchmarkable_call_number);
                //    //assert(((int)g_apiCall.size()-1) == draw_call_number);
                //}
                if(!comment.empty())
                {
                    int i;
                    while((i=(int)comment.find_first_of("|")) >= 0)
                    {
                        comment.replace(i, 1,"\n");
                    }
                    g_pDisplays[disp]->Graphs[graphTime].Measures[benchmarkable_call_number].tooltipComments = comment;
                    g_pDisplays[dispElts]->Graphs[graphElts].Measures[benchmarkable_call_number].tooltipComments = comment;
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
	// Let's do a quick search of possible issues
	g_pDisplays[disp]->searchHighlight("WARNING");
	g_pDisplays[disp]->searchHighlight("ERROR");
    return true;
}
