#include "StdIncludes.h"
#include "Physics.h"
#include "Application.h"
#include "Engine.h"
#include "SimplexNoise/SimplexNoise.h"
#include <numeric>
#include "Mesh.h"
#include "OctTile.h"
#include "Frustum.h"
#include "gmtl/PlaneOps.h"
#include "bullet/btBulletCollisionCommon.h"
#include "bullet/btBulletDynamicsCommon.h"

namespace sam
{
    inline Vec3f IBT(const btVector3& v3)
    {
        return Vec3f(v3[0], v3[1], v3[2]);
    }

    class PhysicsDebugDraw : public btIDebugDraw
    {
        struct Line
        {
            Vec3f from;
            Vec3f to;
            Vec3f color;
        };


        float m_invWorldScale;
        int m_lineIdx;
        std::vector<Line> m_lines[3];
        int m_debugMode;

        void drawLine(const btVector3& from1, const btVector3& to1, const btVector3& color1) override
        {
            Line l;
            l.from = IBT(from1) * m_invWorldScale;
            l.to = IBT(to1) * m_invWorldScale;
            l.color = IBT(color1);
            m_lines[m_lineIdx].push_back(l);
        }

        void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) override
        {
            drawLine(PointOnB, PointOnB + normalOnB * distance, color);
            btVector3 ncolor(0, 0, 0);
            drawLine(PointOnB, PointOnB + normalOnB * 0.001, ncolor);
        }

        virtual void reportErrorWarning(const char* warningString)
        {
        }

        virtual void draw3dText(const btVector3& location, const char* textString)
        {
        }

        virtual void setDebugMode(int debugMode)
        {
            m_debugMode = debugMode;
        }

        virtual int getDebugMode() const
        {
            return m_debugMode;
        }

        virtual void flushLines()
        {
        }
    public:
        PhysicsDebugDraw(float iws) :
            m_invWorldScale(iws),
            m_lineIdx(0),
            m_debugMode(btIDebugDraw::DBG_DrawWireframe) {}

        void BeginDraw()
        {
            m_lines[m_lineIdx].clear();
        }

        void EndDraw()
        {
            m_lineIdx = (m_lineIdx + 1) % 3;
        }


        void Render(const DrawContext& ctx)
        {
            if (!m_shader.isValid())
            {
                m_shader = Engine::Inst().LoadShader("vs_cubes.bin", "fs_targetcube.bin");
            }
            if (ctx.m_nearfarpassIdx != 1)
                return;
            const std::vector<Line> &lines = m_lines[(m_lineIdx + 3 - 1) % 3];
            size_t vtxCount = lines.size() * 4;
            if (vtxCount == 0)
                return;

            PosTexcoordVertex* vertices = new PosTexcoordVertex[vtxCount];
            size_t indexCnt = lines.size() * 6;
            int* indices = new int[indexCnt];
            size_t vtxIdx = 0, indexIdx = 0;
            Vec3f spread(1, 0, 0);
            spread *= 0.01f;
            for (const Line& line : lines)
            {
                indices[indexIdx++] = vtxIdx;
                indices[indexIdx++] = vtxIdx + 1;
                indices[indexIdx++] = vtxIdx + 2;
                indices[indexIdx++] = vtxIdx + 1;
                indices[indexIdx++] = vtxIdx + 3;
                indices[indexIdx++] = vtxIdx + 2;

                {
                    PosTexcoordVertex& v = vertices[vtxIdx++];
                    Vec3f pt = line.from + spread;
                    v.m_x = pt[0];
                    v.m_y = pt[1];
                    v.m_z = pt[2];
                }
                {
                    PosTexcoordVertex& v = vertices[vtxIdx++];
                    Vec3f pt = line.from - spread;
                    v.m_x = pt[0];
                    v.m_y = pt[1];
                    v.m_z = pt[2];
                }
                {
                    PosTexcoordVertex& v = vertices[vtxIdx++];
                    Vec3f pt = line.to + spread;
                    v.m_x = pt[0];
                    v.m_y = pt[1];
                    v.m_z = pt[2];
                }
                {
                    PosTexcoordVertex& v = vertices[vtxIdx++];
                    Vec3f pt = line.to - spread;
                    v.m_x = pt[0];
                    v.m_y = pt[1];
                    v.m_z = pt[2];
                }

            }
            m_vbh = bgfx::createVertexBuffer(
                bgfx::makeRef(vertices, sizeof(PosTexcoordVertex) * vtxCount, ReleaseFn)
                , PosTexcoordNrmVertex::ms_layout
            );

            m_ibh = bgfx::createIndexBuffer(
                bgfx::makeRef(indices, sizeof(int) * indexCnt, ReleaseFn)
            );

            Matrix44f m = ctx.m_mat;
            bgfx::setTransform(m.getData());
            // Set vertex and index buffer.
            bgfx::setVertexBuffer(0, m_vbh);
            bgfx::setIndexBuffer(m_ibh);
            uint64_t state = 0
                | BGFX_STATE_WRITE_RGB
                | BGFX_STATE_WRITE_A
                | BGFX_STATE_WRITE_Z
                | BGFX_STATE_MSAA
                | BGFX_STATE_BLEND_ALPHA;
            // Set render states.l
            bgfx::setState(state);
            bgfx::submit(ctx.m_curviewIdx, m_shader);
        }


        static void ReleaseFn(void* ptr, void* pThis)
        {
            delete[]ptr;

        }

        static bgfxh<bgfx::ProgramHandle> m_shader;
        bgfxh<bgfx::VertexBufferHandle> m_vbh;
        bgfxh<bgfx::IndexBufferHandle> m_ibh;
    };
    
    bgfxh<bgfx::ProgramHandle> PhysicsDebugDraw::m_shader;

    float Physics::WorldScale = 1000;

    Physics::Physics() :
        m_isInit(false)
    {

    }

    void Physics::Init()
    {
        m_collisionConfig = std::make_shared<btDefaultCollisionConfiguration>();
        m_broadPhase = std::make_shared< btDbvtBroadphase>();
        m_collisionDispatcher = std::make_shared<btCollisionDispatcher>(m_collisionConfig.get());
        m_constraintSolver = std::make_shared<btSequentialImpulseConstraintSolver>();
        m_discreteDynamicsWorld = std::make_shared<btDiscreteDynamicsWorld>(
            m_collisionDispatcher.get(), m_broadPhase.get(), m_constraintSolver.get(), m_collisionConfig.get());
        m_discreteDynamicsWorld->setGravity(btVector3(0, 10, 0));
        m_dbgPhysics = std::make_shared<PhysicsDebugDraw>(1 / Physics::WorldScale);
        m_discreteDynamicsWorld->setDebugDrawer(m_dbgPhysics.get());
        m_isInit = true;
    }

    void Physics::AddRigidBody(btRigidBody* pRigidBody)
    {
        m_discreteDynamicsWorld->addRigidBody(pRigidBody);
    }

    void Physics::RemoveRigidBody(btRigidBody* pRigidBody)
    {
        m_discreteDynamicsWorld->removeRigidBody(pRigidBody);
    }

    void Physics::Step(const DrawContext& ctx)
    {
        if (!m_isInit)
            Init();
        m_discreteDynamicsWorld->stepSimulation(1.0f / 60.0f, 10);
        
        m_dbgPhysics->BeginDraw();
        m_discreteDynamicsWorld->debugDrawWorld();
        m_dbgPhysics->EndDraw();
    }
    
    void Physics::DebugRender(DrawContext& ctx)
    {
        m_dbgPhysics->Render(ctx);
    }
}