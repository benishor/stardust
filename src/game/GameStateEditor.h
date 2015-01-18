#pragma once
#include <GameState.h>
#include <memory>

namespace Acidrain {

    using namespace std;

    class Font;

    class Stardust;

    class GameStateEditor : public GameState<Stardust> {

    public:

        static GameStateEditor& instance();

        virtual void onEnter(Stardust* game) override;

        virtual void onExit(Stardust* game) override;

        virtual void update(Stardust* game, float elapsedSeconds) override;

        virtual void render(Stardust* game, float alpha) override;

    private:
        shared_ptr<Font> font;
    };

} // namespace Acidrain