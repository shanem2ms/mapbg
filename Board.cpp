#include "Board.h"
#include "Application.h"
#include "Engine.h"
#include "HSLColor.h"
#include "SimplexNoise/SimplexNoise.h"
#define NOMINMAX


using namespace gmtl;


static const int boardSizeW = 32;
static const int boardSizeH = 48;
Board::Board() :
	m_width(-1),
	m_height(-1),
	m_currentTool(0)
{	   
    m_cursor = std::make_shared<Cursor>();
}

class BoardAnim1 : public Animation
{
	std::shared_ptr<SceneItem> m_item; 
	float m_growduration;
	float m_totalduration;
	float m_start;

	inline float easeInOutQuad(float x)  {
		return x < 0.5f ? 2 * x * x : 1 - powf(-2 * x + 2, 2) / 2;
	}
public:
	BoardAnim1(std::shared_ptr<SceneItem> item) :
		m_item(item),
		m_growduration(0.25f),
		m_totalduration(1)
	{
		m_start = (rand() / (float)(RAND_MAX)) *
			(m_totalduration - m_growduration * 1.5f);
	}
protected:
	bool Tick(float time) override
	{
		float rtime = std::max(0.0f, std::min(1.0f, (time - m_start) / m_growduration));
		
		rtime = easeInOutQuad(rtime);
		m_item->SetScale(Vec3f(rtime,
			rtime, rtime));
		return time < m_totalduration;
	}
};

class Touch
{	
	Point2f m_touch;
	Point3f m_obj;
	bool m_isInit;
public:
	Touch() : m_isInit(false) {}
	
	bool IsInit() const {
		return m_isInit; }
	void SetInitialPos(const Point2f& mouse, const Point3f& obj)
	{
		m_touch = mouse;
		m_obj = obj;
		m_isInit = true;
	}

    Point3f ObjPoint(const Point2f &newTouchPt) const
	{
		Point2f dpt = (newTouchPt - m_touch);
		return Point3f(dpt[0], dpt[1], 0) + m_obj;
	}
};
//https://shanetest-cos-earth.s3.us-east.cloud-object-storage.appdomain.cloud/usa10m_whqt/Q0/L0/R0/C0
//https://shanetest-cos-earth.s3.us-east.cloud-object-storage.appdomain.cloud/world9m_whqt/Q0/L0/R0/C0
//https://shanetest-cos-earth.s3.us-east.cloud-object-storage.appdomain.cloud/world9m_whqt/Q1/L3/R1/Q1_L3_R1_C0.png

bool cursormode = false;
void Board::TouchDown(float x, float y, int touchId)
{
	if (cursormode)
	{
		m_activeTouch = std::make_shared<Touch>();
	}
	else
	{
		AABoxf bounds = m_boardGroup->GetBounds();
		float xoff = x - bounds.getMin()[0];
		float yoff = y - bounds.getMin()[1];

		float xscl = bounds.getMax()[0] - bounds.getMin()[0];
		float yscl = bounds.getMax()[1] - bounds.getMin()[1];
		float xb = xoff / xscl;
		float yb = yoff / yscl;

		if (xb > 0.9f)
			m_camVel[0] += 1;
		else if (xb < 0.1f)
			m_camVel[0] -= 1;
		if (yb > 0.9f)
			m_camVel[1] += 1;
		else if (yb < 0.1f)
			m_camVel[1] -= 1;
	}
}

void Board::TouchDrag(float x, float y, int touchId)
{
	if (m_activeTouch != nullptr)
	{ 
		if (!m_activeTouch->IsInit())
		{
			m_activeTouch->SetInitialPos(Point2f(x, y), Point3f(m_cursor->Pos()[0], m_cursor->Pos()[1], 0));
		}
		Point3f objpt = m_activeTouch->ObjPoint(Point2f(x, y));
		objpt[0] = std::max(0.0f, std::min(objpt[0], (float)m_width));
		objpt[1] = std::max(0.0f, std::min(objpt[1], (float)m_height));
		m_cursor->SetPos(objpt);
		if (objpt[0] > (m_width - 20))
			m_camPos[0] += 1;
		else if (objpt[0] < 20)
			m_camPos[0] -= 1;
		if (objpt[1] > (m_height - 20))
			m_camPos[1] += 1;
		else if (objpt[1] < 20)
			m_camPos[1] -= 1;
	}
}

