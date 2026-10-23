#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>
#include <time.h>

//sterowanie
#define K_U 'w'
#define K_L 'a'
#define K_R 'd'
#define K_D 's'
#define K_SPEED_UP 'i'
#define K_SPEED_DOWN 'o'
#define QUIT 'q'
#define K_ALBATROS 't'

//konfiguracja gry
#define FRAME_TIME  100
#define BIRD_SPEED  1

//konfiguracja gwiazdek
#define MAX_STARS   50
#define SPAWN_RATE  20
#define STARS_BATCH 6

//konfiguracja lowcow
#define MAX_HUNTERS     20
#define HUNTER_SPAWN_S  4

//konfiguracja albatrosa
#define ALBATROS_MAX_USES 3
#define ALBATROS_SAFE_DIST 8

//konfiguracja szarzy
#define HUNTER_DASH_SPEED 3
#define HUNTER_CHARGE_TIME 10
#define HUNTER_COOLDOWN 50
#define HUNTER_COLOR_ALERT 7

//Skalowanie trudności
#define TIME_STEP_SEC   10
#define HP_ADD_PER_STEP 1
#define BASE_HUNTER_HP  3

//Elitarni łowcy
#define ELITE_START_SEC 30
#define BASE_DAMAGE     1
#define ELITE_DAMAGE    2

//kolory
#define MAIN_COLOR   1
#define STAT_COLOR   2
#define PLAY_COLOR   3
#define BIRD_COLOR   4
#define HUNTER_COLOR 5
#define HUNTER_COLOR_WEAK   5
#define HUNTER_COLOR_STRONG 10

//kolory zdrowia
#define HP_COLOR_100 6
#define HP_COLOR_75 7
#define HP_COLOR_50 8
#define HP_COLOR_25 9

//wymiary
#define BORDER     1
#define OFFY       2
#define OFFX       5

//ranking
#define RANKING_FILE "ranking.txt"
#define TOP_N 5


typedef struct {
    WINDOW* window;
    int x, y;
    int rows, cols;
    int color;
} WIN;

typedef struct {
    int rows;
    int cols;
    int game_time;
    int star_goal;
    int hunter_count;
    int player_hp;
    float multiplier;   // Mnożnik punktów
    int color_theme;
    int star_spawn_rate;
    int hunter_spawn_sec;
    int damage_value;
    int fog_type;
    char name[20];
} LevelConfig;

typedef struct {
    WIN* win;
    int x, y;
    int dx, dy;
    int speed;
    char symbol;
    int color;
    int hp;
    int immune_timer;
    int albatros_uses;
} BIRD;

typedef struct {
    int x, y;
    int speed;
    int active;
    char symbol;
    int color;
}STAR;

typedef struct {
    int x, y;
    int dx, dy;
    int w, h;
    int hp;
    int active;
    char symbol;
    int damage;
    int color;
    int state;
    int timer;
    int cooldown;
} HUNTER;

typedef struct {
    char name[10];
    int score;
}RECORD;

void LoadLevel(int level_number, LevelConfig* config) {
   char filename[20];
   sprintf(filename, "level%d.txt", level_number);

   FILE *f = fopen(filename, "r");

   if (f == NULL) {
      printf("BLAD: Nie znaleziono pliku %s! Laduje ustawienia domyslne...\n", filename);
      sleep(2);
      config->rows = 20;
      config->cols = 60;
      config->game_time = 60;
      config->star_goal = 15;
      config->hunter_count = 5;
      config->player_hp = 5;
      config->multiplier = 1.0;
      config->color_theme = 2;
      config->star_spawn_rate = 20;
      config->hunter_spawn_sec = 4;
      config->damage_value = 1;
      config->fog_type = 0;
      strcpy(config->name, "Default");
      return;
   }

   fscanf(f, "%d %d %d %d %d %d %f %d %d %d %d %d %s",
         &config->rows,
         &config->cols,
         &config->game_time,
         &config->star_goal,
         &config->hunter_count,
         &config->player_hp,
         &config->multiplier,
         &config->color_theme,
         &config->star_spawn_rate,
         &config->hunter_spawn_sec,
         &config->damage_value,
         &config->fog_type,
         config->name);

   fclose(f);
}

