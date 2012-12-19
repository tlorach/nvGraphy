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
// BlockLine.cpp : Implementation of CBlockLine

#include "stdafx.h"
#include "display.h"
#include "nvGraphy.h"

//RESOURCES
int TLDisplay::TlineDecoH = 4;
int TLDisplay::TlineHalfH = 16;
int TLDisplay::PixBlocW = 35;
int TLDisplay::PixBlocH = 35;

//-----> BITMAPS
HBITMAP TLDisplay::hbmpfilmholes;	// area of work
//-----> ICONS
HICON TLDisplay::hicon;		// various icons we'll draw
//-----> BRUSHES
HBRUSH TLDisplay::hbrush_filmholes;
HBRUSH TLDisplay::hbrush_middlesepare;
HBRUSH TLDisplay::hbrush_bgnd;
HBRUSH TLDisplay::hbrush_filmsepare;
HBRUSH TLDisplay::hbrush_frames;
HBRUSH TLDisplay::hbrush_blocs;
HBRUSH TLDisplay::hbrush_blocs_hightlight;
HBRUSH TLDisplay::hbrush_fov;
HBRUSH TLDisplay::hbrush_frametxt;
HBRUSH TLDisplay::hbrush_colors[NUM_COLORS];
HBRUSH TLDisplay::hbrush_annot;
HBRUSH TLDisplay::hbrush_annot_selected;
//-----> PENS
HPEN	TLDisplay::hpen_tline;
HPEN	TLDisplay::hpen_arrows;
HPEN	TLDisplay::hpen_arrows_sel[4];
HPEN	TLDisplay::hpen_grid;
HPEN    TLDisplay::hpen_colors[NUM_COLORS];
//-----> Cursors
HCURSOR TLDisplay::hcursor_arrow;
HCURSOR TLDisplay::hcursor_hand;
HCURSOR TLDisplay::hcursor_handdn;
HCURSOR TLDisplay::hcursor_handzoom;
//
//----> Some colors
//
COLORREF TLDisplay::TextColor = RGB(0,0,0);
COLORREF TLDisplay::TextBgndColor = RGB(200,200,250);
COLORREF TLDisplay::TextColorTCode = RGB(0,0,0);
COLORREF TLDisplay::TextBgndColorTCode = RGB(180,180,200);

TLDisplay::TLDisplay()
{
    CleanupVars();
}
TLDisplay::TLDisplay(HWND hwnd, int x, int y, int w, int h, int percent)
{
    CleanupVars();
    init(hwnd, x, y, w, h, percent);
}
TLDisplay::TLDisplay(HWND hwnd, int percent)
{
    CleanupVars();
    init(hwnd, 0,0,0,0, percent);
}
TLDisplay::~TLDisplay()
{
	DeleteObject(TLDisplay::hfont);
    DeleteObject(hbitmap);
    DeleteDC(bmpDC);
}
void    TLDisplay::init(HWND hwnd, int x, int y, int w, int h, int percent)
{
	lockPickedGraph = false;
    percentH = percent;
    bmpX = x; bmpY = y;
    HDC hdc = GetDC(hwnd);
    bmpDC = CreateCompatibleDC(hdc);
    ReleaseDC(hwnd, hdc);
    DrawText(bmpDC,"9999", 4,&rtcode,DT_CALCRECT|DT_LEFT);
    TLDisplay::PixBlocW = rtcode.right;
    TLDisplay::TlineHalfH = rtcode.bottom/2 + 1;
    //
    //----> Font
    //
    hfont = CreateFont(
        -MulDiv(8, GetDeviceCaps(bmpDC, LOGPIXELSY), 72),	// logical height of font 
	    0,	// logical average character width 
	    0,	// angle of escapement 
	    0,	// base-line orientation angle 
	    FW_NORMAL,	// font weight FW_BOLD
	    FALSE,	// italic attribute flag 
	    FALSE,	// underline attribute flag 
	    FALSE,	// strikeout attribute flag 
	    DEFAULT_CHARSET,	// character set identifier 
	    OUT_TT_ONLY_PRECIS,	// output precision 
	    CLIP_DEFAULT_PRECIS,	// clipping precision 
	    PROOF_QUALITY,	// output quality 
	    DEFAULT_PITCH,	// pitch and family 
	    "Courrier"	// pointer to typeface name string 
       );
    ResizeBitmap(w, h);
}
void TLDisplay::releaseStaticResources()
{
	//
	//----> restore defaults
	//
    //TODO : put to TLDisplay
    /*TLDisplay::oldbmp = SelectObject(TLDisplay::bmpDC, TLDisplay::oldbmp);
	TLDisplay::oldpen = SelectObject(TLDisplay::bmpDC, TLDisplay::oldpen);
	TLDisplay::oldbrush = SelectObject(TLDisplay::bmpDC, TLDisplay::oldbrush);
	TLDisplay::oldfont = SelectObject(TLDisplay::bmpDC, TLDisplay::oldfont);*/
	//
	//----> Delete objects
	//
#define DELETEOBJECT(o) { DeleteObject(o); o = NULL; }
    DELETEOBJECT(TLDisplay::hcursor_hand);
	DELETEOBJECT(TLDisplay::hcursor_handdn);
	DELETEOBJECT(TLDisplay::hcursor_handzoom);
	DELETEOBJECT(TLDisplay::hicon);
	DELETEOBJECT(TLDisplay::hbmpfilmholes);
	DELETEOBJECT(TLDisplay::hbrush_filmholes);
	DELETEOBJECT(TLDisplay::hbrush_middlesepare);
	DELETEOBJECT(TLDisplay::hbrush_bgnd);
	DELETEOBJECT(TLDisplay::hbrush_blocs);
	DELETEOBJECT(TLDisplay::hbrush_frames);
	DELETEOBJECT(TLDisplay::hbrush_blocs_hightlight);
	DELETEOBJECT(TLDisplay::hbrush_filmsepare);
	DELETEOBJECT(TLDisplay::hbrush_fov);
	DELETEOBJECT(TLDisplay::hbrush_frametxt);
	DELETEOBJECT(TLDisplay::hpen_tline);
	DELETEOBJECT(TLDisplay::hpen_arrows);
	DELETEOBJECT(TLDisplay::hpen_grid);
    for(int i=0; i<4; i++)
    {
		DELETEOBJECT(TLDisplay::hpen_arrows_sel[i]);
	}
    DELETEOBJECT(TLDisplay::hbrush_annot);
    DELETEOBJECT(TLDisplay::hbrush_annot_selected);

    for(int i=0; i<NUM_COLORS; i++)
    {
	    DELETEOBJECT(TLDisplay::hbrush_colors[i]);
	    DELETEOBJECT(TLDisplay::hpen_colors[i]);
    }
}
void TLDisplay::createStaticResources(HINSTANCE hInstance)
{
    //
	//----> Cursors
	//
	TLDisplay::hcursor_arrow = LoadCursor(NULL, (char *)IDC_ARROW);
	TLDisplay::hcursor_hand = LoadCursor(hInstance, (char *)IDC_HAND);
	TLDisplay::hcursor_handdn = LoadCursor(hInstance, (char *)IDC_HANDDN);
	TLDisplay::hcursor_handzoom = LoadCursor(hInstance, (char *)IDC_HANDZOOM);
	//
	//----> Bitmaps...
	//
	TLDisplay::hbmpfilmholes = LoadBitmap(hInstance, (char*)IDB_FILMHOLES);
	//
	//----> Brushes
	//
	TLDisplay::hbrush_filmholes = CreatePatternBrush(TLDisplay::hbmpfilmholes);
	COLORREF color;
	color = RGB(200,200,200);
	TLDisplay::hbrush_bgnd = CreateSolidBrush(color);
	color = RGB(0,0,0);
	TLDisplay::hbrush_middlesepare = CreateSolidBrush(color);
	color = RGB(0,0,0);
	TLDisplay::hbrush_frametxt = CreateSolidBrush(color);
	color = RGB(128,100,100);
	TLDisplay::hbrush_fov = CreateSolidBrush(color);
	color = RGB(180,180,200);
	TLDisplay::hbrush_blocs = CreateSolidBrush(color);
	color = RGB(200,180,180);
	TLDisplay::hbrush_frames = CreateSolidBrush(color);
	color = RGB(14,103,146);
	TLDisplay::hbrush_blocs_hightlight = CreateSolidBrush(color);
	color = RGB(0,0,0);
	TLDisplay::hbrush_filmsepare = CreateSolidBrush(color);
	color = RGB(200,210,200);
	TLDisplay::hbrush_annot = CreateSolidBrush(color);
	color = RGB(220,220,210);
	TLDisplay::hbrush_annot_selected = CreateSolidBrush(color);
	//
	//----> Pens
	//
	color = RGB(1,10,20);
	TLDisplay::hpen_tline = CreatePen(PS_SOLID, 0, color);
	color = RGB(25,25,0);
	TLDisplay::hpen_arrows = CreatePen(PS_SOLID, 1, color);
	color = RGB(180,180,180);
	TLDisplay::hpen_grid = CreatePen(PS_SOLID, 1, color);
	color = RGB(255,255,50);
	TLDisplay::hpen_arrows_sel[0] = CreatePen(PS_SOLID, 0, color);
	color = RGB(100,255,100);
	TLDisplay::hpen_arrows_sel[1] = CreatePen(PS_SOLID, 0, color);
	color = RGB(100,100,255);
	TLDisplay::hpen_arrows_sel[2] = CreatePen(PS_SOLID, 0, color);
	color = RGB(200,60,200);
	TLDisplay::hpen_arrows_sel[3] = CreatePen(PS_SOLID, 0, color);

    //
    // Brush/Pens in misc colors
	// Let's split them in color trends
    //
	COLORREF colors[] = {
	RGB(80,200,80),		//green
	RGB(200,70,80),		// red
	RGB(247,107,4),		// orange
	RGB(80,140,190),	//blue
	RGB(0,160,160),		//blue2
	RGB(160,160,0),		// yellow
	RGB(140,140,140),	// gray
	RGB(0,0,200),		// blue
	RGB(118,199,120),	//green2
	RGB(21,183,33),		//green3
	RGB(162,182,22)	//green4
	};

	for(int i=0; i< NUM_COLORS; i++)
	{
		TLDisplay::hbrush_colors[i] = CreateSolidBrush(colors[i]);
		TLDisplay::hpen_colors[i] = CreatePen(PS_SOLID, 0, colors[i]);
	}
	//
	//----> Icons
	//
	TLDisplay::hicon = LoadIcon(hInstance, (char*)IDI_ANNOT1);
}
void TLDisplay::CleanupVars()
{
	lastATIColorCandidate = REDDISH;
	lastNVColorCandidate = GREENISH;
    graphSelected = -1;
    frameSelected = -1;
    percentH = 100;
    hfont = NULL;
    rtcode.left = rtcode.bottom = rtcode.top = rtcode.right = 0;
    bmpDC = NULL;
    oldbrush = NULL;
    oldpen = NULL;
    oldbmp = NULL;
    oldfont = NULL;

    selectedRangeAnnot = -1;
    maxVal = minVal = 0;
    bFirstWheel = true;
    ClearPickInfo();
    selRangeX[0] = selRangeX[1] = 0;
	Margin = 10;
	LevelHeigth = 16;

	hbitmap = NULL;
	bmpW = bmpH = 0;
    bmpX = bmpY = 0;

	MouseVec[0] = 0;
	MouseVec[1] = 0;
	Offset[0] = 0;
	Offset[1] = 0;
    minScaleX = minScaleY = 0.00000001f;
	ScaleX = 0.4;
	ScaleY = 0.4;
	ScaleMouseX = 0;
	ScaleMouseY = 0;
	FOV1[0] = 0;
	FOV1[1] = 0;

	NumFrames = 0;

	SetPosFromMouse(0,0);
}
void TLDisplay::ResizeBitmap(int w, int h)
{
	if(hbitmap)
	{
		oldbmp = SelectObject(bmpDC, oldbmp);
		DeleteObject(hbitmap);
	}
	HDC dc = ::GetDC(NULL);
	hbitmap = CreateCompatibleBitmap(dc, w,h);
	bmpW = w;
	bmpH = h;
	::ReleaseDC(NULL, dc);
	oldbmp = SelectObject(bmpDC, hbitmap);
    ClearPickInfo();
}
void TLDisplay::blitBitmap(HDC dc, RECT rcPaint)
{
    if(bmpH <= 0)
        return;
    //TODO: optimize with paint rect
    BitBlt(dc, bmpX,bmpY, bmpW, bmpH, bmpDC, 0,0, SRCCOPY);
}

