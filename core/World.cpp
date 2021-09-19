#include "StdIncludes.h"
#include "World.h"
#include "Application.h"
#include "Engine.h"
#include "SimplexNoise/SimplexNoise.h"
#include <numeric>
#include "Mesh.h"
#include "OctTile.h"
#include "gmtl/PlaneOps.h"


#define NOMINMAX


using namespace gmtl;

namespace sam
{
  
    World::World() :
        m_width(-1),
        m_height(-1),
        m_currentTool(0),
        m_gravityVel(0),        
        m_flymode(true)
    {

    }  

    void World::Open(const std::string& path)
    {
        m_level.OpenDb(path);
    }

    class Touch
    {
        Point2f m_touch;
        Point2f m_lastDragPos;
        bool m_isInit;
        bool m_isDragging;
    public:

        Vec2f m_initCamDir;

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
    void World::TouchDown(float x, float y, int touchId)
    {
        m_activeTouch = std::make_shared<Touch>();
        float xb = x / m_width;
        float yb = y / m_height;

        Engine& e = Engine::Inst();
        Camera::Fly la = e.Cam().GetFly();
        
        m_activeTouch->m_initCamDir = la.dir;
        m_activeTouch->SetInitialPos(Point2f(xb, yb));
    }

    constexpr float pi_over_two = 3.14159265358979323846f * 0.5f;
    void World::TouchDrag(float x, float y, int touchId)
    {
        if (m_activeTouch != nullptr)
        {
            float xb = x / m_width;
            float yb = y / m_height;

            m_activeTouch->SetDragPos(Point2f(xb, yb));

            Vec2f dragDiff = Point2f(xb, yb) - m_activeTouch->TouchPos();

            Engine& e = Engine::Inst();
            Camera::Fly la = e.Cam().GetFly();

            la.dir[0] = m_activeTouch->m_initCamDir[0] - dragDiff[0] * 2;
            la.dir[1] = m_activeTouch->m_initCamDir[1] - dragDiff[1] * 2;
            la.dir[1] = std::max(la.dir[1], -pi_over_two);
            e.Cam().SetFly(la);

        }
    }

    void World::TouchUp(int touchId)
    {        
        m_activeTouch = nullptr;
        m_tiltVel = 0;
    }

    const int LeftShift = 16;
    const int SpaceBar = 32;
    const int AButton = 'A';
    const int DButton = 'D';
    const int WButton = 'W';
    const int SButton = 'S';
    const int FButton = 'F';
    bool isPaused = false;

    int g_maxTileLod = 9;
    void World::KeyDown(int k)
    {
        float speed = 0.001f;
        switch (k)
        {
        case 'P':
            isPaused = !isPaused;
            break;
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
        case FButton:
            m_flymode = !m_flymode;
            break;
        }
        if (k >= '1' && k <= '9')
        {
            g_maxTileLod = k - '0';
        }
    }

    void World::KeyUp(int k)
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

    Loc g_hitLoc(0, 0, 0);

    void World::Update(Engine& e, DrawContext& ctx)
    {
        if (m_worldGroup == nullptr)
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
            m_worldGroup = std::make_shared<SceneGroup>();
            e.Root()->AddItem(m_worldGroup);
            m_targetCube = std::make_shared<TargetCube>();
            e.Root()->AddItem(m_targetCube);

            Camera::Fly fly;
            fly.pos = Point3f(0.0f, 0.0f, -0.5f);
            fly.dir = Vec2f(0, 0.0f);
            e.Cam().SetFly(fly);

            m_shader = e.LoadShader("vs_cubes.bin", "fs_cubes.bin");
            m_worldGroup->BeforeDraw([this](DrawContext& ctx) { ctx.m_pgm = m_shader; return true; });

        }


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
        float xstart = floorf((c[0][0] - xdist));
        float xend = ceilf((c[0][0] + xdist));
        float ystart = floorf((c[0][1] - ydist));
        float yend = ceilf((c[0][1] + ydist));

