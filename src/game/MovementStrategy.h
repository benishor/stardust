#pragma once

namespace Acidrain {

    class Entity;

    class MovementStrategy {
    public:
        void setControlledEntity(Entity *controlledEntity);

        virtual void update(float elapsedSeconds) = 0;

    protected:
        Entity *controlledEntity;
    };
}