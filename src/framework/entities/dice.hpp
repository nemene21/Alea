#ifndef CONT_DICE_H
#define CONT_DICE_H

#include <entity.hpp>
#include <sprites.hpp>

#include <transform_component.hpp>
#include <health_component.hpp>
#include <animation_component.hpp>
#include <slot.hpp>
#include <algorithm>

class ControllableDice: public Entity {
public:
    Sprite sprite;
    bool is_player;
    float speed;

    ControllableDice(float x, float y, bool is_player);

    void process(float delta);
    void draw(float delta);

    void player_control(float delta);
    void  enemy_control(float delta);
};

class Slot;

class DraggableDice : public Entity {
public:
    Sprite sprite;
    bool is_player, rolled, picked_up, placed, being_attacked;
    Slot *player_slot;
    int num, enemy_slot_col, enemy_slot_row, predict;

    DraggableDice(float x, float y, bool is_player);

    void process(float delta);
    void draw(float delta);

    void player_control(float delta);
    void  enemy_control(float delta);

    void enemy_play();
    bool attempt_enemy_play(int col, int row);
};

#endif