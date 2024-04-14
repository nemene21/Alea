#ifndef GAME_SCENE_H
#define GAME_SCENE_H

#include <scene.hpp>
#include <dice.hpp>
#include <slot.hpp>
#include <altar.hpp>

class Slot;
class DraggableDice;

class GameScene: public Scene {
public:
    Sprite bg;
    bool player_turn, done, player_won, tutorial;
    typedef std::vector<Slot *> SlotRow;
    typedef std::vector<SlotRow> SlotField;

    std::vector<std::pair<int, int>> predicts;
    std::vector<Sprite> slides;

    int turn, slide_on;
    float done_timer, done_text_anim, fade;

    Altar *altar;
    Sprite shotgun;

    SlotField enemy_slots;
    SlotField player_slots;
    DraggableDice *thrown_dice;

    GameScene(bool tutorial);

    Slot *get_attacked_slot(int col, bool is_player, int num);
    int get_num_die(int col, bool is_player);
    void new_dice();

    void add_prediction(std::pair<int, int> predict);
    void winlose();

    void restart();
    void draw(float delta);
    void late_draw(float delta);
    void process(float delta);
};

#endif