#include "Board.h"
#include "Application.h"
#include "Engine.h"
#include "HSLColor.h"
#include "SimplexNoise/SimplexNoise.h"
#include <numeric>
#include "Mesh.h"
#include "gmtl/PlaneOps.h"
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
        Point2f m_lastDragPos;
        bool m_isInit;
        bool m_isDragging;
    public:

        Camera::Fly m_initCamPos;

        Touch() : m_isInit(false),
            m_isDragging(false) {}

        bool IsInit() const {
            return m_isInit;
        }
        void SetInitialPos(const Point2f& mouse)
        {
            m_touch = mouse;
            m_isInit = true;
        }

        bool IsDragging() const { return m_isDragging; }

        void SetDragPos(const Point2f& newTouchPt)
        {
            Vec2f dpt = (newTouchPt - m_touch);
            if (length(dpt) > 0.04)
                m_isDragging = true;
            m_lastDragPos = newTouchPt;
        }

        const Point2f& LastDragPos() const { return m_lastDragPos; }
        const Point2f& TouchPos() const { return m_touch; }        
    };
    //https://shanetest-cos-earth.s3.us-east.cloud-object-storage.appdomain.cloud/usa10m_whqt/Q0/L0/R0/C0
    //https://shanetest-cos-earth.s3.us-east.cloud-object-storage.appdomain.cloud/world9m_whqt/Q0/L0/R0/C0
    //https://shanetest-cos-earth.s3.us-east.cloud-object-storage.appdomain.cloud/world9m_whqt/Q1/L3/R1/Q1_L3_R1_C0.png

    bool cursormode = false;
    void Board::TouchDown(float x, float y, int touchId)
    {
        m_activeTouch = std::make_shared<Touch>();
        float xb = x / m_width;
        float yb = y / m_height;

        Engine& e = Engine::Inst();
        Camera::Fly la = e.Cam().GetFly();

        m_activeTouch->m_initCamPos = la;
        m_activeTouch->SetInitialPos(Point2f(xb, yb));
    }

    constexpr float pi_over_two = 3.14159265358979323846f * 0.5f;
    void Board::TouchDrag(float x, float y, int touchId)
    {
        if (m_activeTouch != nullptr)
        {
            float xb = x / m_width;
            float yb = y / m_height;

            m_activeTouch->SetDragPos(Point2f(xb, yb));

            Vec2f dragDiff = Point2f(xb, yb) - m_activeTouch->TouchPos();

            Engine& e = Engine::Inst();
            Camera::Fly la = m_activeTouch->m_initCamPos;

            la.dir[0] -= dragDiff[0] * 2;
            la.dir[1] -= dragDiff[1] * 2;
            la.dir[1] = std::max(la.dir[1], -pi_over_two);
            e.Cam().SetFly(la);

        }
    }

    void Board::TouchUp(int touchId)
    {
        if (m_activeTouch != nullptr &&
            !m_activeTouch->IsDragging())
        {
            
            Engine& e = Engine::Inst();
            Matrix44f viewProj = e.Cam().PerspectiveMatrix() * e.Cam().ViewMatrix();
            invert(viewProj);
            Point2f p = m_activeTouch->TouchPos() * 2.0f - Vec2f(1, 1);
            p[1] *= -1;
            Point4f screenPt(p[0], p[1], 0.9f, 1), objpt;
            xform(objpt, viewProj, screenPt);
            objpt /= objpt[3];

            Camera::LookAt la = e.Cam().GetLookat();
            la.pos = Point3f(objpt[0], objpt[1], objpt[2]);
            la.pos[2] = 0.5f;
            la.tilt = std::max(la.tilt + m_tiltVel, 0.0f);
            la.dist *= 0.5f;
            e.Cam().SetLookat(la);

        }
        m_activeTouch = nullptr;
        m_tiltVel = 0;
    }

    const int LeftShift = 160;
    const int SpaceBar = 32;
    const int AButton = 'A';
    const int DButton = 'D';
    const int WButton = 'W';
    const int SButton = 'S';


    void Board::KeyDown(int k)
    {
        float speed = 0.005f;
        switch (k)
        {
        case LeftShift:
            m_camVel[1] -= speed;
            break;
        case SpaceBar:
            m_camVel[1] += speed;
            break;
        case AButton:
            m_camVel[0] -= speed;
            break;
        case DButton:
            m_camVel[0] += speed;
            break;
        case WButton:
            m_camVel[2] -= speed;
            break;
        case SButton:
            m_camVel[2] += speed;
            break;
        }
    }

    void Board::KeyUp(int k)
    {
        switch (k)
        {
        case LeftShift:
        case SpaceBar:
            m_camVel[1] = 0;
            break;
        case AButton:
        case DButton:
            m_camVel[0] = 0;
            break;
        case WButton:
        case SButton:
            m_camVel[2] = 0;
            break;
        }
    }

    void GetFrustumLocs(Camera& cam, std::vector<Loc>& locs);

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

            Camera::Fly fly;
            fly.pos = Point3f(0, 2.0f, 0);
            fly.dir = Vec2f(0, 0.0f);
            e.Cam().SetFly(fly);
        }

        m_boardGroup->Clear();
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

        auto &cam = e.Cam();
        Camera::Fly fly = cam.GetFly();

        Vec3f right, up, forward;
        fly.GetDirs(right, up, forward);
        fly.pos +=
            m_camVel[0] * right +
            m_camVel[1] * up +
            m_camVel[2] * forward;

        int tx = (int)floor(fly.pos[0]);
        int tz = (int)floor(fly.pos[2]);
        Loc camLoc(tx, 0, tz);

        std::vector<Loc> locs;
        GetFrustumLocs(e.Cam(), locs);

        std::sort(locs.begin(), locs.end());
        if (std::find(locs.begin(), locs.end(), camLoc) == locs.end())
        {
            locs.push_back(camLoc);
        }
        struct LocDist
        {
            float d;
            const Loc* loc;
        };
        std::vector<LocDist> locDists;

        Point3f campos = fly.pos;
        for (const Loc& l : locs)
        {
            if (l.m_y != 0)
                continue;

            float sx = l.m_x * m_squareSize;
            float sy = l.m_z * m_squareSize;
            
            Vec2f diffvec = Vec2f(campos[0], campos[1]) - Vec2f(sx, sy);
            float ls = lengthSquared(diffvec);
            locDists.push_back(LocDist { ls, &l });
        }


        std::sort(locDists.begin(), locDists.end(), [](const LocDist& l, const LocDist& r) { return l.d < r.d;  });

        if (locDists.size() > 10)
        {
            locDists.erase(locDists.begin() + 10, locDists.end());
        }

        for (const LocDist& ld : locDists)
        {
            const Loc& l = *ld.loc;

            auto itSq = m_squares.find(l);
            if (itSq == m_squares.end())
            {
                std::shared_ptr<Tile> sq = std::make_shared<Tile>(l);
                { // Init Tile
                    float sx = l.m_x * m_squareSize;
                    float sy = l.m_z * m_squareSize;
                    sq->SetSquareSize(m_squareSize);
                    sq->SetOffset(Point3f(sx + m_squareSize / 2, 0, sy + m_squareSize / 2));
                }
                m_squares.insert(std::make_pair(l, sq));
                for (int dx = -1; dx <= 1; ++dx) {
                    for (int dy = -1; dy <= 1; ++dy)
                    {
                        auto itNeightborSq = m_squares.find(Loc(l.m_x + dx, 0, l.m_z + dy));
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
       
        static float headHeight = 0.04f;
        auto itCamTile = m_squares.find(camLoc);
        if (itCamTile != m_squares.end())
        {
            fly.pos[1] = 
                itCamTile->second->GetGroundHeight(fly.pos) + headHeight;
        }

        cam.SetFly(fly);
    }


    void Board::Layout(int w, int h)
    {
        m_width = w;
        m_height = h;
        m_squareSize = 1.0f;
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
    void GetBBoxes(std::vector<Loc>& locs, AABoxf curbb,
        const Frustumf& f);

    void GetFrustumLocs(Camera& cam, std::vector<Loc>& locs)
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

        viewFrust.mPlanes[viewFrust.PLANE_NEAR];
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

    void GetBBoxes(std::vector<Loc>& locs, AABoxf curbb,
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
                            locs.push_back(Loc((int)x, (int)y, (int)z));
                        }
                    }
                }
            }
            else
            {
                locs.push_back(Loc((int)curbb.mMin[0], (int)curbb.mMin[1], (int)curbb.mMin[2]));
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
                locs.push_back(Loc((int)curbb.mMin[0], (int)curbb.mMin[1], (int)curbb.mMin[2]));
        }
    }
}