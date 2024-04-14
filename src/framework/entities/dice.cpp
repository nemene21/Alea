#include <dice.hpp>

// Controllable dice
ControllableDice::ControllableDice(float x, float y, bool is_player):
    sprite {Sprite("dice.png", {x, y})},
    speed {60},
    is_player {is_player} {

    auto transform_comp = new TransformComponent(this, Vector2{x, y});
    add_component((Component *)transform_comp);

    auto hp_comp = new HealthComponent(this, 100);
    add_component(hp_comp);
}

std::string dice_textures[6] = {
    "dice1.png",
    "dice2.png",
    "dice3.png",
    "dice4.png",
    "dice5.png",
    "dice6.png"
};

void ControllableDice::process( float delta) {

    auto trans_comp = (TransformComponent *)get_component(CompType::TRANSFORM);
    sprite.set_transform(trans_comp);

    float blend = (float)(Vector2Length(trans_comp->velocity) > speed*.5f);
    sprite.set_scale(Lerp(Vector2{1, 1}, Vector2{
        1.f + sinf(GetTime()*15.f) * 0.25f,
        1.f + cosf(GetTime()*15.f) * 0.25f,
    }, blend));

    if (is_player)
        player_control(delta);
    else
         enemy_control(delta);
    
    trans_comp->position.x = Clamp(trans_comp->position.x, 85, 160);
    trans_comp->position.y = Clamp(trans_comp->position.y, 0, 90);
}


void ControllableDice::draw(float delta) {
    sprite.draw();
}

void ControllableDice::player_control(float delta) {
    auto trans_comp = (TransformComponent *)get_component(CompType::TRANSFORM);

    Vector2 input = {(float)(IsKeyDown(KEY_D)) - (float)(IsKeyDown(KEY_A)),
                     (float)(IsKeyDown(KEY_S)) - (float)(IsKeyDown(KEY_W))};
    input = Vector2Normalize(input);

    trans_comp->interpolate_velocity(Vector2Multiply(input, {speed, speed}), 20);
}

void ControllableDice::enemy_control(float delta) {

}

// Draggable dice
DraggableDice::DraggableDice(float x, float y, bool is_player):
    sprite {Sprite("dice.png")},
    is_player {is_player},
    num {0},
    picked_up {false},
    rolled {false},
    player_slot {nullptr},
    placed {false},
    being_attacked {false},
    predict {-1} {

    auto transform_comp = new TransformComponent(this, Vector2{x, y});
    add_component((Component *)transform_comp);

    // Roll animation
    std::function<void(float)> key_func = [this](float anim) {
        auto trans_comp = (TransformComponent *)get_component(CompType::TRANSFORM);
        trans_comp->scale = Lerp(
            Vector2{2, 2}, Vector2{1, 1}, Easing::bounce_out(anim)
        );
    };

    auto anim_comp = new AnimationComponent(this);
    anim_comp->make_animation("roll", 1.5f, false);

    anim_comp->add_keyframe("roll", 0.f, 1.5f, key_func);

    anim_comp->add_keyframe("roll", 0.f, 1.5f, [this](float anim) {
        auto trans_comp = (TransformComponent *)get_component(CompType::TRANSFORM);
        trans_comp->angle = Easing::ease_out(anim) * 720.f;
    });

    anim_comp->add_event("roll", 1.5, [this](float anim) {
        rolled = true;
        if (predict != -1) {
            num = predict;
        }
        if (!this->is_player) enemy_play();
    });

    anim_comp->add_event("roll", 0.33f, [this](float anim) { AudioManager::play_sfx("throw.wav", .2, .9, .1, .1); });

    anim_comp->play("roll");

    anim_comp->make_animation("enemy_play", 2, false);

    anim_comp->add_keyframe("enemy_play", .33f, 1.f, [this](float anim) {
        auto trans_comp = (TransformComponent *)get_component(CompType::TRANSFORM);
        trans_comp->position = Lerp(Vector2{5.1f, 40.1f}, Vector2{6.1f, 16.1f}, Easing::ease_in_out(anim));
    });
    anim_comp->add_event("enemy_play", .33f, [this](float anim) {AudioManager::play_sfx("pick.wav", 1, 1, .1, .1);});

    anim_comp->add_keyframe("enemy_play", 1.f, 2.f, [this](float anim) {
        auto trans_comp = (TransformComponent *)get_component(CompType::TRANSFORM);
        Slot *slot = ((GameScene *)(SceneManager::scene_on))->enemy_slots[enemy_slot_col][enemy_slot_row];

        auto slot_trans_comp = (TransformComponent *)slot->get_component(CompType::TRANSFORM);

        trans_comp->position = Lerp(Vector2{5.1f, 16.1f}, 
            Vector2Add(slot_trans_comp->position, {.1f, -4.f}),
        Easing::ease_in_out(anim));
    });

    anim_comp->add_event("enemy_play", 2.f, [this](float anim) {
        placed = true;
        Slot *slot = ((GameScene *)(SceneManager::scene_on))->enemy_slots[enemy_slot_col][enemy_slot_row];
        slot->dice_has = this;

        Slot *slot_attacking = ((GameScene *)(SceneManager::scene_on))->get_attacked_slot(enemy_slot_col, this->is_player, num);
        if (slot_attacking != nullptr) {
            slot_attacking->dice_has->queue_free();
            slot_attacking->dice_has = nullptr;
            slot_attacking->dice_particles.set_left(1);

            AudioManager::play_sfx("dice_die.wav", 1, 1, .1, .1);
        }

        AudioManager::play_sfx("place.wav", 1, 1, .1, .1);
        slot->dice_particles.set_left(1);

        ((GameScene *)(SceneManager::scene_on))->new_dice();
    });

    add_component(anim_comp);
}

