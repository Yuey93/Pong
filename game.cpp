#define is_down(b) input->buttons[b].is_down
#define pressed(b) (input->buttons[b].is_down && input->buttons[b].changed)
#define released(b) (!input->buttons[b].is_down && input->buttons[b].changed)

float player_1_p, player_1_dp, player_2_p, player_2_dp;
float arena_half_size_x = 85, arena_half_size_y = 45;
float player_half_size_x = 2.5, player_half_size_y = 12;
float ball_p_x, ball_p_y, ball_dp_x = 100, ball_dp_y, ball_half_size = 1;
						  // Ball speed^
int player_1_score, player_2_score;

internal void
simulate_player(float* p, float* dp, float ddp, float dt) {
	ddp -= *dp * 2.f;		// Friction

	*p = *p + *dp * dt + ddp * dt * dt * .1f; // Acceleration
	*dp = *dp + ddp * dt;

	if (*p + player_half_size_y > arena_half_size_y) {
		*p = arena_half_size_y - player_half_size_y;
		*dp *= -.2f;				// Bounce off top wall
	}
	else if (*p - player_half_size_y < -arena_half_size_y) {
		*p = -arena_half_size_y + player_half_size_y;
		*dp *= -.2f;				// Bounce off bottom wall
	}
}

internal bool
aabb_vs_aabb(float p1x, float p1y, float hs1x, float hs1y,
	float p2x, float p2y, float hs2x, float hs2y) {
	return(p1x + hs1x > p2x - hs2x &&
		p1x - hs1x < p2x + hs2x &&
		p1y + hs1y > p2y - hs2y &&
		p1y - hs1y < p2y + hs2y);
}

enum GameMode {
	GM_MENU,
	GM_GAMEPLAY,
};

GameMode current_gamemode;
int hot_button;
bool enemy_is_ai;

internal void
simulate_game(Input* input, float dt) {

	draw_rect(0, 0, arena_half_size_x, arena_half_size_y, 0xffaa33);    // Arena colour
	draw_arena_borders(arena_half_size_x, arena_half_size_y, 0xff5500); // Border colour

	if (current_gamemode == GM_GAMEPLAY) {
		float player_1_ddp = 0.f;
		if (!enemy_is_ai) {
			if (is_down(BUTTON_UP)) player_1_ddp += 200;
			if (is_down(BUTTON_DOWN)) player_1_ddp -= 200;
		}
		else {
		if (ball_p_y > player_1_p + 2.f) player_1_ddp += 140;
		if (ball_p_y < player_1_p + 2.f) player_1_ddp -= 140;
		}

		float player_2_ddp = 0.f;
		if (is_down(BUTTON_W)) player_2_ddp += 200;
		if (is_down(BUTTON_S)) player_2_ddp -= 200;

		simulate_player(&player_1_p, &player_1_dp, player_1_ddp, dt);
		simulate_player(&player_2_p, &player_2_dp, player_2_ddp, dt);

		//Simulate Ball
		{
			ball_p_x += ball_dp_x * dt;
			ball_p_y += ball_dp_y * dt;

			if (aabb_vs_aabb(ball_p_x, ball_p_y, ball_half_size, ball_half_size, 80, player_1_p, player_half_size_x, player_half_size_y)) {
				ball_p_x = 80 - player_half_size_x - ball_half_size;
				ball_dp_x *= -1; // Ball bounce off p1
				ball_dp_y = (ball_p_y - player_1_p) * 2 + player_1_dp * .75f; // Ball bounces based on where paddle hit and direction of paddle
			}
			else if (aabb_vs_aabb(ball_p_x, ball_p_y, ball_half_size, ball_half_size, -80, player_2_p, player_half_size_x, player_half_size_y)) {
				ball_p_x = -80 + player_half_size_x + ball_half_size;
				ball_dp_x *= -1; // Ball bounce off p2
				ball_dp_y = (ball_p_y - player_2_p) * 2 + player_2_dp * .75f;
			}

			if (ball_p_y + ball_half_size > arena_half_size_y) {
				ball_p_y = arena_half_size_y - ball_half_size;
				ball_dp_y *= -1; // Ball collides with top
			}
			else if (ball_p_y - ball_half_size < -arena_half_size_y) {
				ball_p_y = -arena_half_size_y + ball_half_size;
				ball_dp_y *= -1; // Ball collides with bottom
			}

			if (ball_p_x + ball_half_size > arena_half_size_x) {
				ball_dp_x *= -1;
				ball_dp_y = 0;
				ball_p_x = 0;
				ball_p_y = 0;
				player_1_score++;
			} // Ball hit left side
			else if (ball_p_x - ball_half_size < -arena_half_size_x) {
				ball_dp_x *= -1;
				ball_dp_y = 0;
				ball_p_x = 0;
				ball_p_y = 0;
				player_2_score++;
			} // Ball hit right side
		}

		draw_number(player_1_score, -10, 40, 1.f, 0xbbffbb);
		draw_number(player_2_score, 10, 40, 1.f, 0xbbffbb);


		draw_rect(ball_p_x, ball_p_y, 1, 1, 0xffffff); // Ball


		draw_rect(-80, player_2_p, player_half_size_x, player_half_size_y, 0xff0000);		// Paddles
		draw_rect(80, player_1_p, player_half_size_x, player_half_size_y, 0xff0000);

	}
	else {

		if (pressed(BUTTON_LEFT) || pressed(BUTTON_RIGHT)) {
			hot_button = !hot_button;
		}

		if (pressed(BUTTON_ENTER)) {
			current_gamemode = GM_GAMEPLAY;
			enemy_is_ai = hot_button ? 0 : 1;		//1 Player or 2 Player
		}

		if (hot_button == 0) {
			draw_text("SINGLE PLAYER", -80, -10, 1, 0xff0000);
			draw_text("MULTIPLAYER", 20, -10, 1, 0xaaaaaa);
		}
		else {
			draw_text("SINGLE PLAYER", -80, -10, 1, 0xaaaaaa);
			draw_text("MULTIPLAYER", 20, -10, 1, 0xff0000);
		}

		draw_text("PONG", -80, 40, 2, 0xff00ff);
		draw_text("BY YUSEF JONES", -80, 22, .75, 0xff00ff);

		draw_text("CONTROLS", -80, -32, .8, 0xffffff);
		draw_text("USE W/S AND UP/DOWN ARROWS", -80, -40, .5, 0xffffff);

	}

}