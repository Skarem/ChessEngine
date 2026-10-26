#include "game/game.hpp"

int main(void)
{
    const int Scale = 6;
    App::Game game(Scale);
    game.Run();
    return 0;
}