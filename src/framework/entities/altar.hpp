#ifndef ALTAR_H
#define ALTAR_H
#include <entity.hpp>
#include <transform_component.hpp>
#include <sprites.hpp>
#include <particles.hpp>

class Altar: public Entity {
public:
    Sprite sprite;
    ParticleSystem eye1, eye2;
    std::pair<int, int> last_predict;
    Signal predicted;
    std::string text;
    float text_alpha;
    int charge;

    Altar(Vector2 pos);

    void process(float delta);
    void draw(float delta);
    void late_draw(float delta);
};

#endif