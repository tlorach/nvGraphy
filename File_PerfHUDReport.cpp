#include "stdafx.h"
//
// Load the perfHUD report:
//
//Draw Call #, GPU Time (ns), Frame Buffer Bottleneck, Frame Buffer Utilized, Geometry Unit Bottleneck, Geometry Unit Utilized, Input Assembly Bottleneck, Input Assembly Utilized, Primitive Count, Raster Operations Bottleneck, Raster Operations Utilized, Shader Bottleneck, Shader Utilized, Texture Bottleneck, Texture Utilized, Shaded Pixel Count, Vertex Instruction %, Geometry Instruction %, Pixel Instruction % 
//0          , 26256        , 22.31%                 , 21.00%               , 20.48%                  , 13.11%                , 14.21%                   , 15.57%                 , 973            , 56.61%                      , 19.17%                    , 23.64%           , 80.07%         , 26.48%            , 26.41%          , 129888            , 4.10%               , 0.00%                 , 95.90%
//
bool loadPerfHUDReport(const char *name, const char *fname)
{
    char tmpstr[1024];
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
    // for size of the file so we can display a progress bar
    fs.seekg( -1, std::ios_base::end );
    int bSize = fs.tellg();
    fs.seekg( 0, std::ios_base::beg );
    g_pProgressBar->SetTitle("Loading perfHUD data...");
	if(g_pLog) g_pLog->AddMessage("Loading PerfHUD data from %s", fname);

	// defined outside
    //Matcher *mm = Pattern::compile("\\\\?([a-zA-Z_]+)\\.")->createMatcher(std::string(fname));
    //mm->findFirstMatch();
    //std::string name = mm->getGroup(1);
    int disp = g_pDisplays.size();
    int disp2 = disp;
    TLDisplay *pDisp = new TLDisplay(g_hwnd);
    pDisp->name = "Bottlenecks";
    g_pDisplays.push_back(pDisp);
    g_pDisplays[disp2]->addGraph("FB bottleneck");
    g_pDisplays[disp2]->addGraph("Geom bottleneck");
    g_pDisplays[disp2]->addGraph("IA bottleneck");
    g_pDisplays[disp2]->addGraph("Raster bottleneck");
    g_pDisplays[disp2]->addGraph("Shader bottleneck");
    g_pDisplays[disp2]->addGraph("Texture bottleneck");
    //UI:
    // NOTE: graphs will have the same 'IDs' : many with ID=(LPCSTR)0, 1, 2...
    // meaning that g_pwinHandler will be confused... but since we don't use it for getting back to the controls... thats ok.
    // Now, these IDs are used to get to the curve ID... instead of using user data (which could be done, though)...
    IWindowFolding *pContainer2 = g_pwinHandler->CreateWindowFolding((LPCSTR)((disp2+1)<<8), "Bottlenecks", g_pMainContainer);
        g_pwinHandler->CreateCtrlCheck((LPCSTR)(((disp2+1)<<8)+1), "FB", pContainer2)->SetChecked(true);
        g_pwinHandler->CreateCtrlCheck((LPCSTR)(((disp2+1)<<8)+2), "Geom", pContainer2)->SetChecked(true);
        g_pwinHandler->CreateCtrlCheck((LPCSTR)(((disp2+1)<<8)+3), "IA", pContainer2)->SetChecked(true);
        g_pwinHandler->CreateCtrlCheck((LPCSTR)(((disp2+1)<<8)+4), "Raster", pContainer2)->SetChecked(true);
        g_pwinHandler->CreateCtrlCheck((LPCSTR)(((disp2+1)<<8)+5), "Shader", pContainer2)->SetChecked(true);
        g_pwinHandler->CreateCtrlCheck((LPCSTR)(((disp2+1)<<8)+6), "Texture", pContainer2)->SetChecked(true);

    disp2++;
    pDisp = new TLDisplay(g_hwnd);
    pDisp->name = "Utilized";
    g_pDisplays.push_back(pDisp);
    g_pDisplays[disp2]->addGraph("FB Utilized");
    g_pDisplays[disp2]->addGraph("Geom Utilized");
    g_pDisplays[disp2]->addGraph("IA Utilized");
    g_pDisplays[disp2]->addGraph("Raster Utilized");
    g_pDisplays[disp2]->addGraph("Shader Utilized");
    g_pDisplays[disp2]->addGraph("Texture Utilized");
    //UI:
    pContainer2 = g_pwinHandler->CreateWindowFolding((LPCSTR)((disp2+1)<<8), "Utilized", g_pMainContainer);
        g_pwinHandler->CreateCtrlCheck((LPCSTR)(((disp2+1)<<8)+1), "FB", pContainer2)->SetChecked(true);
        g_pwinHandler->CreateCtrlCheck((LPCSTR)(((disp2+1)<<8)+2), "Geom", pContainer2)->SetChecked(true);
        g_pwinHandler->CreateCtrlCheck((LPCSTR)(((disp2+1)<<8)+3), "IA", pContainer2)->SetChecked(true);
        g_pwinHandler->CreateCtrlCheck((LPCSTR)(((disp2+1)<<8)+4), "Raster", pContainer2)->SetChecked(true);
        g_pwinHandler->CreateCtrlCheck((LPCSTR)(((disp2+1)<<8)+5), "Shader", pContainer2)->SetChecked(true);
        g_pwinHandler->CreateCtrlCheck((LPCSTR)(((disp2+1)<<8)+6), "Texture", pContainer2)->SetChecked(true);
    pContainer2->UnFold(false);

    disp2++;
    pDisp = new TLDisplay(g_hwnd);
    g_pDisplays.push_back(pDisp);
    g_pDisplays[disp2]->addGraph("prim count");
    //UI:
    pContainer2 = g_pwinHandler->CreateWindowFolding((LPCSTR)((disp2+1)<<8), "Prim Count", g_pMainContainer);
        g_pwinHandler->CreateCtrlCheck((LPCSTR)(((disp2+1)<<8)+1), "FB", pContainer2)->SetChecked(true);
    pContainer2->UnFold(false);
    disp2++;

    g_pDisplays.push_back(new TLDisplay(g_hwnd));
    g_pDisplays[disp2]->addGraph("shaded pixels");
    //UI:
    pContainer2 = g_pwinHandler->CreateWindowFolding((LPCSTR)((disp2+1)<<8), "shaded pixels", g_pMainContainer);
        g_pwinHandler->CreateCtrlCheck((LPCSTR)(((disp2+1)<<8)+1), "FB", pContainer2)->SetChecked(true);
    pContainer2->UnFold(false);
    disp2++;

    pDisp = new TLDisplay(g_hwnd);
    pDisp->name = "GPU Load balance";
    g_pDisplays.push_back(pDisp);
    g_pDisplays[disp2]->addGraph("Vtx");
    g_pDisplays[disp2]->addGraph("Geom");
    g_pDisplays[disp2]->addGraph("Pix");
    //UI:
    pContainer2 = g_pwinHandler->CreateWindowFolding((LPCSTR)((disp2+1)<<8), "Units", g_pMainContainer);
        g_pwinHandler->CreateCtrlCheck((LPCSTR)(((disp2+1)<<8)+1), "Vtx", pContainer2)->SetChecked(true);
        g_pwinHandler->CreateCtrlCheck((LPCSTR)(((disp2+1)<<8)+2), "Geom", pContainer2)->SetChecked(true);
        g_pwinHandler->CreateCtrlCheck((LPCSTR)(((disp2+1)<<8)+3), "Pix", pContainer2)->SetChecked(true);
    pContainer2->UnFold(false);

    //Fill in the graph
    Pattern::registerPattern("int", "\\s*([\\-0-9]*)\\s*");
    Pattern::registerPattern("float", "\\s*([\\-0-9\\.]*)(%*)\\s*");
    Pattern *p = Pattern::compile("{float},*");
    Matcher *m = p->createMatcher("");
    do {
        fs.getline(tmpstr, 1023);
        m->setString(tmpstr);
        if(!m->findFirstMatch())
            continue;
        char p;
        int DrawCall;
        DrawCall = atoi(m->getGroup(1).c_str());
#define GETVALUEi(v)\
        int v;\
        if(m->findNextMatch()) {;\
            v = atoi(m->getGroup(1).c_str());\
            p = m->getGroup(2).c_str()[0];\
            if(p == '%') v /= 100.0f; }
#define GETVALUEf(v)\
        float v;\
        if(m->findNextMatch()) {\
            v = (float)atof(m->getGroup(1).c_str());\
            p = m->getGroup(2).c_str()[0];\
            if(p == '%') v /= 100.0f; }
        GETVALUEf(GPUTimens)
            GPUTimens /= 1000;
        GETVALUEf(FrameBufferBottleneck)
        GETVALUEf(FrameBufferUtilized)
        GETVALUEf(GeometryUnitBottleneck)
        GETVALUEf(GeometryUnitUtilized)
        GETVALUEf(InputAssemblyBottleneck)
        GETVALUEf(InputAssemblyUtilized)
        GETVALUEi(PrimitiveCount)
        GETVALUEf(RasterOperationsBottleneck)
        GETVALUEf(RasterOperationsUtilized)
        GETVALUEf(ShaderBottleneck)
        GETVALUEf(ShaderUtilized)
        GETVALUEf(TextureBottleneck)
        GETVALUEf(TextureUtilized)
        GETVALUEi(ShadedPixelCount)
        GETVALUEf(VertexInstruction)
        if(VertexInstruction < 0) VertexInstruction = 0;
        GETVALUEf(GeometryInstruction)
        if(GeometryInstruction < 0) GeometryInstruction = 0;
        GETVALUEf(PixelInstruction)
        if(PixelInstruction < 0) PixelInstruction = 0;
        if( (!fs.eof()) && (DrawCall >= 0) )
        {
            disp2 = disp;

            if((DrawCall % 200)==0)
                g_pProgressBar->SetPercent(100*fs.tellg()/bSize);

            int frame = g_apiCall.size() > 0 ? g_apiCall[DrawCall] : DrawCall;
            g_pDisplays[disp2]->addMeasure(0, TMeasure(FrameBufferBottleneck, DrawCall, "...", "..", "..."), frame);
            g_pDisplays[disp2]->addMeasure(1, TMeasure(GeometryUnitBottleneck, DrawCall, "...", "..", "..."), frame);
            g_pDisplays[disp2]->addMeasure(2, TMeasure(InputAssemblyBottleneck, DrawCall, "...", "..", "..."), frame);
            g_pDisplays[disp2]->addMeasure(3, TMeasure(RasterOperationsBottleneck, DrawCall, "...", "..", "..."), frame);
            g_pDisplays[disp2]->addMeasure(4, TMeasure(ShaderBottleneck, DrawCall, "...", "..", "..."), frame);
            g_pDisplays[disp2++]->addMeasure(5, TMeasure(TextureBottleneck, DrawCall, "...", "..", "..."), frame);

            g_pDisplays[disp2]->addMeasure(0, TMeasure(FrameBufferUtilized, DrawCall, "...", "..", "..."), frame);
            g_pDisplays[disp2]->addMeasure(1, TMeasure(GeometryUnitUtilized, DrawCall, "...", "..", "..."), frame);
            g_pDisplays[disp2]->addMeasure(2, TMeasure(InputAssemblyUtilized, DrawCall, "...", "..", "..."), frame);
            g_pDisplays[disp2]->addMeasure(3, TMeasure(RasterOperationsUtilized, DrawCall, "...", "..", "..."), frame);
            g_pDisplays[disp2]->addMeasure(4, TMeasure(ShaderUtilized, DrawCall, "...", "..", "..."), frame);
            g_pDisplays[disp2++]->addMeasure(5, TMeasure(TextureUtilized, DrawCall, "...", "..", "..."), frame);

            g_pDisplays[disp2++]->addMeasure(0, TMeasure(PrimitiveCount, DrawCall, "...", "..", "..."), frame);

            g_pDisplays[disp2++]->addMeasure(0, TMeasure(ShadedPixelCount, DrawCall, "...", "..", "..."), frame);

            g_pDisplays[disp2]->addMeasure(0, TMeasure(VertexInstruction+GeometryInstruction+PixelInstruction, DrawCall, "...", "..", "..."), frame);
            g_pDisplays[disp2]->addMeasure(1, TMeasure(GeometryInstruction+PixelInstruction, DrawCall, "...", "..", "..."), frame);
            g_pDisplays[disp2++]->addMeasure(2, TMeasure(PixelInstruction, DrawCall, "...", "..", "..."), frame);
        }
    } while(!fs.eof());
	if(g_pLog) g_pLog->AddMessage("yes>>Done");
    return true;
}
