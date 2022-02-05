#include "game_level.h"

#include <fstream>
#include <sstream>


void GameLevel::Load(const char *file, unsigned int levelWidth, unsigned int levelHeight)
{
    // clear old data
    this->Bricks.clear();
    // load from file
    unsigned int tileCode;
    GameLevel level;
    std::string line;
    std::ifstream fstream(file);
    std::vector<std::vector<unsigned int>> tileData;
    if (fstream)
    {
        while (std::getline(fstream, line)) // read each line from level file
        {
            std::istringstream sstream(line);
            std::vector<unsigned int> row;
            while (sstream >> tileCode) // read each word separated by spaces
                row.push_back(tileCode);
            tileData.push_back(row);
        }
        if (tileData.size() > 0)
            this->init(tileData, levelWidth, levelHeight);
    }
}

void GameLevel::Draw(SpriteRenderer &renderer)
{
    for (GameObject &tile : this->Bricks)
        if (!tile.Destroyed)
            tile.Draw(renderer);
}

bool GameLevel::IsCompleted()
{
    for (GameObject &tile : this->Bricks)
        if (!tile.IsSolid && !tile.Destroyed)
            return false;
    return true;
}

void GameLevel::init(std::vector<std::vector<unsigned int>> tileData, unsigned int levelWidth, unsigned int levelHeight)
{
    // calculate dimensions
    unsigned int height = tileData.size();
    unsigned int width = tileData[0].size(); // note we can index vector at [0] since this function is only called if height > 0
    float unit_width = levelWidth / static_cast<float>(width), unit_height = levelHeight / height; 
    // initialize level tiles based on tileData		
    int enemy_count = 0;

    for (unsigned int y = 0; y < height; ++y)
    {
        for (unsigned int x = 0; x < width; ++x)
        {
            // check block type from level data (2D level array)
            if (tileData[y][x] == 1) //wall
            {
                glm::vec2 pos((100.0f * x), (100.0f * y));
                glm::vec2 size(100.0f, 100.0f);
                GameObject obj(pos, size, ResourceManager::GetTexture("border"), glm::vec3(1.0f, 1.0f, 1.0f));
                obj.IsSolid = true;
                obj.IsEnemy = false;
                obj.IsCoin = false;
                obj.IsPlayer = false;
                this->Bricks.push_back(obj);
            }
            else if(tileData[y][x] == 2){   // coins
                glm::vec2 pos((100.0f * x), (100.0f * y));
                glm::vec2 size(60.0f, 60.0f);
                GameObject obj(pos, size, ResourceManager::GetTexture("thunder"), glm::vec3(1.0f, 1.0f, 1.0f));
                obj.IsSolid = false;
                obj.IsEnemy = false;
                obj.IsCoin = true;
                obj.IsPlayer = false;
                this->Bricks.push_back(obj);
            }
            else if(tileData[y][x] == 3){   // enemy
                glm::vec2 pos((100.0f * x), (100.0f * y));
                glm::vec2 size(100.0f, 100.0f);
                float x,y;
                enemy_count++;
                x = 30 + (enemy_count * 10);
                y = 100 - (enemy_count * 10);
                int r = rand() % 2;
                if(r)y*=-1;

                glm::vec2 velocity(x, y);
                GameObject obj(pos, size, ResourceManager::GetTexture("enemy2"), glm::vec3(1.0f, 1.0f, 1.0f), velocity);
                obj.IsSolid = false;
                obj.IsEnemy = true;
                obj.IsCoin = false;
                obj.IsPlayer = false;
                this->Bricks.push_back(obj);
            }
            else if(tileData[y][x] == 6){
                glm::vec2 pos((100.0f * x), (100.0f * y));
                glm::vec2 size(100.0f, 100.0f);
                GameObject obj(pos, size, ResourceManager::GetTexture("door"), glm::vec3(1.0f, 1.0f, 1.0f));
                obj.IsSolid = true;
                obj.IsEnemy = false;
                obj.IsCoin = true;
                obj.IsPlayer = false;
                this->Bricks.push_back(obj);
            }
           
        }
    }
    
}