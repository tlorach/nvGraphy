#include "stdafx.h"
//
// Load perfHUD comments
// formatted as follow:
//RenderingCallCount = 430
//"New Call Range Set": "CopyResource", 1; "Clear ShadowMap", 2;...
//
void loadPHAnnotations(const char * fname, int d, int g)
{
    char tmpstr[1024];
	Pattern *pat;
	Matcher *mm;
    std::ifstream fs(fname);

	if(d >= g_pDisplays.size())
	{
		if(g_pLog) g_pLog->AddMessage("Warning>>Couldn't find the display %d for PerfHUD comments", d);
		return;
	}
	if(g >= g_pDisplays[d]->Graphs.size())
	{
		if(g_pLog) g_pLog->AddMessage("Warning>>Couldn't find the graph %d for PerfHUD comments", g);
		return;
	}

FILE *ffffk = fopen(fname, "r");
if(ffffk) fclose(ffffk);
if(!ffffk)
    return;

	if(!fs.is_open()) //Why the heck this F... fucntion doesn't work, hey ? 
        return;
    if(fs.eof())
        return;
	// "RenderingCallCount = xxx"
	fs.getline(tmpstr, 1023); 
    if(fs.eof()) 
		return;
	// "New Call Range Set":
	fs.getline(tmpstr, 1023, ':'); 
    if(fs.eof()) 
		return;
	pat = Pattern::compile("\\s*\"(.+)\",\\s*([0-9]+)");
	do {
		fs.getline(tmpstr, 1023, ';'); 

		mm = pat->createMatcher(tmpstr);
		if(mm->findFirstMatch())
		{
			std::string comment = mm->getGroup(1);
			std::string n = mm->getGroup(2);
			int i = atoi(n.c_str()) - 1;
            if(i >= g_pDisplays[d]->Graphs[g].Measures.size())
			{
				if(g_pLog) g_pLog->AddMessage("Warning>>PerfHUD comments going beyong the limit of the graph : %d >= %d", i, g_pDisplays[d]->Graphs[g].Measures.size());
                continue;
			}
            if(!comment.empty())
                g_pDisplays[d]->setCommentForMeasure(g, i, comment.c_str(), NULL, RGB(146,162,237));
		}
	} while(!fs.eof());
}
