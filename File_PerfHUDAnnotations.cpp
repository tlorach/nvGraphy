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