void TLDisplay::DrawScene()
{
	//STLBlocs::value_type bloc;
	char strpos[200];
	int y, yy, frame, w;
	float x, x2;
	float step = (ScaleX+ScaleMouseX) * (float)PixBlocW;
	float stepY = (ScaleY+ScaleMouseY) * (float)PixBlocH;
	x2 = (int)(step * (xoffs + NumFrames));
	if(x2 > bmpW)
		x2 = bmpW;
	//
	//----> y is the 0 level
	//
	y = bmpH/2;
	y += yoffs;
	//
	//----> Clear the Screen
	//
	SelectObject(bmpDC, hbrush_bgnd);
	Rectangle(bmpDC, 0,0, bmpW, bmpH);
	//
	//----> Cleanup the free Levels
	//
	YLevelsDn.clear();
	YLevelsUp.clear();
    //
    //----> annotations as rectangles
    //
    SelectObject(bmpDC, hpen_arrows_sel[0]);
    for(int i=0; i<rangeAnnotations.size(); i++)
    {
        RangeAnnotation &item = rangeAnnotations[i];
        RECT rc;
	    rc.left = step * (xoffs + item.start);
	    rc.right = step * (xoffs + item.end);
        rc.bottom = y - (int)(item.minY * stepY)+ 20 - TlineHalfH - TlineDecoH;
        rc.top = y - (int)(item.maxY * stepY)- 20 - TlineHalfH - TlineDecoH;
        FillRect(bmpDC, &rc, selectedRangeAnnot == i ? hbrush_annot_selected : hbrush_annot);
    }
    for(int i=0; i<rangeAnnotations.size(); i++)
    {
        RangeAnnotation &item = rangeAnnotations[i];
        RECT rc;
	    rc.left = step * (xoffs + item.start);
	    rc.right = step * (xoffs + item.end);
        rc.bottom = y - (int)(item.minY * stepY)+ 20 - TlineHalfH - TlineDecoH;
        rc.top = y - (int)(item.maxY * stepY)- 20 - TlineHalfH - TlineDecoH;
        //FillRect(bmpDC, &rc, selectedRangeAnnot == i ? hbrush_annot_selected : hbrush_annot);
        FrameRect(bmpDC, &rc, hbrush_blocs_hightlight);
        rc.left-=2; rc.right+=2; rc.top-=2; rc.bottom+=2;
        FrameRect(bmpDC, &rc, hbrush_blocs_hightlight);
        COLORREF c = selectedRangeAnnot == i ? RGB(220,220,210) : RGB(180,180,180);
        DrawTitle(rc.left, 1, item.text.c_str(), NULL, 1, &c);
        MoveToEx(bmpDC, rc.left, y - TlineHalfH - TlineDecoH, NULL);
        LineTo(bmpDC, rc.left, rc.bottom);
    }
	//
	//----> display the FILM style of Timeline
	//
	SelectObject(bmpDC, hbrush_filmholes);
	yy = y-TlineHalfH-TlineDecoH;
	SetBrushOrgEx(bmpDC, 0, yy%TlineDecoH, NULL);
	PatBlt(bmpDC, 0, yy, x2, TlineDecoH, PATCOPY);
	yy = y+TlineHalfH;
	SetBrushOrgEx(bmpDC, 0, yy%TlineDecoH, NULL);
	PatBlt(bmpDC, 0, yy, x2, TlineDecoH, PATCOPY);

	SelectObject(bmpDC, hbrush_frames);
	PatBlt(bmpDC, 0, y-TlineHalfH, x2, 2*TlineHalfH, PATCOPY);

	//
	//----> display legend
	//
	RECT r;
	if(!name.empty())
	{
		r.left = r.right = bmpW/10;
		r.top = r.bottom = 3;
		DrawText(bmpDC,name.c_str(), name.length(),&r,DT_CALCRECT);
		//FillRect(bmpDC, &r, hbrush_colors[graph.color&7]);
		//FrameRect(bmpDC, &r, hbrush_colors[graph.color&7]);
		DrawText(bmpDC,name.c_str(), name.length(),&r,DT_LEFT);        
	}
	int shift = bmpW/8;
	int ngraphs = Graphs.size();
	//if(ngraphs <= 8)
    for(int g=0; g<ngraphs; g++)
    {
		TGraph &graph = Graphs[g];
        if(!graph.valid)
            continue;
		r.left = r.right = shift;
		r.top = r.bottom = 0;
        DrawText(bmpDC,graph.name.c_str(), graph.name.length(),&r,DT_CALCRECT);
		r.top = bmpH - r.bottom - 4;
		r.bottom = bmpH - 4;
		DrawText(bmpDC,graph.name.c_str(), graph.name.length(),&r,DT_LEFT);        
		r.top = bmpH - 3;
		r.bottom = bmpH;
        FillRect(bmpDC, &r, hbrush_colors[graph.color&7]);
        //FrameRect(bmpDC, &r, hbrush_colors[graph.color&7]);
		shift += 3 + r.right - r.left;
    }//legend
	//
	//====> Display the Frames
	//
	SelectObject(bmpDC, hbrush_filmsepare);
	frame = -(int)xoffs;
	x = shifting*step;
	w = step > 8 ? 2 : 1;
	for(; x < (float)bmpW; x+= step, frame++)
	{
		if(frame >= NumFrames)
			break;
		//
		//----> Display the separation
		//
		if(step > 2)
			PatBlt(bmpDC, (int)x, y-TlineHalfH, w, 2*TlineHalfH, PATCOPY);
		//
		//----> Display the Frame
		//
		RECT rc;
		rc = rtcode;
		rc.left = (int)x;
		rc.right = (int)x + step;
		rc.top += y-TlineHalfH;
		rc.bottom += y-TlineHalfH;
		if(step >= 16)
		{
			SetTextColor(bmpDC, TextColor);
			SetBkMode(bmpDC, TRANSPARENT);
			sprintf(strpos," %d", frame);
			DrawText(bmpDC,strpos, strlen(strpos),&rc,DT_LEFT);//DT_CALCRECT);		
		}
		if((framePicked == frame)&&(framePicked>=0)&&(Graphs.size() > 0))
		{
            if(framePicked < Graphs[0].Measures.size())
            {
                TMeasure &measure = Graphs[0].Measures[framePicked];
                if(    (measure.valid)
                    && (measure.drawcall > 0)
                    && (measure.drawcall != frame))
			        sprintf(strpos," %d (call %d)", frame, measure.drawcall);
                else
			        sprintf(strpos," %d ", frame);
            } else
			        sprintf(strpos," %d ", frame);
			DrawTitle(x + (step*0.4f),50,strpos, NULL, 1);
            //
            // draw a vertical line from the base to the max of all the graphs at frame #
            //
            float h=0;
            for(int i=0; i<Graphs.size(); i++)
            {
                if(framePicked >= Graphs[i].Measures.size())
                    break;
                float h2 = Graphs[i].Measures[framePicked].timing;
                if(h2 > h) h = h2;
            }
            h = y - (int)(h * stepY) - TlineHalfH - TlineDecoH;
            MoveToEx(bmpDC, x+(step/2), y - TlineHalfH - TlineDecoH, NULL);
            LineTo(bmpDC, x+(step/2), h);
		}
	}
    //
    //----> vertical Grid display
    //
   	SelectObject(bmpDC, hpen_grid);
    float l = ((float)bmpW/step);
    float e = floor((float)log10f(l)-0.5);
    if(e < 1.0) 
        e = 1.0;
    l = powf(10.0, e);
    e= (fmodf(xoffs,l));
    frame = ((float)((int)(-xoffs / l))+1)*l;
    float dx = l * step;
    float barx = e * step + dx;
    if(l > 0.0f)
      for(; barx < bmpW; barx += dx, frame += l)
      {
        MoveToEx(bmpDC, barx, y, NULL);
        LineTo(bmpDC,barx, 0);
		RECT rc;
		rc = rtcode;
		rc.left += (int)barx;
		rc.right += (int)barx;
		rc.top += y-TlineHalfH;
		rc.bottom += y-TlineHalfH;
		SetTextColor(bmpDC, TextColor);
		SetBkMode(bmpDC, OPAQUE);
		SetBkColor(bmpDC, RGB(200,180,180));
		sprintf(strpos," %d", frame);
		DrawText(bmpDC,strpos, strlen(strpos),&rc,DT_LEFT);//DT_CALCRECT);		
      }
    //
    //----> horizontal Grid display
    //
   	SelectObject(bmpDC, hpen_grid);
    l = ((float)bmpH/stepY);
    e = (float)log10f(l);
    e = floor(e-0.5);
    l = powf(10.0, e);
    e= (fmodf(-yoffs,l));
    float v = l;
    dx = l * stepY;
    barx = y-TlineHalfH - TlineDecoH - dx;
    if(l > 0.0f)
      for(; barx > 0; barx -= dx, v += l)
      {
        MoveToEx(bmpDC, 0, barx, NULL);
        LineTo(bmpDC,bmpW, barx);
		RECT rc;
		rc = rtcode;
		rc.left += 0;
		rc.right += 0;
		rc.top += barx;
		rc.bottom += barx;
		SetTextColor(bmpDC, TextColor);
		SetBkMode(bmpDC, TRANSPARENT);
		sprintf(strpos," %.2f", v);
		DrawText(bmpDC,strpos, strlen(strpos),&rc,DT_LEFT);//DT_CALCRECT);		*/
      }
	//
	//----> display the Graphs
	//

	ngraphs = Graphs.size();
    for(int g=0; g<ngraphs; g++)
    {
        if(!Graphs[g].valid)
            continue;
		int tempYOffset = Graphs[g].tempYOffset; // hack for shifting the curve temporarily
        STLMeasures &Measures = Graphs[g].Measures;
	    frame = -(int)xoffs;
        bool bFirstOne = true;
   	    SelectObject(bmpDC, hpen_colors[Graphs[g].color&7]);
	    x = shifting*step+(step/2);
	    for(; x < (float)bmpW; x+= step, frame++)
	    {
		    if(frame >= NumFrames)
			    break;
            if(frame < 0)
                continue;
            if(frame >= Measures.size())
                break;
            TMeasure &m = Measures[frame];
            if(m.valid)
			{
				if((!m.userComment.empty())||(m.tagged))
				{
					MoveToEx(bmpDC, x, tempYOffset + y - TlineHalfH - TlineDecoH, NULL);
					LineTo(bmpDC, x, tempYOffset + y - (int)(m.timing * stepY) - TlineHalfH - TlineDecoH);
				}
			}
        }
    }//graphs
    //
    //----> searchResults display
    //
    for(int i=0; i<searchResults.size(); i++)
    {
        FoundItem &item = searchResults[i];
	    int x = step * (xoffs + item.frame)+(step/2);
        if((x < 0)||(x >= bmpW))
            continue;
		SelectObject(bmpDC, hpen_arrows_sel[item.color & 3]);
        MoveToEx(bmpDC, x, y - TlineHalfH - TlineDecoH, NULL);
        LineTo(bmpDC, x, y - (int)(Graphs[item.graph].Measures[item.frame].timing * stepY) - TlineHalfH - TlineDecoH);
    }
    //
    //----> rest of the graph
    //
	ngraphs = Graphs.size();
    for(int g=0; g<ngraphs; g++)
    {
        if(!Graphs[g].valid)
            continue;
		int tempYOffset = Graphs[g].tempYOffset; // hack for shifting the curve temporarily
        STLMeasures &Measures = Graphs[g].Measures;
	    frame = -(int)xoffs;
	    x = shifting*step+(step/2);
        bool bFirstOne = true;
   	    SelectObject(bmpDC, hpen_colors[Graphs[g].color&7]);
		int xint=(int)x, xintprev=-11111;
	    for(; x < (float)bmpW; x+= step, frame++)
	    {
		    if(frame >= NumFrames)
			    break;
            if(frame < 0)
                continue;
            if(frame >= Measures.size())
                break;
			xint = (int)x;
            TMeasure &m = Measures[frame];
			/*if(xint == xintprev)
				continue;*/
            if(m.valid)
			{
				if(bFirstOne)
				{
					bFirstOne = false;
					MoveToEx(bmpDC, xint, tempYOffset + y - (int)(m.timing * stepY) - TlineHalfH - TlineDecoH, NULL);
				}
				else
					LineTo(bmpDC, xint, tempYOffset + y - (int)(m.timing * stepY) - TlineHalfH - TlineDecoH);
			}
			xintprev = xint;
        }
	    frame = -(int)xoffs;
	    x = shifting*step+(step/2);
	    for(; x < (float)bmpW; x+= step, frame++)
	    {
		    if(frame >= NumFrames)
			    break;
            if(frame < 0)
                continue;
            if(frame >= Measures.size())
                break;
            TMeasure &m = Measures[frame];
            if(!m.valid)
                continue;
            if(!m.userComment.empty())
            {
                COLORREF c = m.colorComment == 0 ? RGB(200,200,250) : m.colorComment;
                if((frame == framePicked)&&(g == graphPicked))
                    c = RGB(200,250,200);
                else if((frame == frameSelected)&&(g == graphSelected))
                    c = RGB(250,200,200);
                sprintf(strpos, "%d: %s", frame, m.userComment.c_str());
                DrawTitle(x,40, strpos, NULL, 1, &c);
            }
        }
        // Squares
	    if(step > 2)
        {
            int ptsz = step/6;
            if(ptsz < 2) ptsz = 2;
            if(ptsz > 5) ptsz = 5;
	        frame = -(int)xoffs;
	        x = shifting*step+(step/2);
   	        SelectObject(bmpDC, hpen_arrows);
	        for(; x < (float)bmpW; x+= step, frame++)
	        {
                if(Measures.size() <= frame)
                    break;
		        if(frame >= NumFrames)
			        break;
                if(frame < 0)
                    continue;
                TMeasure &m = Measures[frame];
                if(!m.valid)
                    continue;
                RECT r;
		        r.left = x-ptsz;
		        r.top = y - (int)(m.timing * stepY) - TlineHalfH - TlineDecoH - ptsz;
		        r.right = x+ptsz;
		        r.bottom = r.top+ptsz+ptsz;
		        FillRect(bmpDC, &r, hbrush_colors[Graphs[g].color&7]);
                if((frame == frameSelected)&&(g == graphSelected))
                {
                    r.top -= 2;
                    r.bottom += 2;
                    r.left -= 2;
                    r.right += 2;
                    InvertRect(bmpDC, &r);
                }
            }
        }
    }//graphs
    //
    // Tooltips
    //
    if((graphPicked >= 0) && (framePicked >= 0))
    {
        TGraph &graph = Graphs[graphPicked];
        TMeasure &m = graph.Measures[framePicked];
        int	x = (int)(step * (xoffs + framePicked)+(step/2));
        int y = bmpH/2 + yoffs - (int)(m.timing * stepY) - TlineHalfH - TlineDecoH;
        static char tmpstr[512];
        sprintf(tmpstr, "%s = %.3f (1k/x= %.1f)\n",
            graph.name.c_str(),
            m.timing, 1000.0f/m.timing);
		if(!m.drawcallType.empty()) // added for OpenGL Framebenchmark... prim type...
          sprintf(tmpstr, "%s%s\n",
            tmpstr,
            m.drawcallType.c_str());
		if(m.fbo>=0) // added for OpenGL Framebenchmark...
          sprintf(tmpstr, "%sFBO=%d\n",
            tmpstr,
            m.fbo);
		if(!m.operation.empty())
          sprintf(tmpstr, "%s%s\n",
            tmpstr,
            m.operation.c_str());
		if(!m.vtxShader.empty() || !m.pixShader.empty())
          sprintf(tmpstr, "%s\nVtx: %s\nPix: %s",
            tmpstr,
            m.vtxShader.c_str(),
            m.pixShader.c_str());
		if(Graphs.size() <= 4)
        for(int g2=0; g2<Graphs.size(); g2++)
        {
            if(graphPicked == g2) continue;
            if(framePicked >= Graphs[g2].Measures.size())
                continue;
            float t2 = Graphs[g2].Measures[framePicked].timing;
            sprintf(tmpstr, "%s\nDelta vs. %s = %.2f (%.1f%%)", 
                tmpstr, Graphs[g2].name.c_str(), m.timing - t2,
                100.0*(m.timing - t2)/t2);
        }
        if(!m.tooltipComments.empty())
            sprintf(tmpstr, "%s\n----\n%s", tmpstr, m.tooltipComments.c_str());
        int len = strlen(tmpstr);
        RECT r;
        r.left = r.right = x + 30;
        r.top = r.bottom = y;
        DrawText(bmpDC,tmpstr, len,&r,DT_CALCRECT);
        r.left = r.left-1;
        r.top = r.top-1;
        r.right = r.right+1;
        r.bottom = r.bottom+1;
        SetTextColor(bmpDC, TextColor);
        SetBkMode(bmpDC, TRANSPARENT);
        //SetBkColor(bmpDC, TextBgndColor);
        FillRect(bmpDC, &r, hbrush_blocs);
        FrameRect(bmpDC, &r, hbrush_frametxt);
        DrawText(bmpDC,tmpstr, len,&r,DT_LEFT);//DT_CALCRECT);
    }
    //
    //----> Highlight the selected rectangle
    //
    if(recthighlighted.left < recthighlighted.right)
        InvertRect(bmpDC, &recthighlighted);
    //
    //----> Selection mark
    //
    if(selRangeX[0] != selRangeX[1])
    {
        int x1, x2;
        if(selRangeX[0] < selRangeX[1])
        {
            x1 = selRangeX[0];
            x2 = selRangeX[1];
        } else {
            x1 = selRangeX[1];
            x2 = selRangeX[0];
        }
        RECT rSel;
        rSel.left = (int)(step * (xoffs + x1));
        rSel.right = (int)(step * (xoffs + x2 + 1));
        rSel.top = 0;
        rSel.bottom = bmpH;
        InvertRect(bmpDC, &rSel);
        //
        // Do the tooltip for stats on selection now :
        //
        if(!selStats.empty())
        {
            rSel.top = 0;
            x2 = rSel.right;
            if(rSel.left < 0)
                rSel.left = 0;
            DrawText(bmpDC,selStats.c_str(), strlen(selStats.c_str()),&rSel,DT_CALCRECT);
            if(rSel.right < x2)
                rSel.right = x2;
            SetTextColor(bmpDC, TextColor);
            SetBkMode(bmpDC, TRANSPARENT);
            //SetBkColor(bmpDC, TextBgndColor);
            FillRect(bmpDC, &rSel, hbrush_blocs);
            FrameRect(bmpDC, &rSel, hbrush_frametxt);
            DrawText(bmpDC,selStats.c_str(), strlen(selStats.c_str()),&rSel,DT_LEFT);//DT_CALCRECT);
        }
    }

}
void	TLDisplay::AdjustOffsetFromMouse()
{
    if((ScaleX+ScaleMouseX) < minScaleX)
        ScaleMouseX = minScaleX - ScaleX;
	Offset[0] += (float)MouseVec[0]/((float)PixBlocW*(ScaleX+ScaleMouseX));
	if(Offset[0] > 0)
		Offset[0] = 0;
	Offset[1] += (float)MouseVec[1];
	MouseVec[0] = 0;
	MouseVec[1] = 0;

	ScaleX += ScaleMouseX;
assert(ScaleX > 0.0);
	ScaleMouseX = 0;
    ClearPickInfo();
    bFirstWheel = true;
}