void SelectLevel(LevelConfig* config) {
    int choice = 0;
    while (choice < 1 || choice > 5) {
       system("clear");
       printf("================================\n");
       printf("      WYBIERZ POZIOM TRUDNOSCI   \n");
       printf("================================\n");
       printf("1. BEGINNER (Spacer)\n");
       printf("2. EASY     (Latwy)\n");
       printf("3. MEDIUM   (Standard)\n");
       printf("4. HARD     (Trudny)\n");
       printf("5. EXTREME  (Tylko dla odwaznych)\n");
       printf("================================\n");
       printf("Wybor (1-5): ");

       if (scanf("%d", &choice) != 1) {
          while(getchar() != '\n');
          choice = 0;
       }
    }
    LoadLevel(choice, config);
}


WINDOW* Start()
{
    WINDOW* win;
    if ( (win = initscr()) == NULL ) {
       fprintf(stderr, "Błąd inicjalizacji ncurses.\n");
       exit(EXIT_FAILURE);
    }
    start_color();
    init_pair(MAIN_COLOR, COLOR_WHITE, COLOR_BLACK);
    init_pair(PLAY_COLOR, COLOR_CYAN, COLOR_BLACK);
    init_pair(STAT_COLOR, COLOR_YELLOW, COLOR_BLUE);
    init_pair(BIRD_COLOR, COLOR_RED, COLOR_BLACK);
    init_pair(HUNTER_COLOR_WEAK,   COLOR_MAGENTA, COLOR_BLACK);
    init_pair(HUNTER_COLOR_STRONG, COLOR_CYAN, COLOR_BLACK);
    init_pair(HP_COLOR_100, COLOR_GREEN, COLOR_BLACK);
    init_pair(HP_COLOR_75,  COLOR_YELLOW, COLOR_BLACK);
    init_pair(HP_COLOR_50,  COLOR_MAGENTA, COLOR_BLACK);
    init_pair(HP_COLOR_25,  COLOR_RED, COLOR_BLACK);
    noecho();
    curs_set(0);
    return win;
}

void CleanWin(WIN* W, int bo)
{
    int i, j;
    wattron(W->window, COLOR_PAIR(W->color));
    if (bo) box(W->window, 0, 0);
    for (i = bo; i < W->rows - bo; i++)
       for (j = bo; j < W->cols - bo; j++)
          mvwprintw(W->window, i, j, " ");
    wrefresh(W->window);
}

WIN* InitWin(WINDOW* parent, int rows, int cols, int y, int x, int color, int bo, int delay)
{
    WIN* W = (WIN*)malloc(sizeof(WIN));
    W->x = x;
    W->y = y;
    W->rows = rows;
    W->cols = cols;
    W->color = color;
    W->window = subwin(parent, rows, cols, y, x);
    CleanWin(W, bo);
    if (delay == 0) nodelay(W->window, TRUE);
    wrefresh(W->window);
    return W;
}

BIRD* InitBird(WIN* w, int x, int y, int dx, int dy, LevelConfig* config)
{
    BIRD* b = (BIRD*)malloc(sizeof(BIRD));
    b->win = w;
    b->x = x;
    b->y = y;
    b->dx = dx;
    b->dy = dy;
    b->speed = BIRD_SPEED;
    b->symbol = 'A';
    b->color = BIRD_COLOR;
    b->hp = config->player_hp;
    b->immune_timer = 0;
    b->albatros_uses = ALBATROS_MAX_USES;
    return b;
}

void DrawBird(BIRD* b, LevelConfig* config)
{
    if (b->immune_timer > 0) {
        if (b->immune_timer % 2 != 0) {
           wattron(b->win->window, COLOR_PAIR(b->win->color));
           mvwprintw(b->win->window,b->y,b->x, " ");
           return;
        }
    }

    int color_to_use;
    float percent = (float)b->hp / config->player_hp;

    if (percent > 0.75) {
       color_to_use = HP_COLOR_100;
    }
    else if (percent > 0.50) {
       color_to_use = HP_COLOR_75;
    }
    else if (percent > 0.25) {
       color_to_use = HP_COLOR_50;
    }
    else {
       color_to_use = HP_COLOR_25;
    }
    wattron(b->win->window, COLOR_PAIR(color_to_use));
    mvwprintw(b->win->window, b->y, b->x, "%c", b->symbol);
    wattron(b->win->window, COLOR_PAIR(b->win->color));
}

