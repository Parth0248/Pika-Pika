#include "game.h"
#include "resource_manager.h"
#include "sprite_renderer.h"
#include "game_object.h"
#include "text_renderer.h"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <time.h>
#include <vector>
using namespace std;
// Game-related State data
SpriteRenderer *Renderer;
GameObject *Player;
//  GameObject *Enemy;

float ENEMY_VELOCITY = 125.0f;

// collision detection
bool CheckCollision(GameObject &one, GameObject &two);
Collision CheckCollis(GameObject &one, GameObject &two);
Direction VectorDirection(glm::vec2 closest);


Game::Game(unsigned int width, unsigned int height)
    : State(GAME_ACTIVE), Keys(), Width(width), Height(height)
{
    Lightoff = 0;
}

Game::~Game()
{
    delete Renderer;
    delete Player;
}

void gen () {
    srand(time(NULL));
  ofstream myfile;
  int thunder = 0, wall = 0 , enemy = 0 ;
  for(int i = 1; i <= 3; i++) {
    if(i == 1){
        myfile.open ("./levels/one.lvl");
        thunder = 3;
        wall = 14;
        enemy = 10;
    }
    else if(i == 2){
        myfile.open ("./levels/two.lvl");
        thunder = 4;
        wall = 16;
        enemy = 5;
    }
    else if(i == 3){
        myfile.open ("./levels/three.lvl");
        thunder = 5;
        wall = 17;
        enemy = 5;
    }
    int xcd = 10, ycd = 10;
    for(int j = 0;j < ycd;j++){
        for(int k = 0;k<xcd;k++){
           
            
            if((j == 0 || k == 0 || j == ycd - 1 || k == xcd - 1) || (j == 1 && k == 0 || j == 1 && k == 1 ))
            {
                if(j == 1 && k == 0){
                    myfile << "6 ";    
                }
                else if(j == 1 && k == 1){
                    myfile << "0 ";
                }
                else{
                    myfile << "1 ";
                }
            }
            else{
                int p = rand() % 10 , s = rand() % 10;

                int r = (p + s) % 5; 
                if((j == 5 || j == 6) && k == 8 ) r = 0;
                if(wall == 0)r =0;
                if(thunder == 0)r = 0;
                if(enemy == 0)r = 0;

                if(r == 0 || r > 3)myfile << "0 ";
                else if(r == 1){myfile << "1 ";wall--;}
                else if(r == 2){myfile << "2 ";thunder--;}
                else if(r == 3){myfile << "3 ";enemy--;}
            }
            // else if(r == 4){myfile << "4 ";}
        }
        myfile << endl;
    }
  myfile.close();
  }
}
TextRenderer *Text;
void Game::Init()
{
    // load shaders
    this->begintime = clock();
    // gen();
    ResourceManager::LoadShader("shaders/sprite.vs", "shaders/sprite.frag", nullptr, "sprite");
    Text = new TextRenderer(this->Width, this->Height);
    Text->Load("fonts/ocraext.TTF", 24);
    // configure shaders
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->Width),
                                      static_cast<float>(this->Height), 0.0f, -1.0f, 1.0f);
    ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
    ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
    // set render-specific controls
    Renderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));
    // load textures
    ResourceManager::LoadTexture("textures/grass1.jpeg", false, "background0");
    ResourceManager::LoadTexture("textures/sand1.jpeg", false, "background1");
    ResourceManager::LoadTexture("textures/icyterrain.jpeg", false, "background2");
    ResourceManager::LoadTexture("textures/grassterrain.jpeg", false, "background3");
    ResourceManager::LoadTexture("textures/go.jpg", false, "background4");
    ResourceManager::LoadTexture("textures/thunderbolt.png", true, "thunder");
    ResourceManager::LoadTexture("textures/rock.png", true, "border");
    ResourceManager::LoadTexture("textures/greatball.png", true, "door");
    ResourceManager::LoadTexture("textures/pika.png", true, "paddle");
    ResourceManager::LoadTexture("textures/onix.png", true, "enemy2");
    ResourceManager::LoadTexture("textures/gyarados.png", true, "enemy1");
    ResourceManager::LoadTexture("textures/ggwp.png", true, "background5");
    // load levels
    GameLevel one;
    one.Load("levels/one.lvl", 1000, 1000);
    GameLevel two; two.Load("levels/two.lvl", 1000, 1000);
    GameLevel three; three.Load("levels/three.lvl", 1000, 1000);
    this->Levels.push_back(one);
    this->Levels.push_back(two);
    this->Levels.push_back(three);
    this->Level = 0;
    // configure game objects
    glm::vec2 playerPos = glm::vec2(this->Width / 2.0f - PLAYER_SIZE.x / 2.0f, 800.0f);
    // Player->IsPlayer = true;
    Player = new GameObject(playerPos, PLAYER_SIZE, ResourceManager::GetTexture("paddle"));
}