void	TLDisplay::SetPos(float px, float py)
{
	xoffs = px;
	yoffs = py;
	/*if(xoffs > 0)
		xoffs = 0;*/
	//
	// shifting & offset for Frames
	//
	FramePos = -(int)xoffs;
	shifting = (fmodf(xoffs,1));
	//
	// shifting & start bloc depending on position
	//
	/*FirstBlockShift = 0;
	iStartBloc = Blocs.begin();
	//
	// Find the first block that could appear to the screen
	//
	int x = 0;
	while(iStartBloc != Blocs.end())
	{
		if((FramePos >= x) && (FramePos <= (x+iStartBloc->size)))
			break; // found
		x += iStartBloc->size;
		iStartBloc++;
	}
	//
	// shifting the bloc with x frame number
	//
	StartBlocNum = x;
	FirstBlockShift = x - FramePos; // should be < 0 !!*/
    ClearPickInfo();
}
void	TLDisplay::ScrollPos(int dx, int dy)
{
	MouseVec[0] += dx;
	MouseVec[1] += dy;
	SetPos(Offset[0] + (MouseVec[0]/((float)PixBlocW*(ScaleX+ScaleMouseX))),
		(Offset[1]+MouseVec[1]));
    ClearPickInfo();
    bFirstWheel = true;
}
/*---------------------------------------------------
 * Shift the picked Graph.
 * this can be nice to put two curves together and see how the diverge...
 */