void ClearBird(BIRD* b)
{
    mvwprintw(b->win->window, b->y, b->x, " ");
}

void MoveBird(BIRD* b, LevelConfig* config) {
    ClearBird(b);
    int new_x = b->x + b->dx * b->speed;
    if (new_x <= BORDER) {
       b->x = BORDER;
    }
    else if (new_x >= b->win->cols - BORDER -1) {
       b->x = b->win->cols - BORDER -1;
    }
    else {
       b->x = new_x;
    }
    int new_y = b->y + b->dy * b->speed;
    if (new_y <= BORDER) {
       b->y = BORDER;
    }
    else if (new_y >= b->win->rows - BORDER -1) {
       b->y = b->win->rows - BORDER -1;
    }
    else {
       b->y = new_y;
    }
    DrawBird(b, config);
}

void moveUp(BIRD* b) {
    b->dx = 0; b->dy = -1;
}
void moveDown(BIRD* b) {
    b->dy = 1; b->dx = 0;
}
void moveLeft(BIRD* b) {
    b->dx = -1; b->dy = 0;
}
void moveRight(BIRD* b) {
    b->dx = 1; b->dy = 0;
}
void SpeedUp(BIRD*b) {
    if (b->speed <5) b->speed++;
}
void SpeedDown(BIRD*b) {
    if (b->speed > 1) b->speed--;
}

void InitStars(STAR stars[]) {
    for (int i=0; i<MAX_STARS;i++) {
       stars[i].active = 0;
       stars[i].symbol = '*';
    }
}

void SpawnStars(STAR stars[], int cols) {
    int spawned = 0;
    for (int i=0; i<MAX_STARS;i++) {
       if (!stars[i].active) {
          stars[i].active = 1;
          stars[i].y =1;
          stars[i].x = (rand() % (cols -2)) +1;
          stars[i].speed = (rand() % 2) + 1;
          spawned++;
          if (spawned >= STARS_BATCH) break;
       }
    }
}

void UpdateStars(STAR stars[], WIN* w) {
    wattron(w->window, COLOR_PAIR(w->color));
    for (int i = 0; i < MAX_STARS; i++) {
       if (stars[i].active) {
          mvwprintw(w->window, stars[i].y, stars[i].x, " ");
          stars[i].y += stars[i].speed;

          if (stars[i].y >= w->rows - 1) {
             stars[i].active = 0;
          } else {
             mvwprintw(w->window, stars[i].y, stars[i].x, "%c", stars[i].symbol);
          }
       }
    }
    wrefresh(w->window);
}

int CheckCollision(BIRD* b, STAR stars[]) {
    int points = 0;
    for (int i = 0; i < MAX_STARS; i++) {
       if (stars[i].active) {
          if (b->x == stars[i].x && abs(b->y - stars[i].y) <= 1) {
             mvwprintw(b->win->window,stars[i].y, stars[i].x, " ");
             stars[i].active = 0;
             points++;
          }
       }
    }
    return points;
}

void InitHunters(HUNTER hunters[]) {
    for (int i=0; i<MAX_HUNTERS;i++) {
       hunters[i].active = 0;
       hunters[i].symbol = '#';
       hunters[i].state = 0;
       hunters[i].timer = 0;
       hunters[i].cooldown = 0;
    }
}

void DrawHunter(WIN* w, HUNTER* h, int erase) {
    if (!h->active) return;

    char sym;
    if (erase) sym = ' ';
    else sym = '0' + h->hp;

    if (!erase) {
       if (h->state == 1) {
          wattron(w->window, COLOR_PAIR(HUNTER_COLOR_ALERT));
       } else {
          wattron(w->window, COLOR_PAIR(h->color));
       }
    }

    for(int i = 0; i < h->h; i++) {
       for(int j = 0; j < h->w; j++) {
          int draw_y = h->y + i;
          int draw_x = h->x + j;

          if (draw_y > 0 && draw_y < w->rows - 1 &&
             draw_x > 0 && draw_x < w->cols - 1) {
             mvwprintw(w->window, draw_y, draw_x, "%c", sym);
             }
       }
    }
    if (!erase) wattron(w->window, COLOR_PAIR(w->color));
}

