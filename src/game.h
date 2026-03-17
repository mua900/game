#ifndef _GAME_H
#define _GAME_H

#include "common.h"
#include "template.h"

#include "input.h"

#include "box2d/box2d.h"

enum GameObjectType {
    GOT_Wall,
    GOT_Player,
    GOT_Enemy,
};

struct Wall {
    vec2 scale;

    Wall(vec2 scale)
        : scale(scale)
    {}
};

struct Player {
    vec2 velocity = {};
    float speed = 0;
    float rotation = {};
};

struct Enemy {
    vec2 velocity = {};
    float rotation = {};
};

struct GameObject {
    GameObjectType type;
    vec2 position;

    union {
        Player player;
        Enemy enemy;
        Wall wall;
    };

    GameObject()
    {}
    GameObject(vec2 position, Wall& wall)
        : type(GOT_Wall), position(position), wall(wall)
    {}
    GameObject(vec2 position, Wall&& wall)
        : type(GOT_Wall), position(position), wall(wall)
    {}
    GameObject(vec2 position, Player& player)
        : type(GOT_Player), position(position), player(player)
    {}
    GameObject(vec2 position, Enemy& enemy)
        : type(GOT_Enemy), position(position), enemy(enemy)
    {}
};

inline int spatial_hash(vec2 pos)
{
    return int(floor(pos.x) * 962623) ^ int(floor(pos.y) * 1193771);
}

#define OVERFLOW_CELL_INDEX_SENTINEL -1
#define CELL_CAPACITY 8
struct SpatialGridCell {
    int objects[CELL_CAPACITY];
    int count;
    int overflow_cell;
};

struct SpatialGrid {
    SpatialGridCell* cells;
    int dimension_x;
    int dimension_y;
    DArray<SpatialGridCell> overflow_cells;

    void initialize(int dim_x, int dim_y)
    {
        dimension_x = dim_x;
        dimension_y = dim_y;

        cells = new SpatialGridCell[dim_x * dim_y];
    }

    int size()
    {
        return dimension_x * dimension_y;
    }

    int calculate_cell_index(vec2 position)
    {
        int hash = spatial_hash(position);
        hash = abs(hash);
        hash %= size();
        return hash;
    }

    void add(vec2 position, int object)
    {
        int cell_index = calculate_cell_index(position);
        add_to_cell(cells[cell_index], object);
    }

    void remove(vec2 position, int object)
    {
        int cell_index = calculate_cell_index(position);
        remove_from_cell(cells[cell_index], object);
    }

    // if we want the entries to be unique we would need to check all of them which we can do in a separate function as an opt in way
    void add_to_cell(SpatialGridCell& cell, int object)
    {
        if (cell.count == CELL_CAPACITY)
        {
            if (cell.overflow_cell == OVERFLOW_CELL_INDEX_SENTINEL)
            {
                int oc = overflow_cells.add(SpatialGridCell());
                cell.overflow_cell = oc;
            }

            add_to_cell(overflow_cells.get_ref(cell.overflow_cell), object);
            return;
        }

        cell.objects[cell.count] = object;
        cell.count += 1;
    }

    void remove_from_cell(SpatialGridCell& cell, int object)
    {
        for (int i = 0; i < cell.count; i++)
        {
            if (cell.objects[i] == object)
            {
                cell.objects[i] = cell.objects[cell.count - 1];
                cell.count -= 1;
                return;
            }
        }

        if (cell.overflow_cell != OVERFLOW_CELL_INDEX_SENTINEL)
        {
            remove_from_cell(overflow_cells.get_ref(cell.overflow_cell), object);
        }
    }
};

struct GameState {
    BucketList<GameObject> game_objects = {};
    SpatialGrid grid = {};

    b2WorldId worldId = {};
    DArray<b2BodyId> bodies = {};

    bool initialize();
    void cleanup();
    void update(double delta_time, const Input& input);
    void fixed_update(int ticks_per_second);  // dt is 1 / tps

    void add_object(GameObject& obj)
    {
        int object_index = game_objects.add(obj);
        grid.add(obj.position, object_index);
    }

    void remove_object(int object)
    {
        vec2 pos = game_objects.get(object).position;
        game_objects.remove(object);
        grid.remove(pos, object);
    }
};

#endif // _GAME_H