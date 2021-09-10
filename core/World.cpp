#include "StdIncludes.h"
#include "World.h"
#include "Application.h"
#include "Engine.h"
#include "SimplexNoise/SimplexNoise.h"
#include <numeric>
#include "Mesh.h"
#include "gmtl/PlaneOps.h"
#include "leveldb/dumpfile.h"
#include "leveldb/env.h"
#include "leveldb/status.h"
#include "leveldb/options.h"
#include "leveldb/filter_policy.h"
#include "leveldb/cache.h"
#include "leveldb/zlib_compressor.h"
#include "leveldb/decompress_allocator.h"
#include "leveldb/db.h"

#define NOMINMAX


using namespace gmtl;

namespace sam
{
    class NullLogger : public leveldb::Logger {
    public:
        void Logv(const char*, va_list) override {
        }
    };

    World::World() :
        m_width(-1),
        m_height(-1),
        m_currentTool(0),
        m_gravityVel(0),
        m_db(nullptr)
    {

        leveldb::Env* env = leveldb::Env::Default();
        leveldb::Options options;
        //create a bloom filter to quickly tell if a key is in the database or not
        options.filter_policy = leveldb::NewBloomFilterPolicy(10);

        //create a 40 mb cache (we use this on ~1gb devices)
        options.block_cache = leveldb::NewLRUCache(40 * 1024 * 1024);

        //create a 4mb write buffer, to improve compression and touch the disk less
        options.write_buffer_size = 4 * 1024 * 1024;

        //disable internal logging. The default logger will still print out things to a file
        options.info_log = new NullLogger();

        //use the new raw-zip compressor to write (and read)
        options.compressors[0] = new leveldb::ZlibCompressorRaw(-1);

        //also setup the old, slower compressor for backwards compatibility. This will only be used to read old compressed blocks.
        options.compressors[1] = new leveldb::ZlibCompressor();

        options.create_if_missing = true;
        leveldb::Status status = leveldb::DB::Open(options, "testlvl", &m_db);
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
            m_octTileSelection.GetNearFarMidDist(ctx.m_nearfar);
        }

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

        Vec3f right, up, forward;
        Vec3f upworld(0, 1, 0);
        fly.GetDirs(right, up, forward);
        Vec3f fwWorld;
        cross(fwWorld, right, upworld);
        fly.pos +=
            m_camVel[0] * right +
            (m_camVel[1] + m_gravityVel) * upworld +
            m_camVel[2] * fwWorld;
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