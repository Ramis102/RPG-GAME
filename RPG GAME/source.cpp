#include <iostream>
#include "SFML/Graphics.hpp"
#include "SFML/Audio.hpp"
#include <fstream>
using namespace std;
using namespace sf;

class VisualAsset;
class Character;
class Player;
class Enemy;
class Weapon;
class Projectile;
class Level;

class Helper
{
public:
    static int StringLength(const char* str)
    {
        int length = 0;

        for (int i = 0; str[i] != '\0'; i++)
        {
            length++;
        }

        return length;
    }
    static void StringCopy(char*& dest, const char*& src)
    {
        int i = 0;

        while (src[i] != '\0')
        {
            dest[i] = src[i];
            i++;
        }

        dest[i] = '\0';
    }
    static char* GetStringFromBuffer(const char* str)
    {
        int size = StringLength(str);

        char* newTemp = new char[size + 1];
        StringCopy(newTemp, str);

        return newTemp;
    }
};

class VisualAsset
{
protected:
    RenderWindow* window;   //pointer to window in main
    Sprite sprite;
    Vector2f* spriteDimensions;     //holds dimensions of every array of sprite textures. Dimensions are constant throughout an array and variable across arrays
    Vector2f spawnPoint;    //initial location of every visual asset
    Vector2f textureCuttingPoint;   //the point where function IntRect must cut every .png file before creating texture
    Vector2i currTexture;   //the current texture being used on display. X coordinate represents column on textures array and Y coordinate represents row
    Texture** textures;     //holds rows of textures. eg. row1: Idle animation textures, row2: Run animation Textures
    int* textureArraySizes; //holds number of textures per array in textures matrix
    int totalTextureArrays; //number of total texture arrays in textures matrix
public:
    VisualAsset(RenderWindow* ptr)
    {
        window = ptr;
        spriteDimensions = nullptr;
        textures = nullptr;
        textureArraySizes = nullptr;
    }
    ~VisualAsset()
    {
        if (spriteDimensions)
            delete[] spriteDimensions;
        if (textures)
        {
            for (int i = 0; i < textureArraySizes[i]; i++)
                delete[] textures[i];
            delete[] textures;
        }
        if (textureArraySizes)
            delete[] textureArraySizes;
    }
    void LoadTextures(char** fileNames)
    {
        textures = new Texture * [totalTextureArrays];
        for (int i = 0; i < totalTextureArrays; i++)
        {
            textures[i] = new Texture[textureArraySizes[i]];
            for (int j = 0; j < textureArraySizes[i] && j < 9; j++)    //remove j < 9 later
            {
                int size = Helper::StringLength(fileNames[i]);
                fileNames[i][size - 5] = j + 1 + '0';
                if (!textures[i][j].loadFromFile(fileNames[i], IntRect(textureCuttingPoint.x, textureCuttingPoint.y, spriteDimensions[i].x, spriteDimensions[i].y)))
                {
                    cout << "Error occured while loading texture...\n";
                }
            }
        }
    }
    void LoadVisualAssetFromFile(istream& fin)
    {
        fin >> textureCuttingPoint.x; fin >> textureCuttingPoint.y;
        fin >> spawnPoint.x; fin >> spawnPoint.y;

        fin >> totalTextureArrays;
        textureArraySizes = new int[totalTextureArrays];
        spriteDimensions = new Vector2f[totalTextureArrays];

        for (int i = 0; i < totalTextureArrays; i++)
        {
            fin >> textureArraySizes[i];
        }
        for (int i = 0; i < totalTextureArrays; i++)
        {
            fin >> spriteDimensions[i].x; fin >> spriteDimensions[i].y;
        }

        char** fileNames = new char* [totalTextureArrays];
        char buffer[100];
        fin.getline(buffer, 100);
        for (int i = 0; i < totalTextureArrays; i++)
        {
            fin.getline(buffer, 100);
            fileNames[i] = Helper::GetStringFromBuffer(buffer);
        }
        LoadTextures(fileNames);
    }
    void Initialize()
    {
        currTexture = Vector2i(0.f, 0.f);
        sprite.setTexture(textures[currTexture.y][currTexture.x]);
        sprite.setPosition(spawnPoint);
    }
    void Move(Vector2f direction, float distance, Vector2f bounds, bool isFlipped)
    {
        Vector2f currPos = sprite.getPosition();
        Vector2f displacement = Vector2f(distance * direction.x, distance * direction.y);
        currPos = currPos + displacement;
        if (!isFlipped)
        {
            if (currPos.x < 0 || currPos.x > bounds.x - spriteDimensions[currTexture.y].x || currPos.y < 0 || currPos.y + spriteDimensions[currTexture.y].y)
            {
                sprite.setPosition(currPos);
            }
        }
        else
        {
            if (currPos.x < spriteDimensions[currTexture.y].x || currPos.x > bounds.x || currPos.y < 0 || currPos.y + spriteDimensions[currTexture.y].y)
            {
                sprite.setPosition(currPos);
            }
        }
    }
    void Flip()
    {
        Vector2f orientation = sprite.getScale();
        orientation.x *= -1;
        sprite.setScale(orientation);
        if (orientation.x < 0)
        {
            sprite.setPosition(sprite.getPosition() + Vector2f(spriteDimensions[currTexture.y].x, 0.f));
        }
        else
        {
            sprite.setPosition(sprite.getPosition() - Vector2f(spriteDimensions[currTexture.y].x, 0.f));
        }
    }
    bool Intersection(VisualAsset& object)
    {
        if (sprite.getGlobalBounds().intersects(object.sprite.getGlobalBounds()))
            return true;
        return false;
    }
    void Animate()
    {
        //sprite.setTexture(textures[currTexture.y][currTexture.x]);
        currTexture.x++;
        if (currTexture.x > textureArraySizes[currTexture.y] || currTexture.x > 9)
            currTexture.x = 0;
    }
    void Display(float deltaTime)
    {
        static float totalTime = 0;
        totalTime += deltaTime;
        if (totalTime >= 5.0 / 60.0)
        {
            currTexture.x++;
            if (currTexture.x > 9)
                currTexture.x = 0;
            totalTime = 0;
        }
        sprite.setTexture(textures[currTexture.y][currTexture.x]);
        sprite.setTextureRect(IntRect(0, 0, 50, 45));
        window->draw(sprite);
        currTexture.y = 0;
    }

};

