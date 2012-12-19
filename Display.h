#ifndef __DISPLAY_H_
#define __DISPLAY_H_

#include "resource.h"       // main symbols
#include "ISvcUI.h"

#define GREENISH 0
#define BLUEISH 2
#define REDDISH 4
#define YELLOW 6
#define GREY 7
#define NUM_COLORS 9

extern IWindowHandler      *g_pwinHandler;
extern IControlString      *g_pCommentEditBox;
extern IControlString      *g_pPixelShaderEditBox;

typedef enum {
	PICK_HIGHLIGHT = 0,
	PICK_SELECT = 1
} PickAction;

struct TMeasure
{
    TMeasure() {
        timing = 0;
        valid =false;
        operation;
        vtxShader;
        pixShader;
        modified = false;
        tagged = 0;
        colorComment = 0;
        drawcall = 0;
        stateID = -1;
		fbo=-1;
    }
    TMeasure(float t, int dc, const char *strop, const char *vtx, const char *pix, int stateid=-1, COLORREF color=0) {
        timing = t;
        valid =true;
        if(strop)   operation = strop;
        if(vtx)     vtxShader = vtx;
        if(pix)     pixShader = pix;
        modified = false;
        tagged = 0;
        colorComment = color;
        drawcall = dc;
        stateID = stateid;
		fbo=-1;
    }
    TMeasure(float t, int dc, const char *dcType, int curFBO/*, int stateid=-1, COLORREF color=0*/) {
        timing = t;
        valid =true;
		if(dcType) 
			drawcallType = dcType;
        modified = false;
        tagged = 0;
        colorComment = 0;//color;
        drawcall = dc;
        stateID = -1;//stateid;
		fbo = curFBO;
    }
    float                   timing;
    int                     drawcall;
	int						fbo;
    bool                    valid;
    bool                    modified;
    int                     tagged;
    int                     stateID;
    std::string operation;
	std::string drawcallType; //in OpenGL we also store the Drawcall primitive type
    std::string vtxShader;
    std::string pixShader;
    std::string tooltipComments;
    std::string userComment;
    std::string userCommentAddons;
    COLORREF colorComment;
};

typedef std::vector<TMeasure> STLMeasures;
struct TGraph
{
    TGraph()
    {
        color = 0;
        valid = true;
        minVal = 1000.0;
        maxVal = -1000.0;
		tempYOffset = 0;
    }
    TGraph(const char * n, int c, STLMeasures m)
    {
        if(n) name = n;
        //Measures = m;
        color = c;
        valid = true;
        minVal = 1000.0;
        maxVal = -1000.0;
		tempYOffset = 0;
    }
    int color;
    bool valid;
    std::string name;
    STLMeasures Measures;
    float minVal, maxVal;
	int   tempYOffset; // temporary offset for when we want to shift the curve for comparison purpose. See TLDisplay::ShiftPickedGraph
};

//typedef std::deque<TBloc> STLBlocs;

typedef std::vector<int> STLLevels;

typedef std::vector<TGraph> STLGraphs;

class TLDisplay
{
public:
    //RESOURCES
	std::string name;
    //-----> BITMAPS
	static HBITMAP hbmpfilmholes;	// area of work
	//-----> ICONS
	static HICON hicon;		// various icons we'll draw
	//-----> BRUSHES
	static HBRUSH hbrush_filmholes;
	static HBRUSH hbrush_middlesepare;
	static HBRUSH hbrush_bgnd;
	static HBRUSH hbrush_filmsepare;
	static HBRUSH hbrush_frames;
	static HBRUSH hbrush_blocs;
	static HBRUSH hbrush_blocs_hightlight;
	static HBRUSH hbrush_fov;
	static HBRUSH hbrush_frametxt;
	static HBRUSH hbrush_colors[NUM_COLORS];
    static HBRUSH hbrush_annot;
    static HBRUSH hbrush_annot_selected;
	//-----> PENS
	static HPEN	hpen_tline;
	static HPEN	hpen_arrows;
	static HPEN	hpen_arrows_sel[4];
	static HPEN	hpen_grid;
	static HPEN    hpen_colors[NUM_COLORS];
	//-----> Cursors
	static HCURSOR hcursor_arrow;
	static HCURSOR hcursor_hand;
	static HCURSOR hcursor_handdn;
	static HCURSOR hcursor_handzoom;
	//-----> Backup of previous stuff (SelectObject())
	HGDIOBJ oldbrush;
	HGDIOBJ oldpen;
	HGDIOBJ oldbmp;
	HGDIOBJ oldfont;
	//-----> FONTS
	HFONT	hfont;
	RECT	rtcode; // size of the timecode TODO: remove...
	//
	//----> Some colors
	//
	static COLORREF TextColor;
	static COLORREF TextBgndColor;
	static COLORREF TextColorTCode;
	static COLORREF TextBgndColorTCode;
	//
	//====> Little hack to make ATI be green'ish and NV be red'ish
	//
	int lastATIColorCandidate, lastNVColorCandidate;