bool DraggableDice::attempt_enemy_play(int col, int row) {
    Slot *slot = ((GameScene *)(SceneManager::scene_on))->enemy_slots[col][row];

    if (slot->dice_has == nullptr) {
        enemy_slot_col = col;
        enemy_slot_row = row;
        return true;
    }
    return false;
}


void DraggableDice::enemy_play() {
    ((AnimationComponent *)get_component(CompType::ANIMATION))->play("enemy_play");
    GameScene *scene = ((GameScene *)SceneManager::scene_on);

    std::vector<std::pair<int, int>> plays {};

    bool sucess = false;
    if (num > 2) {
        for (int i = 0; i < 4; i++) {
            int points = scene->get_num_die(i, is_player) * 2;
            if (scene->get_attacked_slot(i, is_player, num) != nullptr) points += 6;

            plays.push_back(std::make_pair(i, points));
        }
        std::sort(plays.begin(), plays.end(), [](std::pair<int, int> a, std::pair<int, int> b) { return a.second > b.second; });
    } else {
        for (int i = 0; i < 4; i++) {
            int points = -scene->get_num_die(i, is_player) * 3;
            if (scene->get_attacked_slot(i, is_player, num) != nullptr) points -= 6;

            plays.push_back(std::make_pair(i, points));
        }
        std::sort(plays.begin(), plays.end(), [](std::pair<int, int> a, std::pair<int, int> b) { return a.second > b.second; });
    }

    for (auto play: plays) {
        bool sucess = attempt_enemy_play(play.first, 0);
        if (sucess) break;
        sucess = attempt_enemy_play(play.first, 1);
        if (sucess) break;
    }
}

void DraggableDice::process(float delta) {
    sprite = Sprite(dice_textures[num-1]);

    auto trans_comp = (TransformComponent *)get_component(CompType::TRANSFORM);
    sprite.set_transform(trans_comp);

    if (is_player) {
        player_control(delta);
    } else {
        enemy_control(delta);
    }

    if (!rolled) {
        num = rand()%6 + 1;
    }

    if (being_attacked) {
        sprite.angle = sinf(GetTime()*20.f) * 15;
    }
}

void DraggableDice::player_control(float delta) {
    auto trans_comp = (TransformComponent *)get_component(CompType::TRANSFORM);

    if (placed) return;

    if (picked_up) {
        if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            picked_up = false;
            AudioManager::play_sfx("pick.wav", .6f, 1, .03, .03);
        }

        if (player_slot == nullptr) {
            trans_comp->position = Lerpi(trans_comp->position, Vector2Divide(GetMousePosition(), {12, 12}), 40);
        } else {
            trans_comp->position = Lerpi(
                trans_comp->position,
                Vector2Add(((TransformComponent *)(player_slot->get_component(CompType::TRANSFORM)))->position, {.1f, -4.f}),
                20
            );
        }

    } else {

        float dist = Vector2Distance(Vector2Divide(GetMousePosition(), {12, 12}), trans_comp->position);
        if (dist < 10 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && rolled) {
            picked_up = true;
            AudioManager::play_sfx("pick.wav", 1, 1, .03, .03);
        }
    }
    player_slot = nullptr;
}

void DraggableDice::enemy_control(float delta) {

}

void DraggableDice::draw(float delta) {
    sprite.draw();
    being_attacked = false;
}