class MyPlayer : public VisualAsset
{
private:
    char* playerID;
    int maxSpeed;
    int speed;
    int acceleration;
    int health;
public:
    MyPlayer(RenderWindow* windowPtr) : VisualAsset(windowPtr)
    {
        currTexture = Vector2i(0.f, 0.f);
        playerID = nullptr;
    }
    ~MyPlayer()
    {
        if (playerID)
            delete[] playerID;
    }
    void LoadFromFile(istream& fin)
    {
        LoadVisualAssetFromFile(fin);
        char buffer[100];

        fin.getline(buffer, 100);
        playerID = Helper::GetStringFromBuffer(buffer);
        fin >> speed;
        fin >> acceleration;
        fin >> maxSpeed;
        fin >> health;
        Initialize();
    }
    void Run(Vector2f direction, Vector2f bounds, bool isFlipped)
    {
        currTexture.y = 1;  //second row of textures matrix
        Move(direction, speed, bounds, isFlipped);
    }
    void Update(Vector2f gameBounds)
    {
        static bool isFlipped = false;
        if (Keyboard::isKeyPressed(Keyboard::D))
        {
            if (isFlipped)
            {
                Flip();
                isFlipped = false;
            }
            Run(Vector2f(1.f, 0.f), gameBounds, isFlipped);
        }
        else if (Keyboard::isKeyPressed(Keyboard::A))
        {

            if (!isFlipped)
            {
                Flip();
                isFlipped = true;
            }
            Run(Vector2f(-1.f, 0.f), gameBounds, isFlipped);
        }

    }

};

class Enemy : public VisualAsset
{
private:
    char* enemyType;
public:
    Enemy(RenderWindow* ptr) : VisualAsset(ptr)
    {

    }
    ~Enemy()
    {
        if (enemyType)
            delete[] enemyType;
    }
};

class Weapon : VisualAsset
{
private:

};

class Projectile : public VisualAsset
{
private:
    Texture* textures;
public:
};

class Level
{
private:
    Enemy** enemies;
    int enemyCount;
public:

};


class Game
{
private:
    Vector2f gameBounds;
    RenderWindow* window;
    MyPlayer* player;
public:
    Game(Vector2f bounds, RenderWindow* ptr)
    {
        gameBounds = bounds;
        window = ptr;
        player = nullptr;
    }
    ~Game()
    {
        if (player)
            delete player;
    }
    void Load()
    {
        fstream fin;
        fin.open("Assets/GameData/PlayerInfo.txt", ios::in);
        if (!fin)
            cout << "eror\n";

        player = new MyPlayer(window);
        player->LoadFromFile(fin);
    }
    void Update()
    {
        player->Update(gameBounds);
    }
    void Run(float deltaTime)
    {
        //Time time = milliseconds(1000);
       // static float totalTime = 0;
        //totalTime += deltaTime;
        //if (totalTime >= 5.0 / 60.0)
        //{
        //   player->Animate();
        //   
        //    totalTime = 0;
        //}
        player->Display(deltaTime);
        //sleep(time);
    }



};

int main()
{
    RenderWindow window(VideoMode(900, 600), "Test Game", Style::Default);
    window.setVerticalSyncEnabled(true);
    Cursor cursor;
    cursor.loadFromSystem(sf::Cursor::Arrow);
    window.setMouseCursor(cursor);
    Game Platformer(Vector2f(900, 600), &window);
    Platformer.Load();

    float deltaTime = 0;
    Clock clock;
    while (window.isOpen())
    {
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                window.close();
        }

        deltaTime = clock.restart().asSeconds();
        Platformer.Update();

        window.clear();
        Platformer.Run(deltaTime);
        window.display();
    }
}