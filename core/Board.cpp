#include "Board.h"
#include "Application.h"
#include "Engine.h"
#include "HSLColor.h"
#include "SimplexNoise/SimplexNoise.h"
#include <numeric>
#include "Mesh.h"
#define NOMINMAX


using namespace gmtl;

namespace sam
{

	Board::Board() :
		m_width(-1),
		m_height(-1),
		m_currentTool(0)
	{
	}

	class BoardAnim1 : public Animation
	{
		std::shared_ptr<SceneItem> m_item;
		float m_growduration;
		float m_totalduration;
		float m_start;

		inline float easeInOutQuad(float x) {
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
			return m_isInit;
		}
		void SetInitialPos(const Point2f& mouse, const Point3f& obj)
		{
			m_touch = mouse;
			m_obj = obj;
			m_isInit = true;
		}

		Point3f ObjPoint(const Point2f& newTouchPt) const
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
			float xb = x / m_width;
			float yb = y / m_height;

			float speed = m_squareSize / 16.0f;
			if (xb > 0.9f)
				m_camVel[0] += speed;
			else if (xb < 0.1f)
				m_camVel[0] -= speed;
			if (yb > 0.9f)
				m_tiltVel -= 0.01f;
			else if (yb < 0.1f)
				m_tiltVel += 0.01f;
		}
	}

	void Board::TouchDrag(float x, float y, int touchId)
	{
		if (m_activeTouch != nullptr)
		{
			Point3f objpt = m_activeTouch->ObjPoint(Point2f(x, y));
			objpt[0] = std::max(0.0f, std::min(objpt[0], (float)m_width));
			objpt[1] = std::max(0.0f, std::min(objpt[1], (float)m_height));
		}
	}

	void Board::TouchUp(int touchId)
	{
		m_activeTouch = nullptr;
		m_camVel = Point3f(0, 0, 0);
		m_tiltVel = 0;
	}

	void RefreshBiomes(int x, int y, int w, int h, unsigned char* pImgBuf);
	void GetFrustumLocs(Camera& cam, std::vector<Board::Loc>& locs);

	void Board::Update(Engine& e, DrawContext& ctx)
	{
		if (m_boardGroup == nullptr)
		{
			/*
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
			*/
			m_boardGroup = std::make_shared<SceneGroup>();
			e.Root()->AddItem(m_boardGroup);
		}

		m_boardGroup->Clear();
		{
			float offsetY = -0.75f;
			float offsetX = -0.5f;
			//m_boardGroup->SetOffset(Point3f(offsetX  * m_squareSize, 
			//	offsetY * m_squareSize, 0.5f));
		}

		auto oldSquares = m_activeSquares;
		m_activeSquares.clear();

		Matrix44f viewProj = e.Cam().PerspectiveMatrix() * e.Cam().ViewMatrix();
		invert(viewProj);

		Point4f corners[5] =
		{ Point4f(0, 0, 0.5f, 1),
			Point4f(1, 1, 0.5f, 1),
			Point4f(1, -1, 0.5f, 1),
			Point4f(-1, 1, 0.5f, 1),
			Point4f(-1, -1, 0.5f, 1) };
		Point4f c[5];

		float xdist = 0;
		float ydist = 0;
		for (int idx = 0; idx < 5; ++idx)
		{
			xform(c[idx], viewProj, corners[idx]);
			c[idx] /= c[idx][3];
			xdist += fabs(c[idx][0] - c[0][0]);
			ydist += fabs(c[idx][1] - c[0][1]);
		}

		xdist *= 0.25f;
		ydist *= 0.25f;
		float xstart = floorf((c[0][0] - xdist) / m_squareSize);
		float xend = ceilf((c[0][0] + xdist) / m_squareSize);
		float ystart = floorf((c[0][1] - ydist) / m_squareSize);
		float yend = ceilf((c[0][1] + ydist) / m_squareSize);

		std::vector<Board::Loc> locs;
		GetFrustumLocs(e.Cam(), locs);

		std::sort(locs.begin(), locs.end());
		for (const Loc& l : locs)
		{
			if (l.m_z != 0)
				continue;

			auto itSq = m_squares.find(l);
			if (itSq == m_squares.end())
			{
				std::shared_ptr<Square> sq = std::make_shared<Square>(l);
				{ // Init Square
					float sx = l.m_x * m_squareSize;
					float sy = l.m_y * m_squareSize;
					sq->SetSquareSize(m_squareSize);
					sq->SetOffset(Point3f(sx + m_squareSize / 2, sy + m_squareSize / 2, 0));
					//sq->ProceduralBuild(ctx, simplex, wx, wy);
				}
				m_squares.insert(std::make_pair(l, sq));
				for (int dx = -1; dx <= 1; ++dx) {
					for (int dy = -1; dy <= 1; ++dy)
					{
						auto itNeightborSq = m_squares.find(Loc(l.m_x + dx, l.m_y + dy));
						if (itNeightborSq == m_squares.end())
							continue;
						sq->SetNeighbor(dx, dy, itNeightborSq->second);
						itNeightborSq->second->SetNeighbor(-dx, -dy, sq);
					}
				}
			}
			m_activeSquares.insert(l);
		}

		for (auto loc : oldSquares)
		{
			if (m_activeSquares.find(loc) == m_activeSquares.end())
			{
				m_squares[loc]->Decomission();
			}
		}
		for (auto sqPair : m_activeSquares)
		{
			auto itSq = m_squares.find(sqPair);
			m_boardGroup->AddItem(itSq->second);
		}

		auto& cam = e.Cam();
		Camera::LookAt la = cam.GetLookat();
		la.pos += m_camVel;
		la.pos[2] = 0.5f;
		la.tilt = std::max(la.tilt + m_tiltVel, 0.0f);
		la.dist = 1.5f;
		cam.SetLookat(la);
	}


	void Board::Layout(int w, int h)
	{
		m_width = w;
		m_height = h;
		m_squareSize = 1.0f;
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
		float nx = wx * SquarePtsCt;
		float ny = wy * SquarePtsCt;
		float scale = 1 / 256.0f;
		for (int oy = 0; oy < SquarePtsCt; ++oy)
		{
			for (int ox = 0; ox < SquarePtsCt; ++ox)
			{
				float n1 = simplex.fractal(10, (float)(nx + ox) * scale, (float)(ny + oy) * scale);
				float n2 = simplex.fractal(5, (float)(nx + ox) * scale * 0.5f, (float)(ny + oy) * scale * 0.5f);
				avgn1 += n1;
				avgn2 += n2;
				m_pts[oy * SquarePtsCt + ox].height = cHiehgt(n1, n2);
				m_pts[oy * SquarePtsCt + ox].sediment = 0;
			}
		}
		avgn1 /= (float)(SquarePtsCt * SquarePtsCt);
		avgn2 /= (float)(SquarePtsCt * SquarePtsCt);
		SetVals(Vec2f(avgn1, avgn2));
	}


	inline float RandF()
	{
		static const float RM = 1.0f / RAND_MAX;
		return rand() * RM;
	}

	inline float Lerp(float l, float r, float t)
	{
		return l * (1 - t) + r * t;
	}

	const float resolution = 1.0f;
	const float scale = 1.0f;

	float Board::Square::SampleHeight(float x, float y)
	{
		x *= scale;
		y *= scale;

		if (x < 0)
			x = 0;
		if (y < 0)
			y = 0;
		if (x >= (SquarePtsCt - 1))
			x = (SquarePtsCt - 1);
		if (y >= (SquarePtsCt - 1))
			y = (SquarePtsCt - 1);

		int ix = (int)x;
		int iy = (int)y;
		int iyy = iy < (SquarePtsCt - 1) ? iy + 1 : iy;
		int ixx = ix < (SquarePtsCt - 1) ? ix + 1 : ix;

		const Board::SqPt& pt00 = m_pts[iy * SquarePtsCt + ix];
		const Board::SqPt& pt01 = m_pts[iy * SquarePtsCt + ixx];
		const Board::SqPt& pt10 = m_pts[iyy * SquarePtsCt + ix];
		const Board::SqPt& pt11 = m_pts[iyy * SquarePtsCt + ixx];
		float lerpx = x - ix;
		float lerpy = y - iy;

		float hy0 = Lerp(pt00.height, pt01.height, lerpx);
		float hy1 = Lerp(pt10.height, pt11.height, lerpx);
		return Lerp(hy0, hy1, lerpy);
	}

	float terrain = 0.05f;
	Vec3f Board::Square::SampleNormal(float x, float y)
	{
		float doubleRadius = -(resolution + resolution);
		float left = SampleHeight(x - resolution, y);
		float right = SampleHeight(x + resolution, y);

		float top = SampleHeight(x, y - resolution);
		float bottom = SampleHeight(x, y + resolution);

		Vec3f nrm(
			-1 * (right - left),
			-1 * (bottom - top),
			terrain);

		normalize(nrm);
		return nrm;
	}

	void Board::Square::AdjustHeight(float x, float y, float amt)
	{
		amt *= 0.1f;
		x *= scale;
		y *= scale;
		if (x < 0)
			x = 0;
		if (y < 0)
			y = 0;
		if (x >= (SquarePtsCt - 1))
			x = (SquarePtsCt - 1);
		if (y >= (SquarePtsCt - 1))
			y = (SquarePtsCt - 1);

		int ix = (int)x;
		int iy = (int)y;
		int iyy = iy < (SquarePtsCt - 1) ? iy + 1 : iy;
		int ixx = ix < (SquarePtsCt - 1) ? ix + 1 : ix;
		float lerpx = x - ix;
		float lerpy = y - iy;

		float amt00 = amt * (1 - lerpx) * (1 - lerpy);
		Board::SqPt& pt00 = m_pts[iy * SquarePtsCt + ix];
		pt00.height += amt00;
		pt00.sediment += amt00;

		float amt01 = amt * lerpx * (1 - lerpy);
		Board::SqPt& pt01 = m_pts[iy * SquarePtsCt + ixx];
		pt01.height += amt01;
		pt01.sediment += amt01;

		float amt10 = amt * (1 - lerpx) * lerpy;
		Board::SqPt& pt10 = m_pts[iyy * SquarePtsCt + ix];
		pt10.height += amt10;
		pt10.sediment += amt10;

		float amt11 = amt * lerpx * lerpy;
		Board::SqPt& pt11 = m_pts[iyy * SquarePtsCt + ixx];
		pt11.height += amt11;
		pt11.sediment += amt11;
	}



	void Board::Square::TraceBall(float x, float y)
	{
		float erosionRate = 0.04f;
		float depositionRate = 0.03f;
		float speed = 0.15f;
		float friction = 0.7f;
		float radius = 0.8f;
		const int maxIterations = 800;
		float iterationScale = 0.04f;

		float sediment = 0; // The amount of carried sediment
		float xp = x; // The previous X position
		float yp = y; // The previous Y position
		float vx = 0; // The horizontal velocity
		float vy = 0; // The vertical velocity

		for (int i = 0; i < maxIterations; ++i)
		{
			const float ox = (RandF() * 2 - 1) * radius; // The X offset
			const float oy = (RandF() * 2 - 1) * radius; // The Y offset
			float h = SampleHeight(x, y);
			if (h < 0)
				break;
			Vec3f nrm = SampleNormal(x, y);
			if (nrm[2] > 0.99f)
				break;

			// Calculate the deposition and erosion rate
			const float deposit = sediment * depositionRate * nrm[2];
			const float erosion = erosionRate * (1 - nrm[2]) * std::min(1.0f, i * iterationScale);

			// Change the sediment on the place this snowball came from
			AdjustHeight(xp, yp, deposit - erosion);
			sediment += erosion - deposit;

			vx = friction * vx + nrm[0] * speed;
			vy = friction * vy + nrm[1] * speed;
			xp = x;
			yp = y;
			x += vx;
			y += vy;
		}
	};


	void Board::Square::Erode()
	{
		for (int i = 0; i < 5000; ++i)
		{
			TraceBall(RandF() * SquarePtsCt * resolution,
				RandF() * SquarePtsCt * resolution);
		}
	}

	void Board::Square::GradientGen()
	{

	}

	void Board::Square::ProceduralBuild(DrawContext& ctx)
	{
		GradientGen();
		Erode();
		int wx = m_l.m_x;
		int wy = m_l.m_y;
		float nx = wx * SquarePtsCt;
		float ny = wy * SquarePtsCt;

		const bgfx::Memory* m = bgfx::alloc(SquarePtsCt * SquarePtsCt * sizeof(Vec4f));
		Vec4f* flData = (Vec4f*)m->data;

		for (int oy = 0; oy < SquarePtsCt; ++oy)
		{
			for (int ox = 0; ox < SquarePtsCt; ++ox)
			{
				float val = m_pts[oy * SquarePtsCt + ox].height;
				flData[oy * SquarePtsCt + ox] = Vec4f(val, 0, 0, 0);
			}
		}

		for (int i = 0; i < 2; ++i)
		{
			m_tex[i] = bgfx::createTexture2D(
				SquarePtsCt, SquarePtsCt, false,
				1,
				bgfx::TextureFormat::Enum::RGBA32F,
				BGFX_TEXTURE_COMPUTE_WRITE | BGFX_TEXTURE_NONE | BGFX_SAMPLER_POINT,
				i == 0 ? m : nullptr
			);
		}

		m_needRecalc = false;
	}

	AABoxf Board::Square::GetBounds() const
	{
		const int padding = 2;
		Matrix44f m = CalcMat() *
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

	void Board::Square::Draw(DrawContext& ctx)
	{
		if (m_needRecalc)
			ProceduralBuild(ctx);
		bgfx::setTexture(0, ctx.m_texture, m_tex[0]);
		bgfx::setImage(1, m_tex[1], 0, bgfx::Access::Write, bgfx::TextureFormat::RGBA16);
		bgfx::dispatch(0, ctx.m_compute, 16, 16);

		float val = cHiehgt(m_vals[0], m_vals[1]);
		float z = 2;// std::min(0.0f, -val * 4);

		Matrix44f m =
			ctx.m_mat * CalcMat() *
			makeScale<Matrix44f>(Vec3f(
				(float)(m_squareSize / 2), (float)(m_squareSize / 2), (float)(m_squareSize / 2))) *
			gmtl::makeTrans<gmtl::Matrix44f>(Vec3f(0, 0, z));


		bgfx::setTransform(m.getData());
		bgfx::setTexture(0, ctx.m_texture, m_tex[1]);

		Grid::init();

		// Set vertex and index buffer.
		bgfx::setVertexBuffer(0, Grid::vbh);
		bgfx::setIndexBuffer(Grid::ibh);
		uint64_t state = 0
			| BGFX_STATE_WRITE_RGB
			| BGFX_STATE_WRITE_A
			| BGFX_STATE_WRITE_Z
			| BGFX_STATE_DEPTH_TEST_LESS
			| BGFX_STATE_CULL_CCW
			| BGFX_STATE_MSAA;
		// Set render states.
		bgfx::setState(state);
		bgfx::submit(0, ctx.m_pgm);
	}

	void Board::Square::Decomission()
	{
		for (int i = 0; i < 2; ++i)
		{
			bgfx::destroy(m_tex[i]);
			m_tex[i] = BGFX_INVALID_HANDLE;
		}
		m_needRecalc = true;
	}

	Board::Square::~Square()
	{
		if (m_image >= 0)
		{
		}
	}

	Board::~Board()
	{

	}

	enum class ContainmentType
	{
		Disjoint = 0,
		Contains = 1,
		Intersects = 2
	};

	Vec3f FrustumCenter(Matrix44f viewproj);
	void GetBBoxes(std::vector<Board::Loc>& locs, AABoxf curbb,
		const Frustumf& f);

	void GetFrustumLocs(Camera& cam, std::vector<Board::Loc>& locs)
	{
		Frustumf viewFrust = cam.GetFrustum();
		Matrix44f viewproj = cam.PerspectiveMatrix() * cam.ViewMatrix();
		Vec3f ctr = FrustumCenter(viewproj);
		Vec3f startlen(128, 128, 128);

		Vec3f chkpos(floorf(ctr[0]), floorf(ctr[1]), floorf(ctr[2]));
		AABoxf bb;
		bb.mMin = chkpos - startlen;
		bb.mMax = chkpos + startlen;
		GetBBoxes(locs, bb, viewFrust);
	}

	Vec3f FrustumCenter(Matrix44f viewproj)
	{
		Matrix44f mInv;
		invert(viewproj);
		Vec4f inpt4;
		xform(inpt4, mInv, Vec4f(0, 0, 0.5f, 1.0f));
		inpt4 /= inpt4[3];
		return Vec3f(inpt4[0], inpt4[1], inpt4[2]);
	}

	template< class DATA_TYPE >
	DATA_TYPE pdistance(const Plane<DATA_TYPE>& plane, const Point<DATA_TYPE, 3>& pt)
	{
		return (dot(plane.mNorm, static_cast<Vec<DATA_TYPE, 3>>(pt)) + plane.mOffset);
	}


	ContainmentType Contains(Frustumf f, AABoxf box)
	{
		ContainmentType result = ContainmentType::Contains;
		for (int i = 0; i < 6; i++)
		{
			Planef plane = f.mPlanes[i];

			// Approach: http://zach.in.tu-clausthal.de/teaching/cg_literatur/lighthouse3d_view_frustum_culling/index.html

			Point3f positive = Point3f(box.mMin[0], box.mMin[1], box.mMin[2]);
			Point3f negative = Point3f(box.mMax[0], box.mMax[1], box.mMax[2]);

			if (plane.mNorm[0] >= 0)
			{
				positive[0] = box.mMax[0];
				negative[0] = box.mMin[0];
			}
			if (plane.mNorm[1] >= 0)
			{
				positive[1] = box.mMax[1];
				negative[1] = box.mMin[1];
			}
			if (plane.mNorm[2] >= 0)
			{
				positive[2] = box.mMax[2];
				negative[2] = box.mMin[2];
			}

			// If the positive vertex is outside (behind plane), the box is disjoint.
			float positiveDistance = pdistance(plane, positive);
			if (positiveDistance < 0)
			{
				return ContainmentType::Disjoint;
			}

			// If the negative vertex is outside (behind plane), the box is intersecting.
			// Because the above check failed, the positive vertex is in front of the plane,
			// and the negative vertex is behind. Thus, the box is intersecting this plane.
			float negativeDistance = pdistance(plane, negative);
			if (negativeDistance < 0)
			{
				result = ContainmentType::Intersects;
			}
		}

		return result;
	}


	void SplitAABox(AABoxf children[8], const AABoxf box)
	{
		Vec3f mid = (box.mMax + box.mMin) * 0.5f;
		Vec3f s[3] = { box.mMin, mid, box.mMax };
		for (int i = 0; i < 8; ++i)
		{
			int xoff = i & 1;
			int yoff = (i / 2) & 1;
			int zoff = i / 4;

			children[i].mMin[0] = s[xoff][0];
			children[i].mMax[0] = s[xoff + 1][0];
			children[i].mMin[1] = s[yoff][1];
			children[i].mMax[1] = s[yoff + 1][1];
			children[i].mMin[2] = s[zoff][2];
			children[i].mMax[2] = s[zoff + 1][2];
		}
	}

	void GetBBoxes(std::vector<Board::Loc>& locs, AABoxf curbb,
		const Frustumf& f)
	{
		ContainmentType res = Contains(f, curbb);
		if (res == ContainmentType::Contains)
		{
			if ((curbb.mMax[0] - curbb.mMin[0]) > 1)
			{
				for (float x = curbb.mMin[0]; x < curbb.mMax[0]; x += 1.0f)
				{
					for (float y = curbb.mMin[1]; y < curbb.mMax[1]; y += 1.0f)
					{
						for (float z = curbb.mMin[2]; z < curbb.mMax[2]; z += 1.0f)
						{
							locs.push_back(Board::Loc((int)x, (int)y, (int)z));
						}
					}
				}
			}
			else
			{
				locs.push_back(Board::Loc((int)curbb.mMin[0], (int)curbb.mMin[1], (int)curbb.mMin[2]));
			}
		}
		else if (res == ContainmentType::Intersects)
		{
			if ((curbb.mMax[0] - curbb.mMin[0]) > 1)
			{
				AABoxf children[8];
				SplitAABox(children, curbb);
				for (int i = 0; i < 8; ++i)
				{
					GetBBoxes(locs, children[i], f);
				}
			}
			else
				locs.push_back(Board::Loc((int)curbb.mMin[0], (int)curbb.mMin[1], (int)curbb.mMin[2]));
		}
	}
}