#pragma once
#include <map>
#include <set>

class btDefaultCollisionConfiguration;
struct btDbvtBroadphase;
class btCollisionDispatcher;
class btDiscreteDynamicsWorld;
class btConstraintSolver;

namespace sam
{
    class DrawContext;
    class Physics
    {
    public:
        Physics();

        void Step(const DrawContext& ctx);
    private:
        void Init();

        std::shared_ptr<btDefaultCollisionConfiguration> m_collisionConfig;
        std::shared_ptr<btDbvtBroadphase> m_broadPhase;
        std::shared_ptr<btCollisionDispatcher> m_collisionDispatcher;
        std::shared_ptr<btDiscreteDynamicsWorld> m_discreteDynamicsWorld;
        std::shared_ptr<btConstraintSolver> m_constraintSolver;
        bool m_isInit;
    };
}