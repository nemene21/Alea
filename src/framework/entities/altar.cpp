#include <altar.hpp>

Altar::Altar(Vector2 pos):
    sprite {Sprite("skull_stick.png")},
    eye1 {ParticleSystem("eye_particles.json")},
    eye2 {ParticleSystem("eye_particles.json")},
    text {"Mark prey..."},
    text_alpha {0},
    charge {7}
{    
    auto trans_comp = new TransformComponent(this, pos);
    add_component(trans_comp);

    eye1.set_left(0);
    eye2.set_left(0);
}

void Altar::process(float delta) {
    auto trans_comp = (TransformComponent *)get_component(CompType::TRANSFORM);
    sprite.set_transform(trans_comp);

    eye1.process(delta);
    eye2.process(delta);

    Vector2 mouse_pos = Vector2Divide(GetMousePosition(), {12, 12});
    if (charge > 8) {
        eye1.set_left(1);
        eye2.set_left(1);

        if (Vector2Distance(mouse_pos, Vector2Add(trans_comp->position, {0, -10})) < 24) {

            text_alpha = Lerp(text_alpha, 255, 0.075);
            sprite.angle = sin(GetTime() * 20) * 3;
            
        } else {
            text_alpha = Lerp(text_alpha, 0, 0.075);
        }
    } else {
        text_alpha = Lerp(text_alpha, 0, 0.075);
    }
    eye1.set_position(Vector2Add(trans_comp->position, {5, -3}));
    eye2.set_position(Vector2Add(trans_comp->position, {10, -4}));
}

void Altar::draw(float delta) {
    sprite.draw();
    eye1.draw();
    eye2.draw();

    auto trans_comp = (TransformComponent *)get_component(CompType::TRANSFORM);

    DrawText(text.c_str(), trans_comp->position.x - 26, trans_comp->position.y + 33, 1, {0, 0, 0, (unsigned char)(text_alpha * .5f)});
    DrawText(text.c_str(), trans_comp->position.x - 25, trans_comp->position.y + 32, 1, {255, 255, 255, (unsigned char)text_alpha});
}

void Altar::late_draw(float delta) {}