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
    }

    void Physics::Step(const DrawContext& ctx)
    {
        m_discreteDynamicsWorld->stepSimulation(1.0f / 60.0f, 10);
    }
}