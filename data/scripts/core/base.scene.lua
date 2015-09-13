---------------------------------------------------------------------
-- The scene abstraction
---------------------------------------------------------------------

Scene = {}
function Scene.create(type)
    local entityPointer = createEntity(SCENE, type);
    return Entity.from(entityPointer)
end

function Scene.add(entity)
    addEntity(SCENE, entity:getPointer())
end

function Scene.confineToPlayingArea(entity)
    return confineToPlayingArea(SCENE, entity:getPointer())
end

function Scene.dump()
    dumpEntities(SCENE)
end

function Scene.endLevel()
    endLevel(SCENE)
end

function Scene.pauseLevel()
    pauseLevel(SCENE)
end

function Scene.resumeLevel()
    resumeLevel(SCENE)
end