void Board::TouchUp(int touchId) 
{
	m_activeTouch = nullptr;
	m_camVel = Point3f(0, 0, 0);
}

void RefreshBiomes(int x, int y, int w, int h, unsigned char* pImgBuf);

void Board::Update(Engine& e, DrawContext & ctx)
{	
	int cx = (int)m_camPos[0];
	int cy = (int)m_camPos[1];
	if (m_boardGroup == nullptr)
	{
		std::shared_ptr<UIButton> btn1 = std::make_shared<UIButton>("Zoom", 100.0f, 10.0f, 100.0f, 50.0f);
		btn1->OnPressed([this](int touchId) {
			m_currentTool = 1;
			});
		Application::Inst().UIMgr().AddControl(btn1);

		std::shared_ptr<UIButton> btn2 = std::make_shared<UIButton>("Move", 250.0f, 10.0f, 100.0f, 50.0f);
		btn2->OnPressed([this](int touchId) {
			m_currentTool = 0;
			});
		Application::Inst().UIMgr().AddControl(btn2);

		m_boardGroup = std::make_shared<SceneGroup>();
		e.Root()->AddItem(m_boardGroup);
	}

	if (m_uiGroup == nullptr)
	{
		m_uiGroup = std::make_shared<SceneGroup>();
		m_cursor->SetPos(Point3f((boardSizeW * m_squareSize) * 0.5f,
			(boardSizeH * m_squareSize) * 0.5f,
			0));
		m_uiGroup->AddItem(m_cursor);
	}
	
	m_boardGroup->Clear();
	{
		float offsetY = (m_height - (boardSizeH * m_squareSize)) * 0.5f;
		float offsetX = (m_width - (boardSizeW * m_squareSize)) * 0.5f;
		m_boardGroup->SetOffset(Point3f(offsetX + -m_camPos[0] * m_squareSize, 
			offsetY + -m_camPos[1] * m_squareSize, m_width / 2));
	}
	m_activeSquares.clear();
	for (int x = 0; x < boardSizeW; ++x)
	{
		for (int y = 0; y < boardSizeH; ++y)
		{
			int wx = cx + x;
			int wy = cy + y;
			Loc l(wx, wy);
			auto itSq = m_squares.find(l);
			if (itSq == m_squares.end())
			{
				std::shared_ptr<Square> sq = std::make_shared<Square>(l);
				{ // Init Square
					float sx = wx * m_squareSize;
					float sy = wy * m_squareSize;
					sq->SetSquareSize(m_squareSize);
					sq->SetOffset(Point3f(sx + m_squareSize / 2, sy + m_squareSize / 2, 0));
					//sq->ProceduralBuild(ctx, simplex, wx, wy);
				}
				m_squares.insert(std::make_pair(l, sq));
				for (int dx = -1; dx <= 1; ++dx) {
					for (int dy = -1; dy <= 1; ++dy)
					{
						auto itNeightborSq = m_squares.find(Loc(wx + dx, wy + dy));
						if (itNeightborSq == m_squares.end())
							continue;
						sq->SetNeighbor(dx, dy, itNeightborSq->second);
						itNeightborSq->second->SetNeighbor(-dx, -dy, sq);
					}
				}
			}
			m_activeSquares.insert(l);
		}
	}


	for (auto sqPair : m_activeSquares)
	{
		auto itSq = m_squares.find(sqPair);
		m_boardGroup->AddItem(itSq->second);
	}

	m_camPos += m_camVel;
}


void Board::Layout(int w, int h)
{
	m_width = w;
	m_height = h;
	m_squareSize = (float)w / (boardSizeW + 1.0f);
    
    m_cursor->SetSquareSize(m_squareSize);
    m_cursor->SetColor(Vec3f(0.5f,6.0f,1));
}

