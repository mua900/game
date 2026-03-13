#ifndef _GAME_H
#define _GAME_H

#include "common.h"
#include "template.h"

enum GameObjectType {
    OT_Wall,
    OT_Player,
    OT_Enemy,
};

struct Wall {
    vec2 scale;
};

struct Player {
    float rotation;
};

struct Enemy {
    float rotation;
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
    BucketList<GameObject> game_objects;
    SpatialGrid grid;

    bool initialize();
    void update(double delta_time);
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