void	TLDisplay::LockPickedGraph(bool yes)
{
	lockPickedGraph = yes;
}
void	TLDisplay::ShiftPickedGraph(int dx, int dy)
{
	if(graphPicked < 0)
		return;
	TGraph &graph = Graphs[graphPicked];
	graph.tempYOffset = dy;
}
void	TLDisplay::SetPosFromMouse(int dx, int dy)
{
	MouseVec[0] = dx;
	MouseVec[1] = dy;
	SetPos(Offset[0] + (MouseVec[0]/((float)PixBlocW*(ScaleX+ScaleMouseX))),
		(Offset[1]+MouseVec[1]));
    ClearPickInfo();
    bFirstWheel = true;
}
void	TLDisplay::SetScale(float scaleX, float scaleY)
{
	ScaleX = scaleX;
	ScaleY = scaleY;
    ClearPickInfo();
    bFirstWheel = true;
}
void	TLDisplay::SetScaleFromMouseWheel(float scaleX, float scaleY, int mousex, int mousey)
{
    if((scaleX < 0.0)&&bFirstWheel)
    {
        //selRangeX[0] = FramePos;
        //selRangeX[1] = FramePos + (int)((float)bmpW / ((ScaleX+ScaleMouseX) * (float)PixBlocW) - shifting);
        bFirstWheel = false;
    }
    if(scaleX > 0.0)
        bFirstWheel = true;

	float dx = mousex/((float)PixBlocW*(ScaleX+ScaleMouseX));
    //
    //----> Do scaling
    //
	ScaleX += scaleX * ScaleX;
    if(ScaleX < minScaleX)
        ScaleX = minScaleX;
	ScaleY += scaleY * ScaleY;
    if(ScaleY < minScaleY)
        ScaleY = minScaleY;

	float dx2 = mousex/((float)PixBlocW*(ScaleX+ScaleMouseX));
    dx -= dx2;

    Offset[0] = xoffs - dx;
    Offset[1] = yoffs;
	SetPos(xoffs - dx,	yoffs);

    ClearPickInfo();
}
void	TLDisplay::SetScaleFromMouse(float scaleX, float scaleY)
{
	ScaleMouseX = scaleX;
	ScaleMouseY = scaleY;
    ClearPickInfo();
}