void SpawnHunter(HUNTER hunters[], WIN* w, BIRD* b, int* total_spawned, int elapsed_time, LevelConfig* config) {
    if (*total_spawned >= config->hunter_count) return;

    for (int i=0; i<MAX_HUNTERS; i++) {
       if (!hunters[i].active) {
          hunters[i].active = 1;
          (*total_spawned)++;

          int difficulty_level = elapsed_time / TIME_STEP_SEC;
          hunters[i].hp = BASE_HUNTER_HP + (difficulty_level * HP_ADD_PER_STEP);

          if (elapsed_time >= ELITE_START_SEC) {
             hunters[i].damage = config->damage_value + 1;
             hunters[i].color  = HUNTER_COLOR_STRONG;
          } else {
             hunters[i].damage = config->damage_value;
             hunters[i].color  = HUNTER_COLOR_WEAK;
          }

          int shape_type = rand() % 5;
          switch(shape_type) {
             case 0: hunters[i].w = 1; hunters[i].h = 2; break;
             case 1: hunters[i].w = 2; hunters[i].h = 1; break;
             case 2: hunters[i].w = 1; hunters[i].h = 3; break;
             case 3: hunters[i].w = 3; hunters[i].h = 1; break;
             case 4: hunters[i].w = 2; hunters[i].h = 2; break;
          }

          int side = rand() % 2;
          hunters[i].y = (rand() % (w->rows - 6)) + 3;

          if (side == 0) hunters[i].x = BORDER + 1;
          else hunters[i].x = w->cols - BORDER - 1 - hunters[i].w;

          if (b->x > hunters[i].x) hunters[i].dx = 1;
          else if (b->x < hunters[i].x) hunters[i].dx = -1;
          else hunters[i].dx = 0;

          if (b->y > hunters[i].y) hunters[i].dy = 1;
          else if (b->y < hunters[i].y) hunters[i].dy = -1;
          else hunters[i].dy = 0;

          hunters[i].state = 0;
          hunters[i].timer = 0;
          hunters[i].cooldown = 0;
          break;
       }
    }
}

void HandleHunterPatrol(HUNTER* h, WIN* w, BIRD* b, int frame_counter) {
    int seen_x = (b->x >= h->x && b->x < h->x + h->w);
    int seen_y = (b->y >= h->y && b->y < h->y + h->h);

    if (h->cooldown == 0 && (seen_x || seen_y)) {
        h->state = 1;
        h->timer = HUNTER_CHARGE_TIME;
    } else {
        if (frame_counter % 2 == 0) {
            h->x += h->dx;
            h->y += h->dy;

            int hit_wall = 0;
            if (h->x <= 0 || h->x + h->w >= w->cols) {
                h->dx = -h->dx; h->x += h->dx; hit_wall = 1;
            }
            if (h->y <= 0 || h->y + h->h >= w->rows) {
                h->dy = -h->dy; h->y += h->dy; hit_wall = 1;
            }
            if (hit_wall) {
                h->hp--;
                if (h->hp <= 0) h->active = 0;
            }
        }
    }
}

void HandleHunterAiming(HUNTER* h, BIRD* b) {
    h->timer--;
    if (h->timer <= 0) {
        h->state = 2;
        h->dx = 0; h->dy = 0;
        if (b->x >= h->x && b->x < h->x + h->w) {
            if (b->y > h->y) h->dy = 1; else h->dy = -1;
        } else {
            if (b->x > h->x) h->dx = 1; else h->dx = -1;
        }
    }
}

