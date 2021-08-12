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

    const int LeftShift = 16;
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
            fly.pos = Point3f(0.1f, 1.0f, 0.1f);
            fly.dir = Vec2f(0, 0.0f);
            e.Cam().SetFly(fly);
        }

        m_boardGroup->Clear();

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

        Vec3f right, up, forward;
        fly.GetDirs(right, up, forward);
        fly.pos +=
            m_camVel[0] * right +
            m_camVel[1] * up +
            m_camVel[2] * forward;


        m_tileSelection.Update(e, ctx);
        m_tileSelection.AddTilesToGroup(m_boardGroup);
       
        fly.pos[1] = std::max(m_tileSelection.GetGroundHeight(fly.pos), fly.pos[1]);

        cam.SetFly(fly);
    }


    void Board::Layout(int w, int h)
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


    

    Board::~Board()
    {

    }

}