void	TLDisplay::DrawVArrow(int x, int y, int oy)
{
	SelectObject(bmpDC, hpen_arrows);
	if(oy > y)
	{
		MoveToEx(bmpDC, x, oy - TlineHalfH - TlineDecoH, NULL);
		LineTo(bmpDC, x, y);
		MoveToEx(bmpDC, x + 4, oy - TlineHalfH - TlineDecoH - 4, NULL);
		LineTo(bmpDC, x, oy - TlineHalfH - TlineDecoH);
		LineTo(bmpDC, x - 5, oy - TlineHalfH - TlineDecoH - 5);
	}
	else
	{
		MoveToEx(bmpDC, x, oy + TlineHalfH + TlineDecoH, NULL);
		LineTo(bmpDC, x, y);
		MoveToEx(bmpDC, x + 4, oy + TlineHalfH + TlineDecoH + 4, NULL);
		LineTo(bmpDC, x, oy + TlineHalfH + TlineDecoH);
		LineTo(bmpDC, x - 5, oy + TlineHalfH + TlineDecoH + 5);
	}
}
int		TLDisplay::DrawTitle(int x, int y, const char *title, RECT *rc, int pass, COLORREF *bgnd, COLORREF *txtcolor)
{
	RECT r = {0,0,1,1};
	RECT r2;
	int oy = yoffs + bmpH/2;
	SelectObject(bmpDC, hfont);
	//
	// if rc exists : only compute the rectangle & display the link
	//
	if((pass == 0) || (!rc))
	{
		DrawText(bmpDC,title, strlen(title),&r,DT_CALCRECT);

		y = FindFreeYLevel(x, r.right + 2, y)*(LevelHeigth+Margin);
		y += oy;
		if(oy > y)
		{
			y -= TlineHalfH + TlineDecoH + Margin;
		}
		else
		{
			y += TlineHalfH + TlineDecoH + Margin;
		}

		r.left += x;
		r.top += y;
		r.right += x;
		r.bottom += y;
		if(rc)
			*rc = r;
		if(oy > r.bottom)
			DrawVArrow(r.left, r.bottom+1, oy);
		else
			DrawVArrow(r.left, r.top-1, oy);
	}
	if((pass > 0) || (!rc)) // display the stuff
	{
		if(rc)
			r = *rc;
		r2.left = r.left-1;
		r2.top = r.top-1;
		r2.right = r.right+1;
		r2.bottom = r.bottom+1;
		FrameRect(bmpDC, &r2, hbrush_frametxt);

		SetTextColor(bmpDC, txtcolor ? *txtcolor : TextColor);
		SetBkMode(bmpDC, OPAQUE);
		SetBkColor(bmpDC, bgnd ? *bgnd : TextBgndColor);
		DrawText(bmpDC,title, strlen(title),&r,DT_LEFT);//DT_CALCRECT);
	}
	return 0;
}
int		TLDisplay::DrawAnnot(int x, int y, int type, RECT *rc, int pass)
{
	int oy = yoffs + bmpH/2;
	SelectObject(bmpDC, hfont);

	//
	// if rc exists : only compute the rectangle & display the link
	//
	if((pass == 0) || (!rc))
	{
		y = FindFreeYLevel(x, 16 + 2, y)*(LevelHeigth+Margin);
		y += oy;
		if(oy > y)
		{
			y -= TlineHalfH + TlineDecoH + Margin;
		}
		else
		{
			y += TlineHalfH + TlineDecoH + Margin;
		}
		if(rc)
		{
			rc->left = x;
			rc->top = y;
			rc->right = x + 16;
			rc->bottom = y + 16;
		}
		if(oy > y+8)
			DrawVArrow(x, y+16, oy);
		else
			DrawVArrow(x, y, oy);
	}
	if((pass > 0) || (!rc)) // display the stuff
	{
		if(rc)
		{
			x = rc->left;
			y = rc->top;
		}
		DrawIcon(bmpDC, x,y, hicon/*[type]*/);
	}
	return 0;
}
int		TLDisplay::FindFreeYLevel(int PosX, int Width, int bUp)
{
	if(bUp < 0)
	{
		for(int i=0; i<YLevelsUp.size(); i++)
		{
			if(YLevelsUp[i] <= PosX) // We found a free place
			{
				YLevelsUp[i] = PosX + Width;
				return -i-1;
			}
		}
		// still no place : create one
		YLevelsUp.push_back(PosX+Width);
		int r = YLevelsUp.size();
		return -r;
	}
	else
	{
		for(int i=0; i<YLevelsDn.size(); i++)
		{
			if(YLevelsDn[i] <= PosX) // We found a free place
			{
				YLevelsDn[i] = PosX + Width;
				return i;
			}
		}
		// still no place : create one
		YLevelsDn.push_back(PosX+Width);
		return YLevelsDn.size()-1;
	}
}
#define IFINBOX(x,y,r)\
	if(   (x >= r.left)\
		&&(x <= r.right)\
		&&(y >= r.top)\
		&&(y <= r.bottom))