void HandleHunterCharge(HUNTER* h, WIN* w, BIRD* b) {
    for (int s = 0; s < HUNTER_DASH_SPEED; s++) {
        h->x += h->dx;
        h->y += h->dy;

        if (b->immune_timer == 0) {
            if (b->x >= h->x && b->x < h->x + h->w &&
                b->y >= h->y && b->y < h->y + h->h) {
                b->hp -= h->damage;
                b->immune_timer = 20;
            }
        }

        if (h->x <= 0 || h->x + h->w >= w->cols ||
            h->y <= 0 || h->y + h->h >= w->rows) {
            h->x -= h->dx; h->y -= h->dy;
            if (h->dx != 0) h->dx = -h->dx;
            if (h->dy != 0) h->dy = -h->dy;
            h->hp--;
            h->state = 0;
            h->cooldown = HUNTER_COOLDOWN;
            if (h->hp <= 0) h->active = 0;
            break;
        }
    }
}

void UpdateHunters(HUNTER hunters[], WIN* w, BIRD* b, int frame_counter) {
    for (int i = 0; i < MAX_HUNTERS; i++) {
        if (hunters[i].active) {
            DrawHunter(w, &hunters[i], 1);
            if (hunters[i].cooldown > 0) hunters[i].cooldown--;

            if (hunters[i].state == 0) HandleHunterPatrol(&hunters[i], w, b, frame_counter);
            else if (hunters[i].state == 1) HandleHunterAiming(&hunters[i], b);
            else if (hunters[i].state == 2) HandleHunterCharge(&hunters[i], w, b);

            if (hunters[i].active) DrawHunter(w, &hunters[i], 0);
        }
    }
    wrefresh(w->window);
}

int CheckHunterCollision(BIRD* b, HUNTER hunters[]) {
    if (b->immune_timer > 0) return 0;
    int hit = 0;
    for(int i=0; i<MAX_HUNTERS; i++) {
       if(hunters[i].active) {
          if (b->x >= hunters[i].x && b->x < hunters[i].x + hunters[i].w &&
             b->y >= hunters[i].y && b->y < hunters[i].y + hunters[i].h) {
             hit = 1;
             b->hp -= hunters[i].damage;
             b->immune_timer = 10;
             }
       }
    }
    return hit;
}

void UseAlbatros(BIRD* b, HUNTER hunters[], int cols, int rows) {
    if (b->albatros_uses <=0) return;
    b->albatros_uses--;
    int best_x = cols;
    int best_y = rows;
    for (int attempt = 0; attempt < 10; attempt++) {
       int rand_x = (rand()%(cols - 4)) + 2;
       int rand_y = (rand()%(rows - 4)) + 2;
       int is_safe = 1;
       for (int i = 0; i<MAX_HUNTERS; i++) {
          if (hunters[i].active) {
             int dist_x = abs(rand_x - hunters[i].x);
             int dist_y = abs(rand_y - hunters[i].y);
             if (dist_x < ALBATROS_SAFE_DIST && dist_y < ALBATROS_SAFE_DIST) {
                is_safe = 0; break;
             }
          }
       }
       if (is_safe) {
          best_x = rand_x; best_y = rand_y; break;
       }
    }
    mvwprintw(b->win->window,b->y,b->x," ");
    b->x = best_x;
    b->y = best_y;
    b-> immune_timer = 30;
    b->dx = 0; b->dy = 0;
}

void DrawFog(WIN* w, BIRD* b, int time_left, LevelConfig* config) {
   if (config->fog_type == 0) return;

   float time_ratio = (float)time_left / config->game_time;
   int max_radius = w->cols;
   int min_radius = 4;
   int current_radius = min_radius + (int)((max_radius - min_radius) * time_ratio);

   for (int y = 1; y < w->rows - 1; y++) {
      for (int x = 1; x < w->cols - 1; x++) {
         int dist_x = abs(b->x - x);
         int dist_y = abs(b->y - y);

         if (dist_x > current_radius || dist_y > current_radius / 2) {
            wattron(w->window, COLOR_PAIR(MAIN_COLOR) | A_DIM);
            mvwprintw(w->window, y, x, "#");
            wattroff(w->window, A_DIM);
         }
         else {
            chtype current_char = mvwinch(w->window, y, x) & A_CHARTEXT;
            if (current_char == '#') {
               mvwprintw(w->window, y, x, " ");
            }
         }
      }
   }
}