inline void AABoxAdd(AABoxf& aab, const Point3f& pt)
{
	if (aab.isEmpty())
	{
		aab.setEmpty(false);
		aab.setMax(pt);
		aab.setMin(pt);
	}
	else
	{
		const Point3f& min = aab.getMin();
		aab.setMin(Point3f(pt[0] < min[0] ? pt[0] : min[0],
			pt[1] < min[1] ? pt[1] : min[1],
			pt[2] < min[2] ? pt[2] : min[2]));

		const Point3f& max = aab.getMax();
		aab.setMax(Point3f(pt[0] > max[0] ? pt[0] : max[0],
			pt[1] > max[1] ? pt[1] : max[1],
			pt[2] > max[2] ? pt[2] : max[2]));
	}
}

struct Palette
{
	float v;
	unsigned char r;
	unsigned char g;
	unsigned char b;

	Palette(float _V, unsigned char _r, unsigned char _g, unsigned char _b) :
		v(_V),
		r(_r),
		g(_g),
		b(_b)
	{  }
};


Board::Square::Square(const Loc& l) : m_image(-1), m_l(l), m_needRecalc(true)
{
	NoiseGen();
}

void Board::Square::SetNeighbor(int dx, int dy, std::weak_ptr<Square> sq)
{
	m_neighbors[(dy + 1) * 3 + (dx + 1)] = sq;
	if ((dx == -1 && dy == 0) ||
		(dy == -1 && dx == 0))
		m_needRecalc = true;
}


inline float cHiehgt(float n1, float n2) { return (n2 + n1 * 0.5f) * 0.666f; }
const SimplexNoise simplex;

void Board::Square::NoiseGen()
{
	float avgn1 = 0;
	float avgn2 = 0;
	int wx = m_l.m_x;
	int wy = m_l.m_y;
	float nx = wx * 16;
	float ny = wy * 16;
	for (int oy = 0; oy < 16; ++oy)
	{
		for (int ox = 0; ox < 16; ++ox)
		{
			float n1 = simplex.fractal(10, (float)(nx + ox) / (boardSizeW * 16), (float)(ny + oy) / (boardSizeH * 16));
			float n2 = simplex.fractal(5, (float)(nx + ox) / (boardSizeW * 16) * 0.5f, (float)(ny + oy) / (boardSizeH * 16) * 0.5f);
			avgn1 += n1;
			avgn2 += n2;
			m_pts[oy * 16 + ox].height = cHiehgt(n1, n2);
		}
	}
	avgn1 /= 256.0f;
	avgn2 /= 256.0f;
	SetVals(Vec2f(avgn1, avgn2));
}

void Board::Square::Erode()
{
	const float factor = 50.0f;
	for (int oy = 0; oy < 16; ++oy)
	{
		for (int ox = 0; ox < 16; ++ox)
		{
			float xprev = m_pts[oy * 16 + ox].dh[0] * -factor;
			float yprev = m_pts[oy * 16 + ox].dh[1] * -factor;

		}
	}
}

void Board::Square::GradientGen()
{
	for (int oy = 0; oy < 16; ++oy)
	{
		for (int ox = 0; ox < 16; ++ox)
		{
			if (ox == 0)
			{
				if (!m_neighbors[3].expired())
					m_pts[oy * 16 + ox].dh[0] = m_pts[oy * 16 + ox].height - m_neighbors[3].lock()->Pts()[oy * 16 + 15].height;
			}
			else
				m_pts[oy * 16 + ox].dh[0] = m_pts[oy * 16 + ox].height - m_pts[oy * 16 + ox - 1].height;
			if (oy == 0)
			{
				if (!m_neighbors[1].expired())
					m_pts[oy * 16 + ox].dh[1] = m_pts[oy * 16 + ox].height - m_neighbors[1].lock()->Pts()[15 * 16 + ox].height;
			}
			else
				m_pts[oy * 16 + ox].dh[1] = m_pts[oy * 16 + ox].height - m_pts[(oy - 1) * 16 + ox].height;
		}
	}
	m_mindh = Vec2f(1000, 1000);
	m_maxdh = Vec2f(-1000, -1000);
	for (int idx = 0; idx < 256; ++idx)
	{
		m_mindh[0] = std::min(m_mindh[0], m_pts[idx].dh[0]);
		m_mindh[1] = std::min(m_mindh[1], m_pts[idx].dh[1]);
		m_maxdh[0] = std::max(m_maxdh[0], m_pts[idx].dh[0]);
		m_maxdh[1] = std::max(m_maxdh[1], m_pts[idx].dh[1]);
	}
}

