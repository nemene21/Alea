#include <slot.hpp>

Slot::Slot(float x, float y, int index, bool is_player):
    index {index},
    is_player {is_player},
    sprite {Sprite("slot.png")},
    selector_sprite {Sprite("select.png")},
    attacked_sprite {Sprite("attacked_icon.png")},
    kill_selector_sprite {Sprite("kill_select.png")},
    dice_has {nullptr},
    dice_particles {ParticleSystem("dice_place.json", Vector2{x, y})},
    potentially_placing {false}, being_sacraficed {false} {
    
    auto trans_comp = new TransformComponent(this, Vector2{x, y});
    add_component(trans_comp);

    dice_particles.set_left(0);
}

void Slot::process(float delta) {
    being_selected = false;

    auto trans_comp = (TransformComponent *)get_component(CompType::TRANSFORM);
    sprite.set_transform(trans_comp);

    selector_sprite.set_transform(trans_comp);
    selector_sprite.set_scale({sinf(GetTime()*25.f)*.25f + 1.f, sinf(GetTime()*25.f)*.25f + 1.f});
    selector_sprite.position.y -= 2;
    selector_sprite.position.x += .5f;

    kill_selector_sprite.set_transform(trans_comp);
    kill_selector_sprite.set_scale({sinf(GetTime()*25.f)*.25f + 1.f, sinf(GetTime()*25.f)*.25f + 1.f});
    kill_selector_sprite.position.y -= 2;
    kill_selector_sprite.position.x += .5f;

    attacked_sprite.set_transform(trans_comp);
    attacked_sprite.position.x -= 7.1f;
    attacked_sprite.position.y -= 3.1f;

    if (!is_player && dice_has != nullptr) {
        Altar *altar = ((GameScene *)SceneManager::scene_on)->altar;
        if (altar->charge > 8) {
            Vector2 mouse_pos = Vector2Divide(GetMousePosition(), {12, 12});
            float dist = Vector2Distance(trans_comp->position, mouse_pos);

            if (dist < 7) {
                if (!being_sacraficed) {
                    being_sacraficed = true;
                    AudioManager::play_sfx("potential_kill.wav", 1, 1, .1, .1);
                }

                if ((IsMouseButtonReleased(MOUSE_BUTTON_LEFT))) {
                    dice_has->queue_free();
                    dice_has = nullptr;
                    dice_particles.set_left(1);

                    AudioManager::play_sfx("dice_die.wav", 1, 1, .1, .1);

                    altar->charge = 0;
                }
            } else {
                being_sacraficed = false;
            }
        } else being_sacraficed = false;
    } else being_sacraficed = false;

    if (dice_has == nullptr) {
        DraggableDice *dice = ((GameScene *)SceneManager::scene_on)->thrown_dice;

        if (is_player && dice->is_player && dice->picked_up) {

            Vector2 mouse_pos = Vector2Divide(GetMousePosition(), {12, 12});
            float dist = Vector2Distance(trans_comp->position, mouse_pos);

            if (dist < 7) {
                being_selected = true;
                dice->player_slot = this;

                Slot *attacking = ((GameScene *)SceneManager::scene_on)->get_attacked_slot(index, is_player, dice->num);
                if (attacking != nullptr) {
                    attacking->being_attacked = true;
                    attacking->dice_has->being_attacked = true;
                    
                    if (!potentially_placing) {
                        potentially_placing = true;
                        AudioManager::play_sfx("potential_kill.wav", 1, 1, .1, .1);
                    }
                } else if (!potentially_placing) {
                    potentially_placing = true;
                    AudioManager::play_sfx("potential_placement.wav", 1, 1, .1, .1);
                }

                if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                    AudioManager::play_sfx("place.wav", 1, 1, .1, .1);

                    dice_has = dice;
                    dice_has->placed = true;

                    if (attacking != nullptr) {
                        attacking->dice_has->queue_free();
                        attacking->dice_has = nullptr;
                        attacking->dice_particles.set_left(1);

                        AudioManager::play_sfx("dice_die.wav", 1, 1, .1, .1);
                    }

                    dice_particles.set_left(1);

                    ((TransformComponent *)dice_has->get_component(CompType::TRANSFORM))->position = Vector2Add(trans_comp->position, {.1f, -4.f});
                    ((GameScene *)SceneManager::scene_on)->new_dice();
                }
            } else potentially_placing = false;
        }
    } else {
        dice_has->player_slot = this;
    }
    dice_particles.process(delta);
}

void Slot::draw(float delta) {
    sprite.draw();

    if (being_selected) {
        selector_sprite.draw();
    }
    if (being_sacraficed) {
        kill_selector_sprite.draw();
    }
}

void Slot::late_draw(float delta) {
    dice_particles.draw();
    
    if (being_attacked) {
        attacked_sprite.draw();
    }
    being_attacked = false;
}