void HandleRanking(WIN*w,int final_score) {
    RECORD records[100];
    int count = 0;
    FILE* f = fopen(RANKING_FILE, "r");
    if (f!=NULL) {
       while (fscanf(f,"%19s %d", records[count].name, &records[count].score) ==2) {
          count++;
          if (count >= 99) break;
       }
       fclose(f);
    }
    CleanWin(w,1);
    mvwprintw(w->window,2,2,"TWOJ WYNIK: %d", final_score);
    mvwprintw(w->window,4,2,"Wpisz imie: ");
    wrefresh(w->window);

    echo();
    curs_set(1);
    char player_name[10];
    wgetnstr(w->window,player_name,9);
    noecho();
    curs_set(0);

    strcpy(records[count].name, player_name);
    records[count].score = final_score;
    count++;

    for (int i =0; i<count -1; i++) {
       for (int j=0; j<count -i -1; j++) {
          if (records[j].score < records[j+1].score) {
             RECORD temp = records[j];
             records[j] = records[j+1];
             records[j+1] = temp;
          }
       }
    }
    f = fopen(RANKING_FILE, "w");
    if (f != NULL) {
       for (int i=0; i<count; i++) {
          fprintf(f,"%s %d\n", records[i].name, records[i].score);
       }
       fclose(f);
    }
    CleanWin(w,1);
    wattron(w->window,COLOR_PAIR(MAIN_COLOR));
    box(w->window,0,0);
    mvwprintw(w->window,1,2,"=== TOP 5 WYNIKOW ===");
    for (int i =0; i<TOP_N && i< count;i++) {
       mvwprintw(w->window,3+i,2,"%d. %s - %d pkt", i+1, records[i].name, records[i].score);
    }
    mvwprintw(w->window, 16, 2, "[Nacisnij cokolwiek, aby zakonczyc gre]");
    wrefresh(w->window);
    flushinp();
    wgetch(w->window);
}

void ShowStatus(WIN* W, BIRD* b, int time_left, int score, LevelConfig* config)
{
    wattron(W->window, COLOR_PAIR(W->color));
    box(W->window, 0, 0);
   mvwprintw(W->window, 1, 2, "[I/O] Spd:%d [T]=albatros taxi [Q]=quit",b->speed);
   mvwprintw(W->window, 2, 2, "Czas:%ds Gwiazdy:%d/%d HP:%d/%d",
              time_left,
              score,
              config->star_goal,
              b->hp,
              config->player_hp);
   mvwprintw(W->window, 3, 2, "LVL: %s | Spd:%d | Taxi:%d", config->name,b->speed, b->albatros_uses);
   wrefresh(W->window);
}

void EndGame(WIN* W, int reason, int stars_collected,int time_played,int hp_left, LevelConfig* config){
    nodelay(W->window, FALSE);
    CleanWin(W, 1);
    wattron(W->window, A_BOLD);
    if (reason == 1) mvwprintw(W->window, 2, 5, "KONIEC CZASU!");
    else if (reason == 2) mvwprintw(W->window, 2, 5, "PODDALES SIE!");
    else if (reason == 3) mvwprintw(W->window, 2, 5, "ZGINALES!");
    else if (reason == 4) {
       wattron(W->window, COLOR_PAIR(HP_COLOR_100));
       mvwprintw(W->window, 2, 5, "POZIOM UKONCZONY! BRAWO!");
       wattron(W->window, COLOR_PAIR(W->color));
    }
    wattroff(W->window, A_BOLD);

    int base_score = (stars_collected * 100) + (time_played * 10) + (hp_left * 200);
    int final_score = (int)(base_score * config->multiplier);

    if (final_score < 0) final_score = 0;

    mvwprintw(W->window, 4, 5, "Podstawa:    %d pkt", base_score);
    mvwprintw(W->window, 5, 5, "Mnoznik:     x%.1f (%s)", config->multiplier, config->name);
    mvwprintw(W->window, 7, 5, "-------------------------");
    mvwprintw(W->window, 9, 5, "WYNIK KONCOWY: %d", final_score);

    wattron(W->window, A_BLINK);
    mvwprintw(W->window, 15, 5, "[ Wcisnij ENTER aby przejsc dalej ]");
    wattroff(W->window, A_BLINK);

    wrefresh(W->window);
    flushinp();
    sleep(1);
    wgetch(W->window);

    HandleRanking(W, final_score);
}