        auto &cam = e.Cam();
        Camera::Fly fly = cam.GetFly();
        const float headheight = 0.01f;
        Vec3f boundsExt(0.01f, 0.01f, 0.01f);
        AABoxf playerbounds(fly.pos - boundsExt, fly.pos + boundsExt);

        if (!isPaused)
        {
            m_worldGroup->Clear();
            m_octTileSelection.Update(e, ctx, playerbounds);
            m_octTileSelection.AddTilesToGroup(m_worldGroup);
        }

        std::shared_ptr<OctTile> tile = m_octTileSelection.TileFromPos(fly.pos);
        float grnd = tile != nullptr ? tile->GetGroundPos(Point2f(fly.pos[0], fly.pos[2])) :
            INFINITY;
        m_octTileSelection.GetNearFarMidDist(ctx.m_nearfar);
        e.Cam().SetNearFar(ctx.m_nearfar[0], ctx.m_nearfar[2]);

        {
            Matrix44f mat0 = cam.GetPerspectiveMatrix(ctx.m_nearfar[0], ctx.m_nearfar[1]) *
                cam.ViewMatrix();
            invert(mat0);
            Vec4f nearWsPt, farWsPt;
            xform(nearWsPt, mat0, Vec4f(0, 0, 0, 1));
            Matrix44f mat1 = cam.GetPerspectiveMatrix(ctx.m_nearfar[1], ctx.m_nearfar[2]) *
                cam.ViewMatrix();
            invert(mat1);
            xform(farWsPt, mat1, Vec4f(0, 0, 1, 1));
            nearWsPt /= nearWsPt[3];
            farWsPt /= farWsPt[3];
            Vec3f dir = Point3f(farWsPt[0], farWsPt[1], farWsPt[2]) -
                Point3f(nearWsPt[0], nearWsPt[1], nearWsPt[2]);
            normalize(dir);
            Loc hitLoc(0,0,0);
            Vec3i hitpt;

            if (m_octTileSelection.Intersects(Point3f(nearWsPt[0], nearWsPt[1], nearWsPt[2]), dir, hitLoc, hitpt))
            {
                AABox aabb = hitLoc.GetBBox();
                Vec3f extents = (aabb.mMax - aabb.mMin);
                const int tsz = 256;
                float scl = extents[0] / (float)tsz; 
                
                Point3f offset = Vec3f(hitpt[0], hitpt[1], hitpt[2]) * scl + aabb.mMin;
                m_targetCube->SetOffset(offset);
                m_targetCube->SetScale(Vec3f(scl, scl, scl));
                g_hitLoc = hitLoc;
            }
        }

        float flyspeedup = 1;
        if (!m_flymode)
        {
            std::shared_ptr<OctTile> tile = m_octTileSelection.TileFromPos(fly.pos);
            float grnd = tile->GetGroundPos(Point2f(fly.pos[0], fly.pos[2]));

            if (isnan(grnd) || fly.pos[1] > (grnd + headheight))
            {
                m_gravityVel -= 0.0005f;
            }
            else
            {
                fly.pos[1] = grnd + headheight;
                m_gravityVel = 0;
            }
        }
        else
        {
            flyspeedup = 10;
            m_gravityVel = 0;
        }

        Vec3f right, up, forward;
        Vec3f upworld(0, 1, 0);
        fly.GetDirs(right, up, forward);
        Vec3f fwWorld;
        cross(fwWorld, right, upworld);
        fly.pos +=
            m_camVel[0] * right * flyspeedup +
            (m_camVel[1] + m_gravityVel) * upworld * flyspeedup +
            m_camVel[2] * fwWorld * flyspeedup;
        //fly.pos[1] = std::max(m_octTileSelection.GetGroundHeight(fly.pos), fly.pos[1]);

        cam.SetFly(fly);        

    }

    void World::Layout(int w, int h)
    {
        m_width = w;
        m_height = h;
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


    

    World::~World()
    {

    }

}