#define IFNINBOX(x,y,r) \
	if(   (x < r.left)\
		||(x > r.right)\
		||(y < r.top)\
		||(y > r.bottom))
#define TESTRECT(r)\
		IFINBOX(pickx,picky, r)\
		{\
			recthighlighted = r;\
			bHit++;\
			break;\
		}
bool    TLDisplay::inBitmap(int x, int y)
{
	return (x >= bmpX)
		&&(x <= (bmpX+bmpW))
		&&(y >= bmpY)
		&&(y <= bmpY+bmpH);
}
int		TLDisplay::PickScene(int pickx, int picky, PickAction action)
{
    if(bmpH <= 0)
        return 0;
    ClearPickInfo();
	int bHit = 0;
	int frame;
	float step = (ScaleX+ScaleMouseX) * (float)PixBlocW;
	float stepY = (ScaleY+ScaleMouseY) * (float)PixBlocH;
	float x;
	int y;

	//STLBlocs::value_type bloc;

    recthighlighted.left = 0;
	recthighlighted.right = 0;
	recthighlighted.top = 0;
	recthighlighted.bottom = 0;

    pickx -= bmpX;
    picky -= bmpY;
	y = bmpH/2;
	y += yoffs;
	frame = -(int)xoffs;
	x = shifting*step;
    if(action == PICK_HIGHLIGHT)
	for(; x < (float)bmpW; x+= step, frame++)
	{
		RECT rc;
		rc = rtcode;
		rc.left = (int)x;
		rc.right = (int)x + step;
		rc.top = 0;//+= y-TlineHalfH;
		rc.bottom = bmpH;//+= y-TlineHalfH;
		//IFINBOX(pickx,picky, rc)
        if((pickx >= (int)x)&&(pickx <= (int)x + step))
		{
		    rc.top = rtcode.top + y-TlineHalfH;
		    rc.bottom = rtcode.bottom + y-TlineHalfH;
			recthighlighted = rc;
            framePicked = frame;
			bHit++;
			break;
		}
	}
    /*
	x = shifting*step + FirstBlockShift*step;
	STLBlocs::iterator iBlock = iStartBloc;
	while(x < (float)bmpW)
	{
		if(iBlock == Blocs.end())
			break;
		bloc = *iBlock;
		//
		// Test picking with each part of this bloc
		//
		TESTRECT(bloc.rbloc);
		if((bloc.tcodestr && bloc.bTCodeinwin))
			TESTRECT(bloc.rtcode);
		if(bloc.title)
			TESTRECT(bloc.rtitle);
		if(bloc.annots > 0)
			TESTRECT(bloc.rannots);
		x += step*iBlock->size;
		iBlock++;
	}*/
	//
	//----> display the Graphs
	//
	int ngraphs = Graphs.size();
    for(int g=0; g<ngraphs; g++)
    {
        if(!Graphs[g].valid)
            continue;
		int tempYOffset = Graphs[g].tempYOffset; // hack for shifting the curve temporarily
	    //if(step > 4)
        {
            STLMeasures &Measures = Graphs[g].Measures;
            int ptsz = step/6;
            if(ptsz < 2) ptsz = 2;
            if(ptsz > 5) ptsz = 5;
	        frame = -(int)xoffs;
	        x = shifting*step+(step/2);
   	        SelectObject(bmpDC, hpen_arrows);
	        for(; x < (float)bmpW; x+= step, frame++)
	        {
		        if(frame >= NumFrames)
			        break;
                if(frame < 0)
                    continue;
                if(frame >= Measures.size())
                    break;
                TMeasure &m = Measures[frame];
                if(!m.valid)
                    continue;
                RECT r;
		        r.left = x-ptsz;
		        r.top = tempYOffset + y - (int)(m.timing * stepY) - TlineHalfH - TlineDecoH - ptsz;
		        r.right = x+ptsz;
		        r.bottom = r.top+ptsz+ptsz;
		        IFINBOX(pickx,picky, r)
		        {
		            bHit++;
                    if(action == PICK_SELECT)
                    {
                        if(frameSelected == frame)
                        {
                            frameSelected = -1;
                            if(g_pCommentEditBox) g_pCommentEditBox->SetString("");
                            if(g_pPixelShaderEditBox) g_pPixelShaderEditBox->SetString("");
							if(g_pCommentConsole) g_pCommentConsole->Clear();
                        } else {
                            frameSelected = -1; // to avoid editbox cb to re-enter for update...
                            if(g_pCommentEditBox) g_pCommentEditBox->SetString(m.userComment.c_str());
                            if(g_pPixelShaderEditBox) g_pPixelShaderEditBox->SetString(m.pixShader.c_str());
							if(g_pCommentConsole) 
							{
								g_pCommentConsole->Clear();
								g_pCommentConsole->Printf(m.userCommentAddons.c_str());
							}
                            frameSelected = frame;
                        }
                        graphSelected = g;
                        return 1;
                    } else {
			            recthighlighted = r;
                        framePicked = frame;
                        if((!lockPickedGraph) || (graphPicked < 0)) 
							graphPicked = g;
                    }
			        break;
		        }
            }
        }
    }//graphs
    if((action == PICK_SELECT))
      for(int i=0; i<rangeAnnotations.size(); i++)
      {
        RangeAnnotation &item = rangeAnnotations[i];
        RECT rc;
	    rc.left = step * (xoffs + item.start);
	    rc.right = step * (xoffs + item.end);
        rc.bottom = y - (int)(item.minY * stepY)+ 20 - TlineHalfH - TlineDecoH;
        rc.top = y - (int)(item.maxY * stepY) - 20 - TlineHalfH - TlineDecoH;
		IFINBOX(pickx,picky, rc)
		{
            selectedRangeAnnot = selectedRangeAnnot == i ? -1 : i;
			return 1;
		}
      }
	return bHit;
}
void TLDisplay::ClearPickInfo()
{
    recthighlighted.left = recthighlighted.right = 0;
    framePicked = -1;
	if(!lockPickedGraph)
		graphPicked = -1;
}
void TLDisplay::ClearSelection()
{
    frameSelected = -1;
    graphSelected = -1;
    selectedRangeAnnot = -1;
    selectedRangeAnnot = -1;
}