void MainLoop(WIN* playwin, WIN* statwin, BIRD* bird, STAR stars[], HUNTER hunters[], LevelConfig* config)
{
    int ch;
    time_t start_time = time(NULL);
    int remaining_time = config->game_time;

    int frame_counter = 0;
    int hunter_timer = 0;
    int total_hunters_spawned = 0;

    int score = 0;

    int hunter_spawn_frames = config->hunter_spawn_sec * (1000 / FRAME_TIME);

    while (1)
    {
       ch = wgetch(statwin->window);

       if (ch == QUIT) {
          int elapsed = config->game_time - remaining_time;
          EndGame(playwin, 2, score, elapsed, bird->hp, config);
          break;
       }

       if (ch == K_U) moveUp(bird);
       if (ch == K_D) moveDown(bird);
       if (ch == K_L) moveLeft(bird);
       if (ch == K_R) moveRight(bird);
       if (ch == K_SPEED_UP) SpeedUp(bird);
       if (ch == K_SPEED_DOWN) SpeedDown(bird);
       if (ch == K_ALBATROS) {
           UseAlbatros(bird, hunters, playwin->cols, playwin->rows);
       }

       time_t current_time = time(NULL);
       remaining_time = config->game_time - (int)(current_time - start_time);

       if (remaining_time <= 0) {
           EndGame(playwin, 1, score, 0, bird->hp, config);
           break;
       }

       frame_counter++;
       hunter_timer++;

       if (frame_counter >= config->star_spawn_rate) {
          SpawnStars(stars, playwin->cols);
          frame_counter = 0;
       }

       int elapsed_seconds = (int)(current_time - start_time);
       if (hunter_timer >= hunter_spawn_frames) {
           SpawnHunter(hunters, playwin, bird, &total_hunters_spawned, elapsed_seconds, config);
           hunter_timer = 0;
       }

       if (frame_counter % 2 == 0) {
           UpdateStars(stars, playwin);
       }

       UpdateHunters(hunters, playwin, bird, frame_counter);

       MoveBird(bird, config);
       if (bird->immune_timer > 0) {
           bird->immune_timer--;
       }

       score += CheckCollision(bird, stars);

       if (score >= config->star_goal) {
           int elapsed = config->game_time - remaining_time;
           EndGame(playwin, 4, score, remaining_time, bird->hp, config);
           break;
       }

       CheckHunterCollision(bird, hunters);

       if (bird->hp <= 0) {
           int elapsed = config->game_time - remaining_time;
           EndGame(playwin, 3, score, elapsed, 0, config);
           break;
       }

       ShowStatus(statwin, bird, remaining_time, score, config);
       DrawFog(playwin, bird, remaining_time, config);

       wrefresh(playwin->window);
       flushinp();
       usleep(FRAME_TIME * 1000);
    }
}


int main()
{
    srand(time(NULL));

    LevelConfig current_level;

    SelectLevel(&current_level);

    WINDOW *mainwin = Start();

    WIN* playwin = InitWin(mainwin, current_level.rows, current_level.cols, OFFY, OFFX, PLAY_COLOR, BORDER, 0);
    WIN* statwin = InitWin(mainwin, 5, current_level.cols, current_level.rows + OFFY, OFFX, STAT_COLOR, BORDER, 0);

    BIRD* bird = InitBird(playwin, current_level.cols/2, current_level.rows/2, 0, 0, &current_level);
    STAR stars[MAX_STARS];
    InitStars(stars);

    HUNTER hunters[MAX_HUNTERS];
    InitHunters(hunters);

    DrawBird(bird, &current_level);
    ShowStatus(statwin, bird, current_level.game_time, 0, &current_level);
    wrefresh(playwin->window);

    MainLoop(playwin, statwin, bird, stars, hunters, &current_level);

    delwin(playwin->window);
    delwin(statwin->window);
    delwin(mainwin);

    endwin();
    refresh();

    free(bird);
    free(playwin);
    free(statwin);

    return EXIT_SUCCESS;
}