    //
	//====> Objects in the scene
	//
	//STLBlocs	Blocs;
    STLGraphs   Graphs;
	long		NumFrames;
    float       maxVal, minVal;
    struct FoundItem
    {
        FoundItem(int g, int f, int c) {graph = g; frame = f; color=c; }
        int graph;
        int frame;
		int color;
    };
    std::vector<FoundItem> searchResults;
    //
    // Range annotations
    //
    struct RangeAnnotation
    {
        RangeAnnotation(int x1, int x2, float y1, float y2, LPCSTR t, LPCSTR t2)
        { start = x1; end = x2; minY = y1; maxY = y2; if(t) text = t; if(t2) text2 = t2; }
        int start, end;
        float minY, maxY;
        std::string text;
        std::string text2;
    };
    std::vector<RangeAnnotation> rangeAnnotations;
    int selectedRangeAnnot;
	//
	//====> Keep here available X depending on Y
	//
	STLLevels	YLevelsUp;
	STLLevels	YLevelsDn;
	//
	//====> GDI stuff for drawing
	//
	//----> AREA OF WORK
	HBITMAP hbitmap;	// area of work
	int bmpX, bmpY, bmpW, bmpH;			// size of this area
    int percentH;
	HDC bmpDC;		// Compatible Device context used to draw

	//
	//====> Selection & Highlight
	//
	RECT	recthighlighted;
    int     framePicked;
    int     graphPicked;
    int     frameSelected;
    int     graphSelected;
    int     selRangeX[2];
    std::string selStats;
	bool	lockPickedGraph;
	//
	//====> Position stuff
	//
	int		MouseVec[2]; // used during dragging
	float	Offset[2];
	float	ScaleX;
	float	ScaleY;
	float	minScaleX;
	float	minScaleY;
	float	ScaleMouseX; // used during dragging
	float	ScaleMouseY; // used during dragging
    bool    bFirstWheel;
	float	FOV1[2];

	float	xoffs, yoffs; // position de la vue sur la timeline
	float	shifting; // scrolling effect : to slide between Frames
	long	FramePos; // Frame where to start the rendering
	//int		FirstBlockShift; // shift value for the first block in View
	//int		StartBlocNum; // first block Number for rendering the scene
	//STLBlocs::iterator iStartBloc; // first block for rendering the scene

	static int TlineDecoH;
	static int TlineHalfH;
	static int PixBlocW;
	static int PixBlocH;
	int	LevelHeigth;
	int	Margin;

	//
	//====> Some methods for Drawing
	//
            TLDisplay();
            TLDisplay(HWND hwnd, int x, int y, int w, int h, int percent);
            TLDisplay(HWND hwnd, int percent = 100);
            ~TLDisplay();
    void    init(HWND hwnd, int x, int y, int w, int h, int percent);
    static void    createStaticResources(HINSTANCE hInstance);
    static void    releaseStaticResources();
	void	CleanupVars();
	void	AdjustOffsetFromMouse(); // put MouseVec[] in Offset with scale...
	void	SetPos(float x, float y); 
	void	SetPosFromMouse(int dx, int dy); 
	void	ShiftPickedGraph(int dx, int dy);
	void	LockPickedGraph(bool yes);
	void	ScrollPos(int dx, int dy);
	void	ScaleToSelection();
	void	ScaleToRange(int x1=-1, int x2=-1, float y1=-1, float y2=-1);
	void	SetScale(float scaleX, float scaleY);
	void	SetScaleFromMouse(float scaleX, float scaleY); 
	void	SetScaleFromMouseWheel(float scaleX, float scaleY, int mousex, int mousey); 
	void	ResizeBitmap(int w, int h);
    void    blitBitmap(HDC dc, RECT rcPaint);
	int		FindFreeYLevel(int PosX, int Width, int bUp);
	void	DrawScene();
	int		DrawTitle(int x, int y, const char *title, RECT *rc, int pass, COLORREF *bgnd=NULL, COLORREF *txtcolor = NULL);
	int		DrawAnnot(int x, int y, int type, RECT *rc, int pass);
	int		DrawTCode(int x, int y, char *txt, RECT *rc, int pass); // txt == NULL => just a mark
	void	DrawVArrow(int x, int y, int oy);

	void	ClearPickInfo();
    void    ClearSelection();
	int		PickScene(int pickx, int picky, PickAction action);
	int		PickTitle(int x, int y, char *title = NULL); // title == NULL => iconized
	int		PickAnnot(int x, int y, int type);
	int		PickTCode(int x, int y, char *txt = NULL); // txt == NULL => just a mark
    void    SetXSelectionStart(int x);
    void    SetXSelectionEnd(int x);

    void    clearEntries(int graph=-1);
    void    addMeasure(int g, TMeasure &m, int index = -1, float forcedMaxVal = 0.0, float forcedMinVal = 0.0);
    void    setCommentForMeasure(int g, int index, const char *str, const char *str2, COLORREF color=0);
    TMeasure *getSelected(int *graph=NULL, int *frame=NULL);
    int     addGraph(const char *name, int color=-1);
    void    deleteGraphs(int g=-1);
    void    smoothSelectedValue();

    void    searchHighlight(LPCSTR string = NULL);
    void    searchAndTag(LPCSTR string, bool bTag=true);

    void    deleteSelectedRangeAnnotation();
    void    addAnnotationsFromSelection(const char *str, const char *str2, int x1=-1, int x2=-1);
    bool    inBitmap(int x, int y);
    void    update(HWND hwnd);
};

#endif