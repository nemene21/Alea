#ifndef SLOT_H
#define SLOT_H

#include <entity.hpp>
#include <transform_component.hpp>

#include <sprites.hpp>
#include <particles.hpp>
#include <scenes/game_scene.hpp>
#include <audio.hpp>

class DraggableDice;

class Slot: public Entity {
public:
    Sprite sprite, selector_sprite, kill_selector_sprite, attacked_sprite;
    ParticleSystem dice_particles;
    int index;
    bool is_player, being_selected, being_attacked, potentially_placing, being_sacraficed;
    DraggableDice *dice_has;

    Slot(float x, float y, int index, bool is_player);

    void process(float delta);
    void draw(float delta);
    void late_draw(float delta);
};

#endif