void Board::Square::ProceduralBuild(DrawContext & ctx)
{
	GradientGen();
	Erode();
	int wx = m_l.m_x;
	int wy = m_l.m_y;
	float nx = wx * 16;
	float ny = wy * 16;
	std::vector<unsigned char> data(16 * 16 * 4);

	Palette palette[] =
	{
		Palette(-1000, 0, 10, 160),
		Palette(-0.4, 0, 15, 190),
		Palette(-0.2, 0, 0, 255),
		Palette(-0.1, 0, 128, 192),
		Palette(0, 239, 228, 176),
		Palette(0.1, 128, 64, 0),
		Palette(0.2, 0, 128, 0),
		Palette(0.5, 192, 192, 192),
		Palette(0.7, 220, 240, 240),
		Palette(0.8, 240, 255, 255)
	};

	const int pSize = sizeof(palette) / sizeof(Palette);
	
	

	for (int oy = 0; oy < 16; ++oy)
	{
		for (int ox = 0; ox < 16; ++ox)
		{
			float val = m_pts[oy * 16 + ox].height;
			int pIdx = 0;
			for (; palette[pIdx].v < val && pIdx < pSize; ++pIdx);
			pIdx--;
			int offset = (oy * 16 + ox) * 4;
			Vec2f v = m_pts[oy * 16 + ox].dh * 25.0f;
			float d = length(v);
			float diff = std::max(0.0f, 1 - d);
			if (val < 0) diff = 1;
			data[offset] = palette[pIdx].r * diff;
			data[offset + 1] = palette[pIdx].g * diff;
			data[offset + 2] = palette[pIdx].b * diff;
			data[offset + 3] = 255;
		}
	}


	m_needRecalc = false;
}

AABoxf Board::Square::GetBounds() const
{
	const int padding = 2; 
	Matrix44f m = CalcMat()*
		makeScale<Matrix44f>(Vec3f(
			(float)(m_squareSize / 2 - padding), (float)(m_squareSize / 2 - padding), 0));

	Point3f pts[4] = { Point3f(-1, -1, 0),
		Point3f(1, -1, 0) ,
		Point3f(1, 1, 0) ,
			Point3f(-1, -1, 0) };

	AABoxf aab; 	
	for (int idx = 0; idx < 4; ++idx)
	{
		Point3f p1;
		xform(p1, m, Point3f(-1, -1, 0));
		AABoxAdd(aab, p1);
	}
	return aab;
}

void Board::Square::Draw(DrawContext &ctx)
{
	if (m_needRecalc)
		ProceduralBuild(ctx);
	const int padding = 2;
	Matrix44f m =
		ctx.m_mat * CalcMat() *
		makeScale<Matrix44f>(Vec3f(
			(float)(m_squareSize / 2 - padding), (float)(m_squareSize / 2 - padding), (float)(m_squareSize / 2 - padding)));


}

Board::Square::~Square()
{
	if (m_image >= 0)
	{
	}
}

AABoxf Board::Cursor::GetBounds() const
{
	return AABoxf();
}

void Board::Cursor::Draw(DrawContext &ctx)
{
    SetOffset(Point3f(m_pos[0], m_pos[1], 0));
    const int padding = 2;
    Matrix44f m =
        ctx.m_mat * CalcMat() *
        makeScale<Matrix44f>(Vec3f(
            (float)(m_squareSize / 2 - padding), (float)(m_squareSize / 2 - padding), 0));
 
}


Board::~Board()
{

}