void Game::Update(float dt)
{
    if (this->State == GAME_ACTIVE)
    {

        // checks for collisions
        ResourceManager::GetShader("sprite").Use().SetInteger("Lightoff", this->Lightoff);
        ResourceManager::GetShader("sprite").Use().SetVector2f("PlayerPosition", Player->Position);
        this->DoCollisions();

        for (GameObject &box : this->Levels[this->Level].Bricks)
        {
            if (box.IsEnemy)
            {
                box.Move(dt, this->Width);
            }
            if(CheckCollision(*Player, box) && box.IsEnemy)
            {
                cout << "collision" << endl;
                this->State = GAME_OVER;
                // this->Level = 4;
                // this->ResetLevel();
            }
            if(CheckCollision(*Player,box) && box.IsCoin && box.IsSolid){
                this->Level++;
                if(this->Level == 3){
                    cout << "Game Won!\n";
                    this->State = GAME_WIN;
                }
                else{
                    cout << this->Level << endl;
                    this->NextLevel();
                    this->ResetPlayer();
                }
            }
        }
    
    }
}

void Game::ProcessInput(float dt)
{
    Player->dx = 0;
    Player->dy = 0;
    // player movement
    if (this->State == GAME_ACTIVE)
    {
        float velocity = PLAYER_VELOCITY * dt;
        // move playerboard

        if (this->Keys[GLFW_KEY_A])
        {
            if (Player->Position.x >= 0.0f)
                Player->Position.x -= velocity;
            Player->dx -= velocity;
        }
        if (this->Keys[GLFW_KEY_D])
        {
            if (Player->Position.x <= this->Width - Player->Size.x)
                Player->Position.x += velocity;
            Player->dx += velocity;
        }
        if (this->Keys[GLFW_KEY_W])
        {
            if (Player->Position.y >= 0.0f)
                Player->Position.y -= velocity;
            Player->dy -= velocity;
        }
        if (this->Keys[GLFW_KEY_S])
        {
            if (Player->Position.y <= this->Height - Player->Size.y)
                Player->Position.y += velocity;
            Player->dy += velocity;
        }
    }
}

void Game::ResetLevel()
{
    gen();
    this->Lives = 3;
    if (this->Level == 0)
        this->Levels[0].Load("levels/one.lvl", this->Width, this->Height / 2);
    else if (this->Level == 1)
        this->Levels[1].Load("levels/two.lvl", this->Width, this->Height / 2);
    else if (this->Level == 2)
        this->Levels[2].Load("levels/three.lvl", this->Width, this->Height / 2);
}

void Game::NextLevel(){
    if (this->Level == 0)
        this->Levels[0].Load("levels/one.lvl", this->Width, this->Height / 2);
    else if (this->Level == 1)
        this->Levels[1].Load("levels/two.lvl", this->Width, this->Height / 2);
    else if (this->Level == 2)
        this->Levels[2].Load("levels/three.lvl", this->Width, this->Height / 2);
}

void Game::ResetPlayer()
{
    // reset player/ball stats
    Player->Size = PLAYER_SIZE;
    Player->Position = glm::vec2(this->Width / 2.0f - PLAYER_SIZE.x / 2.0f, 800.0f );
}