void    TLDisplay::SetXSelectionStart(int x)
{
    float v = (float)x / ((ScaleX+ScaleMouseX) * (float)PixBlocW) - shifting;
    x = FramePos + (int)v;
    if(x < 0)
        x = 0;
    if(x >= NumFrames)
        x = NumFrames - 1;
    selRangeX[0] = selRangeX[1] = x;
}
void    TLDisplay::SetXSelectionEnd(int x)
{
    float v = (float)x / ((ScaleX+ScaleMouseX) * (float)PixBlocW) - shifting;
    x = FramePos + (int)v;
    if(x >= NumFrames)
        x = NumFrames - 1;
    if(x < 0)
        x = 0;
    selRangeX[1] = x;
    //
    // Compute stats to display in the selection tip
    //
    int x1, x2;
    if(selRangeX[0] < selRangeX[1])
    {
        x1 = selRangeX[0];
        x2 = selRangeX[1];
    } else {
        x1 = selRangeX[1];
        x2 = selRangeX[0];
    }
    static char tmpstr[1024];
    sprintf(tmpstr, "%d selected frames\n", x2 + 1 - x1);
    float averages[2];
	if(Graphs.size() <= 4)
    for(int g=0; g<Graphs.size(); g++)
    {
        TGraph &graph = Graphs[g];
        float average = 0;
        int n = 0;
		if(x1 < graph.Measures.size())
		{
			for(int m=x1; m <= x2; m++)
			{
				if(m >= graph.Measures.size())
					break;
				if(!graph.Measures[m].valid)
					continue;
				average += graph.Measures[m].timing;
				n++;
			}
			average /= (float)n;
			if(g < 2) 
				averages[g] = average;
			if(x2 >= graph.Measures.size())
				x2 = graph.Measures.size()-1;
			if(x1 >= graph.Measures.size())
				x1 = graph.Measures.size()-1;
			sprintf(tmpstr, "%s%s : [%.2f -> %.2f] Ave=%.2f (1k/x=%.1f) growth=%.2f\n", 
				tmpstr, graph.name.c_str(), graph.Measures[x1].timing, graph.Measures[x2].timing, average, 1000.0/average, 
				graph.Measures[x2].timing - graph.Measures[x1].timing);
		}
    }
    //
    // Special case where we display the gap increase between graphs 0 and 1
    //
    if((Graphs.size() == 2) && (x1 < Graphs[0].Measures.size()) && (x1 < Graphs[1].Measures.size()))
    {
        float startGap = Graphs[1].Measures[x1].timing - Graphs[0].Measures[x1].timing;
        float endGap = Graphs[1].Measures[x2].timing - Graphs[0].Measures[x2].timing;
        sprintf(tmpstr, "%saverage Gap= %.2f\n", tmpstr, abs(averages[1] - averages[0]));
        sprintf(tmpstr, "%sGap gradient= %.2f", tmpstr, abs(endGap - startGap));
    }
    selStats = tmpstr;
}

void    TLDisplay::clearEntries(int graph)
{
    int s,e;
    if(graph == -1)
    {
        s = 0;
        e = Graphs.size();
    } else {
        s = graph;
        e = graph+1;
    }
    for(int i=s; i < e; i++)
    {
        Graphs[i].Measures.clear();
        Graphs[i].name = "";
    }
}
void    TLDisplay::addMeasure(int g, TMeasure &m, int index, float forcedMaxVal, float forcedMinVal)
{
    if(index <= 0)
        Graphs[g].Measures.push_back(m);
    else if(index >= (int)Graphs[g].Measures.size())
        Graphs[g].Measures.resize(index+1);
    if(index > 0)
        Graphs[g].Measures[index] = m;
    if(NumFrames < Graphs[g].Measures.size())
    {
        NumFrames = Graphs[g].Measures.size();
    }
	float tmax = m.timing;
	if((forcedMaxVal != 0.0) && (tmax > forcedMaxVal))
		tmax = forcedMaxVal;
	float tmin = m.timing;
	if((forcedMinVal != 0.0) && (tmin < forcedMinVal))
		tmin = forcedMinVal;
    if(maxVal < tmax)
    {
        maxVal = tmax;
    }
    if(minVal > tmin)
    {
        minVal = tmin;
    }
    if(Graphs[g].maxVal < tmax)
    {
        Graphs[g].maxVal = tmax;
    }
    if(Graphs[g].minVal > tmin)
    {
        Graphs[g].minVal = tmin;
    }
}
void    TLDisplay::setCommentForMeasure(int g, int index, const char *str, const char *str2, COLORREF color)
{
    if(index >= Graphs[g].Measures.size())
        return;
    if(selRangeX[0] != selRangeX[1])
        return;
    TMeasure &m = Graphs[g].Measures[index];
    static char tmpstr[400];
    if(str && *str != '\0')
    {
        //sprintf(tmpstr, "%d: %s", index, str);
        m.userComment = str;//tmpstr;
    }
	else m.userComment = "";
    if(str2 && *str2 != '\0')
    {
        m.userCommentAddons = str2;
		if(m.userComment.empty())
			m.userComment = "(*)";
    }
    else m.tagged = 1;
    if(color != 0) m.colorComment = color;
}
TMeasure *TLDisplay::getSelected(int *graph, int *frame)
{
    if(graphSelected < 0)
        return NULL;
    if(frameSelected < 0)
        return NULL;
    if(graph) *graph = graphSelected;
    if(frame) *frame = frameSelected;
    return &(Graphs[graphSelected].Measures[frameSelected]);
}

