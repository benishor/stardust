#include <Entity.h>
#include <glm/gtc/matrix_transform.hpp>
#include <SpritePool.h>
#include <GfxSystem.h>
//#include <iostream>
// #include <glm/ext.hpp>

namespace Acidrain {

    using namespace glm;

    void Entity::update(float dt) {
        localTransform = mat4(1);
        localTransform = glm::translate(localTransform, vec3(position.x, position.y, 0));
        localTransform = glm::rotate(localTransform, rotation, vec3(0, 0, 1));
        localTransform = glm::scale(localTransform, vec3(scale.x, scale.y, 1));
        // std::cout << "position is " << to_string(localTransform) << std::endl;
    }

    vector<vec2> Entity::computeVertices() const {
        Box box(size);
        vector<vec2> vertices = box.computeVertices();
        for (auto& v : vertices) {
            vec4 v4 = vec4(v.x, v.y, 0, 1);
            v4 = localTransform * v4;
            v.x = v4.x;
            v.y = v4.y;
        }
        return vertices;
    }

    void DrawableEntity::update(float dt) {
        Entity::update(dt);
    }

    void DrawableEntity::addTo(SpritePool* spritePool) const {
        spritePool->addTexturedQuad(
                currentSprite.getTexture(),
                computeVertices(),
                currentSprite.getTexCoords(),
                color
        );
    }
} // namespace Acidrain