void Game::Render()
{
    if (this->State == GAME_ACTIVE)
    {
        // draw background
        if(this->Level == 0)
            Renderer->DrawSprite(ResourceManager::GetTexture("background0"), glm::vec2(0.0f, 0.0f), glm::vec2(1000, 1000), 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        else if(this->Level == 1)
            Renderer->DrawSprite(ResourceManager::GetTexture("background1"), glm::vec2(0.0f, 0.0f), glm::vec2(1000, 1000), 0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
        else if(this->Level == 2)
            Renderer->DrawSprite(ResourceManager::GetTexture("background2"), glm::vec2(0.0f, 0.0f), glm::vec2(1000, 1000), 0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
        else if(this->Level == 3)
            Renderer->DrawSprite(ResourceManager::GetTexture("background3"), glm::vec2(0.0f, 0.0f), glm::vec2(1000, 1000), 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        // draw level
        this->Levels[this->Level].Draw(*Renderer);
        // draw player
        Player->Draw(*Renderer);
        // draw door
        // Renderer->DrawSprite(ResourceManager::GetTexture("door"), glm::vec2(0.0f, 100.0f), glm::vec2(100, 100), 0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
        string liv = to_string(this->Level + 1); 
        string scr = to_string(this->Score); 
        Text->RenderText("Level: " + liv, 50.0f, 5.0f, 1.25f);
        Text->RenderText("Score: " + scr ,800.0f, 5.0f, 1.25f);

        string time = to_string((clock() - this->begintime) * 5/ CLOCKS_PER_SEC );
        Text->RenderText("Time: " + time ,450.0f, 5.0f, 1.25f);
    }
    else if (this->State == GAME_OVER)
    {
        Renderer->DrawSprite(ResourceManager::GetTexture("background4"), glm::vec2(0.0f, 0.0f), glm::vec2(1000.0f, 1000.0f), 0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
    }
    else if(this->State == GAME_WIN){
        Renderer->DrawSprite(ResourceManager::GetTexture("background5"), glm::vec2(0.0f, 0.0f), glm::vec2(1000.0f, 1000.0f), 0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
    }
}

bool CheckCollision(GameObject &one, GameObject &two) // AABB - AABB collision
{
    // collision x-axis?
    bool collisionX = one.Position.x + one.Size.x * 0.9 >= two.Position.x && two.Position.x + two.Size.x * 0.9 >= one.Position.x;
    // collision y-axis?
    bool collisionY = one.Position.y + one.Size.y * 0.9 >= two.Position.y &&
                      two.Position.y + two.Size.y * 0.9 >= one.Position.y;
    // collision only if on both axes
    return collisionX && collisionY;
}

int count = 0;

void Game::DoCollisions()
{
    // checking collision with player and coin or wall
    for (GameObject &box : this->Levels[this->Level].Bricks)
    {
        if (!box.Destroyed)
        {
            if (CheckCollision(*Player, box))
            {
                if (box.IsCoin && !box.IsSolid)
                {
                    box.Destroyed = true;
                    if(Lightoff == 1)
                    {
                        this->Score += 20;
                    }
                    this->Score += 15;  // if(LightOff == 1)
                }
                else if (box.IsEnemy)
                {
                    // cout << "You lose!" << endl;
                    // this->Lives--;
                    
                }
                else if(box.IsCoin && box.IsSolid){ // next level
                    
                }
                else
                {
                    // cout << "collision detected with wall: " << count++ << endl;
                    // cout << Player->dx << " " << Player->dy << endl;
                    Player->Position.x -= Player->dx;
                    Player->Position.y -= Player->dy;
                }
            }
        }
    }

    // checking collision of enemy with wall and other
    for (GameObject &enm : this->Levels[this->Level].Bricks)
    {
        if (enm.IsEnemy)
        {
            for (GameObject &box : this->Levels[this->Level].Bricks)
            {
                if (box.Position != enm.Position && CheckCollision(enm,box))
                {
                    Collision result = CheckCollis(box, enm);

                    if (box.IsWall || box.IsCoin || box.IsEnemy)
                    {

                        Direction dir = get<1>(result);

                        glm::vec2 diff_vector = get<2>(result);

                        if (dir == LEFT || dir == RIGHT) // horizontal collision
                        {
                            // relocate
                            enm.Velocity.x = -enm.Velocity.x; // reverse vertical velocity

                            if (dir == RIGHT)
                                enm.Position.x += ((box.Position.x + box.Size.x) - enm.Position.x); // move player to right
                            else
                                enm.Position.x -= ((enm.Position.x + enm.Size.x) - box.Position.x); // move player to left
                        }
                        else // vertical collision
                        {
                            enm.Velocity.y = -enm.Velocity.y; // reverse vertical velocity

                            if (dir == UP)
                                enm.Position.y += ((box.Position.y + box.Size.y) - enm.Position.y); // move player back up
                            else
                                enm.Position.y -= ((enm.Position.y + enm.Size.y) - box.Position.y);
                            // move player back down
                        }
                    }
                }
            }
        }
    }
}

Collision CheckCollis(GameObject &one, GameObject &two) // AABB - Circle collision
{
    // get center point circle first
    glm::vec2 center(one.Position);
    // calculate AABB info (center, half-extents)
    glm::vec2 aabb_half_extents(two.Size.x / 2.0f, two.Size.y / 2.0f);
    glm::vec2 aabb_center(two.Position.x, two.Position.y);
    // get difference vector between both centers
    glm::vec2 difference = center - aabb_center;
    glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
    // now that we know the the clamped values, add this to AABB_center and we get the value of box closest to circle
    glm::vec2 closest = aabb_center + clamped;
    // now retrieve vector between center circle and closest point AABB and check if length < radius
    difference = closest - center;

    if (glm::length(difference) < one.Size.x) // not <= since in that case a collision also occurs when object one exactly touches object two, which they are at the end of each collision resolution stage.
        return std::make_tuple(true, VectorDirection(difference), difference);
    else
        return std::make_tuple(false, UP, glm::vec2(0.0f, 0.0f));
}

// calculates which direction a vector is facing (N,E,S or W)
Direction VectorDirection(glm::vec2 target)
{
    glm::vec2 compass[] = {
        glm::vec2(0.0f, 1.0f),  // up
        glm::vec2(1.0f, 0.0f),  // right
        glm::vec2(0.0f, -1.0f), // down
        glm::vec2(-1.0f, 0.0f)  // left
    };
    float max = 0.0f;
    unsigned int best_match = -1;
    for (unsigned int i = 0; i < 4; i++)
    {
        float dot_product = glm::dot(glm::normalize(target), compass[i]);
        if (dot_product > max)
        {
            max = dot_product;
            best_match = i;
        }
    }
    return (Direction)best_match;
}