void    TLDisplay::deleteGraphs(int g)
{
    if(g == -1)
    {
        Graphs.clear();
    }
    STLGraphs::iterator iG = Graphs.begin();
    int i=0;
    while(iG != Graphs.end())
    {
        if(g == i)
        {
            Graphs.erase(iG);
            break;
        }
        ++iG;
        ++i;
    }
}
int     TLDisplay::addGraph(const char *name, int color)
{
    if(color<0)
	{
		if(   (strstr(name, "ATI"))
			||(strstr(name, "ati"))
			||(strstr(name, "R"))
			||(strstr(name, "r"))
			||(strstr(name, "HD"))
			||(strstr(name, "4870"))
			||(strstr(name, "4850"))
			||(strstr(name, "4500"))
			||(strstr(name, "8650"))
			||(strstr(name, "Fire"))
			||(strstr(name, "fire"))
			||(strstr(name, "hd")))
			color = lastATIColorCandidate++;
		else if(   (strstr(name, "NV"))
			||(strstr(name, "nv"))
			||(strstr(name, "G"))
			||(strstr(name, "g")))
			color = lastNVColorCandidate++;
		else
			color = Graphs.size();
	}
    //int a = color >> 8;
    //Graphs.reserve(256+a * 256);
    Graphs.push_back(TGraph(name, color, STLMeasures()));
    return Graphs.size()-1;
}
void	TLDisplay::ScaleToSelection()
{

    int x1, x2;
    if(selRangeX[0] == selRangeX[1])
    {
        if(selectedRangeAnnot >= 0)
        {
            x1 = rangeAnnotations[selectedRangeAnnot].start;
            x2 = rangeAnnotations[selectedRangeAnnot].end;
        } else {
            x1 = 0;
            x2 = NumFrames;
        }
    }
    else if(selRangeX[0] < selRangeX[1])
    {
        x1 = selRangeX[0];
        x2 = selRangeX[1];
    } else {
        x1 = selRangeX[1];
        x2 = selRangeX[0];
    }
	FramePos = x1;
	shifting = 0;
    xoffs = -(float)FramePos;
    ScaleX = (float)bmpW/((float)PixBlocW * (x2+1 - FramePos));
    selRangeX[0] = selRangeX[1] = 0;

	Offset[0] = xoffs;
	if(Offset[0] > 0)
		Offset[0] = 0;
	//
	// shifting & start bloc depending on position
	//
	/*FirstBlockShift = 0;
	iStartBloc = Blocs.begin();
	//
	// Find the first block that could appear to the screen
	//
	int x = 0;
	while(iStartBloc != Blocs.end())
	{
		if((FramePos >= x) && (FramePos <= (x+iStartBloc->size)))
			break; // found
		x += iStartBloc->size;
		iStartBloc++;
	}
	//
	// shifting the bloc with x frame number
	//
	StartBlocNum = x;
	FirstBlockShift = x - FramePos; // should be < 0 !!*/
    ClearPickInfo();
    bFirstWheel = true;
}
void TLDisplay::ScaleToRange(int x1, int x2, float y1, float y2)
{
    static int lastH = 400; //hack, again... for bmpH == 0 case...
    if(x1<0)
    {
        x1 = selectedRangeAnnot >= 0 ? rangeAnnotations[selectedRangeAnnot].start : 0;
    }
    if(x2<0)
    {
        x2 = selectedRangeAnnot >= 0 ? rangeAnnotations[selectedRangeAnnot].end : NumFrames;
    }
    if(y1<0) y1 = minVal;
    if(y2<0) y2 = maxVal;
	FramePos = x1;
	shifting = 0;
    xoffs = -(float)FramePos;
    ScaleX = (float)bmpW/((float)PixBlocW * (x2+1 - FramePos));
    selRangeX[0] = selRangeX[1] = 0;
	minScaleX = ScaleX;

	Offset[0] = xoffs;
	if(Offset[0] > 0)
		Offset[0] = 0;
    ClearPickInfo();
    bFirstWheel = true;

    int h = bmpH > 0 ? bmpH : lastH; //arbitrary choice to avoid 0
    yoffs = 20 * h / 100;
    Offset[1] = yoffs;
    ScaleY = 2.0f * h / (3.0f * (y2-y1) * (float)PixBlocH);
    lastH = h;
}

void    TLDisplay::smoothSelectedValue()
{
    if(graphSelected < 0)
        return;
    if(frameSelected <= 0)
        return;
    TGraph &graph = Graphs[graphSelected];
    graph.Measures[frameSelected].timing = (graph.Measures[frameSelected-1].timing + graph.Measures[frameSelected+1].timing) * 0.5f;
    graph.Measures[frameSelected].modified = true;
    graph.Measures[frameSelected].valid = false;
}

void    TLDisplay::searchHighlight(LPCSTR string)
{
    if(string == NULL)
    {
        searchResults.clear();
        return;
    }
    if(*string == '\0')
        return;
    for(int g=0; g<Graphs.size(); g++)
    {
        TGraph &graph = Graphs[g];
        if(!graph.valid)
            continue;
        for(int m=0; m < graph.Measures.size(); m++)
        {
            TMeasure &measure = graph.Measures[m];
            if(!measure.valid)
                continue;
            if((int)measure.pixShader.find(string) >= 0)
            {
                searchResults.push_back(FoundItem(g, m, 0));
            }
            else if((int)measure.vtxShader.find(string) >= 0)
            {
                searchResults.push_back(FoundItem(g, m, 0));
            }
            else if((int)measure.userComment.find(string) >= 0)
            {
                searchResults.push_back(FoundItem(g, m, 0));
            }
            else if((int)measure.userCommentAddons.find(string) >= 0)
            {
                searchResults.push_back(FoundItem(g, m, 0));
            }
            else if((int)measure.operation.find(string) >= 0)
            {
                searchResults.push_back(FoundItem(g, m, 0));
            }
            else if((int)measure.drawcallType.find(string) >= 0)
            {
                searchResults.push_back(FoundItem(g, m, 0));
            }
            else if((int)measure.tooltipComments.find(string) >= 0)
            {
                searchResults.push_back(FoundItem(g, m, 0));
            }
        }
    }

}

void    TLDisplay::searchAndTag(LPCSTR string, bool bClearTag)
{
    if(string == NULL)
        return;
    if(*string == '\0')
        return;
    for(int g=0; g<Graphs.size(); g++)
    {
        TGraph &graph = Graphs[g];
        if(!graph.valid)
            continue;
        for(int m=0; m < graph.Measures.size(); m++)
        {
            TMeasure &measure = graph.Measures[m];
            if(!measure.valid)
                continue;
			bool bFound = false;
            if((int)measure.pixShader.find(string) >= 0)
				bFound = true;
            else if((int)measure.vtxShader.find(string) >= 0)
				bFound = true;
            else if((int)measure.userComment.find(string) >= 0)
				bFound = true;
            else if((int)measure.userCommentAddons.find(string) >= 0)
				bFound = true;
            else if((int)measure.operation.find(string) >= 0)
				bFound = true;
            else if((int)measure.drawcallType.find(string) >= 0)
				bFound = true;
            else if((int)measure.tooltipComments.find(string) >= 0)
				bFound = true;
			if(bFound)
			{
				setCommentForMeasure(g, m, bClearTag ? NULL : string, NULL, RGB(128,120,128));
			}
        }
    }

}

void    TLDisplay::deleteSelectedRangeAnnotation()
{
    if(selectedRangeAnnot < 0)
        return;
    std::vector<RangeAnnotation>::iterator iA;
    iA = rangeAnnotations.begin();
    for(int i=0; i<selectedRangeAnnot; i++)
        ++iA;
    rangeAnnotations.erase(iA);
    selectedRangeAnnot = -1;
}

void    TLDisplay::addAnnotationsFromSelection(const char *str, const char *str2, int x1, int x2)
{
    if((!str)&&(!str2))
        return;
    if(str && (*str == '\0'))
        return;
    if(x1 < 0)
    {
        if(selRangeX[0] == selRangeX[1])
            return;
        x1 = selRangeX[0];
        x2 = selRangeX[1];
        if(selRangeX[0] > selRangeX[1])
        {
            x1 = selRangeX[1];
            x2 = selRangeX[0];
        }
    }
    // min and max in y
    float y1 = 0.0;
    float y2 = 0.0;
    bool first = true;
    for(int g=0; g<Graphs.size(); g++)
    {
        TGraph &graph = Graphs[g];
        for(int m=x1; (m<=x2) && (m<graph.Measures.size()); m++)
        {
            if(!graph.Measures[m].valid)
                continue;
            float y = graph.Measures[m].timing;
            if(y > y2)
                y2 = y;
            if((y < y1)||(y1 == 0.0))
                y1 = y;
        }
    }
    std::vector<RangeAnnotation>::iterator iR;
    iR = rangeAnnotations.begin();
    while(iR != rangeAnnotations.end())
    {
        if((iR->start == x1)&&(iR->end == x2))
        {
            rangeAnnotations.erase(iR);
            break;
        }
        ++iR;
    }
    rangeAnnotations.push_back(TLDisplay::RangeAnnotation(x1,x2, y1,y2, str, str2));
    selRangeX[0] = selRangeX[1] = 0;
}

void    TLDisplay::update(HWND hwnd)
{
    DrawScene();
    RECT Rect = {bmpX,bmpY, bmpW+bmpX, bmpH+bmpY};
    BOOL bRes = InvalidateRect(hwnd, &Rect, FALSE);
    //UpdateWindow(hwnd);
    SendMessage(hwnd, WM_PAINT, 0, 0);
}
