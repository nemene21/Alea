#include <scenes/game_scene.hpp>


GameScene::GameScene(bool tutorial):
    tutorial {tutorial},
    Scene("game"),
    slides {
    Sprite("tut1.png", half_res),
    Sprite("tut2.png", half_res),
    Sprite("tut3.png", half_res),
    Sprite("tut4.png", half_res)
    },
    bg {Sprite("background.png", half_res)},
    enemy_slots {},
    player_slots {},
    thrown_dice {nullptr},
    player_turn {true},
    turn {1}, done {false}, done_timer {0}, done_text_anim {0}, fade {255},
    predicts {},
    shotgun {Sprite("shotgun.png", {43.1f, -100})}, slide_on {0} {}

void GameScene::add_prediction(std::pair<int, int> predict) {
    predicts.push_back(std::make_pair(predict.first + turn, predict.second));
}

void GameScene::process(float delta) {
    if (done) {
        shotgun.position.y = Lerp(shotgun.position.y, Lerp(90-14, 14, (int)!player_won), .03);

        done_timer += delta;

        if (done_timer > 3) {
            done = false;
            AudioManager::play_sfx("shot.wav");
        }
    }
}

void GameScene::restart() {
    if (tutorial) {
        return;
    }

    altar = new Altar({120.1, 40.1});
    altar->predicted.connect([this](Entity *alt) {
        add_prediction(altar->last_predict);
    });

    entities = {altar};

    // Enemy slots
    for (int i = 0; i < 4; i++) {
        float offset = i * 16;
        Slot *slot1 = new Slot(19.01f + offset, 13, i, false);
        Slot *slot2 = new Slot(19.01f + offset, 28, i, false);

        enemy_slots.push_back({slot1, slot2});
        entities.push_back(slot1);
        entities.push_back(slot2);
    }
    // Player slots
    for (int i = 0; i < 4; i++) {
        float offset = i * 16;
        Slot *slot1 = new Slot(19.01f + offset, 52, i, true);
        Slot *slot2 = new Slot(19.01f + offset, 67, i, true);

        player_slots.push_back({slot1, slot2});
        entities.push_back(slot1);
        entities.push_back(slot2);
    }

    new_dice();
}

void GameScene::winlose() {
    int enemy_pts = 0;
    for (auto& col: enemy_slots) {
        for (auto& slot: col) {

            if (slot->dice_has != nullptr) enemy_pts += slot->dice_has->num;
        }
    }

    int player_pts = 0;
    for (auto& col:  player_slots) {
        for (auto& slot: col) {

            if (slot->dice_has != nullptr) player_pts += slot->dice_has->num;
        }
    }
    done = true;
    player_won = player_pts > enemy_pts;

    if (player_won) {shotgun.position.y = 300; shotgun.angle = 180; }
    AudioManager::play_sfx("reload.wav", 2);
}

void GameScene::new_dice() {
    int sum = 0;
    for (auto& col: enemy_slots) {
        for (auto& slot: col) sum += (int)(slot->dice_has != nullptr);
    }
    if (sum == 8) { winlose(); return; }

    sum = 0;
    for (auto& col: player_slots) {
        for (auto& slot: col) sum += (int)(slot->dice_has != nullptr);
    }
    if (sum == 8) { winlose(); return; }

    thrown_dice = (DraggableDice *)(new DraggableDice(5.1f, 39.1f, turn % 2 == 1));
    entities.push_back(thrown_dice);

    for (int i = 0; i < predicts.size(); i++) {
        auto predict = predicts[i];

        if (predict.first == turn) {
            thrown_dice->predict = predict.second;
            predicts.erase(predicts.begin() + i);
        }
    }

    turn++;
    altar->charge++;
}

int GameScene::get_num_die(int col, bool is_player) {
    if (is_player) {
        return (int)(enemy_slots[col][0]->dice_has != nullptr) + (int)(enemy_slots[col][1]->dice_has != nullptr);
    } else {
        return (int)(player_slots[col][0]->dice_has != nullptr) + (int)(player_slots[col][1]->dice_has != nullptr);

    }
}

Slot *GameScene::get_attacked_slot(int col, bool is_player, int num) {
    if (is_player) {
        Slot *slot1 = enemy_slots[col][1];
        if (slot1->dice_has != nullptr) {
            if (slot1->dice_has->num == num) {
                return slot1;
            }
        }

        Slot *slot2 = enemy_slots[col][0];
        if (slot2->dice_has != nullptr) {
            if (slot2->dice_has->num == num) {
                return slot2;
            }
        }
    } else {
        Slot *slot1 = player_slots[col][1];
        if (slot1->dice_has != nullptr) {
            if (slot1->dice_has->num == num) {
                return slot1;
            }
        }

        Slot *slot2 = player_slots[col][0];
        if (slot2->dice_has != nullptr) {
            if (slot2->dice_has->num == num) {
                return slot2;
            }
        }
    }
    return nullptr;
}


void GameScene::draw(float delta) {
    bg.draw();
}

void GameScene::late_draw(float delta) {
    if (tutorial) {
        slides[slide_on].draw();

        if (IsKeyPressed(KEY_SPACE)) {
            fade = 255;
            slide_on++;

            if (slide_on >= 4) {
                SceneManager::setup_scene(new GameScene(false));
                SceneManager::set_scene("game");
            }
        }
    }

    shotgun.draw();

    fade = Lerp(fade, 0, 0.05f);

    DrawRectangle(0, 0, 160, 90, {0, 0, 0, (unsigned char)fade});

    if (done_timer > 3) {
        DrawRectangle(0, 0, 160, 90, BLACK);

        done_text_anim = Lerp(done_text_anim, 1, 0.01f);
        float anim = powf(done_text_anim, 3.f);
        if (player_won) {
            DrawText("You beat the game. Good job!", 8, 8 + (24 * (1.f - anim)), 2, {255, 255, 255, (unsigned char)(255 * anim)});
            DrawText("Thanks for playing :) [ESC]", 8, 24 + (24 * (1.f - anim)), 2, {255, 255, 255, (unsigned char)(255 * anim)});
        } else {
            DrawText("Yuck, the tase of lead...", 8, 8 + (24 * (1.f - anim)), 2, {255, 255, 255, (unsigned char)(255 * anim)});
            DrawText("[SPACE] to restart", 8, 24 + (24 * (1.f - anim)), 2, {255, 255, 255, (unsigned char)(255 * anim)});
        }

        if (IsKeyPressed(KEY_SPACE)) {
            SceneManager::setup_scene(new GameScene(false));
            SceneManager::set_scene("game");
        }
    }
}
