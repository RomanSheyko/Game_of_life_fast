//
//  main.c
//  Game_of_life_fast
//
//  Created by Роман on 26/12/2019.
//  Copyright © 2019 Roman. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>

#define CELL_ROWS 640
#define CELL_COLUMNS 640
#define PROPABILITY_OF_LIFE 1

typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned short int ushort;

const uchar _CELLS[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
const ulong _ALLOCATED_MEMORY_IN_USE = ((CELL_ROWS*CELL_COLUMNS)>>3);
const uint _CALCULATED_COLUMNS = ((CELL_COLUMNS)>>3);
const uint _CALCULATED_HEX_COLUMNS = ((CELL_COLUMNS)>>4);

void set_alife(uchar* area, uint i, uint j)
{
    area[(i>>3) + j*_CALCULATED_COLUMNS] |= _CELLS[i - (i & 0xfffffff8)];
}

void set_dead(uchar* area, uint i, uint j)
{
    area[(i>>3) + j*_CALCULATED_COLUMNS] &= ~(_CELLS[i - (i & 0xfffffff8)]);
}

void set_own(uchar *area, uint i, uint j, uchar val)
{
    if(val == 1) area[(i>>3) + j*_CALCULATED_COLUMNS] |= _CELLS[i - (i & 0xfffffff8)];
    else area[(i>>3) + j*_CALCULATED_COLUMNS] &= ~(_CELLS[i - (i & 0xfffffff8)]);
}

uchar isAlife(uchar* area, uint i, uint j)
{
    if(area[(i>>3) + j*_CALCULATED_COLUMNS] & _CELLS[i - (i & 0xfffffff8)]) return 1;
    return 0;
}

uchar check(uchar *area, int i, int j)
{
    if(i >= CELL_COLUMNS) i = 0;
    else if(i < 0) i = CELL_COLUMNS - 1;
    if(j >= CELL_ROWS) j = 0;
    else if(j < 0) j = CELL_ROWS - 1;
    return isAlife(area, (uint)i, (uint)j);
}

/*
void set_all_dead_sse(uchar* area)
{
    int i;
    asm(".intel_syntax noprefix\n"
        "mov rsi, 0");
    asm(".intel_syntax noprefix\n"
        "mov rbx, qword ptr [rbp - 0x8]");
    for(i = 0; i < (_ALLOCATED_MEMORY_IN_USE>>4); i++)
    {
        asm(".intel_syntax noprefix\n"
            "movdqu xmm0, [rbx+rsi]");
        asm(".intel_syntax noprefix\n"
            "xorps xmm0, xmm0");
        asm(".intel_syntax noprefix\n"
            "movdqu [rbx+rsi], xmm0");
        asm(".intel_syntax noprefix\n"
            "add rsi, 16");
    }
}
 */

void set_all_dead(uchar* area)
{
    int i;
    for(i = 0; i < _ALLOCATED_MEMORY_IN_USE; i++)
        area[i] = 0;
}

void show(uchar* area)
{
    uint i, j;
    for(i = 0; i < CELL_ROWS; i++)
    {
        for(j = 0; j < CELL_COLUMNS; j++)
            printf("%d", isAlife(area, j, i));
        printf("\n");
    }
}

void generation(uchar *prev_generation, uchar *current_generation)
{
    int i, j;
    uchar sum;
    for(i = 0; i < CELL_ROWS; i++)
        for(j = 0; j < CELL_COLUMNS; j++)
        {
            sum = check(prev_generation, j-1, i-1) + check(prev_generation, j-1, i) + check(prev_generation, j-1, i+1) + check(prev_generation, j, i-1) + check(prev_generation, j, i+1) + check(prev_generation, j+1, i-1) + check(prev_generation, j+1, i) + check(prev_generation, j+1, i+1);
            
            if(sum == 3 || (isAlife(prev_generation, j, i) && sum == 2))
                set_alife(current_generation, j, i);
            else set_dead(current_generation, j, i);
        }
}

void generation_v_2(ushort *prev_generation, uchar *current_generation)
{
    srand(time(NULL));
    int i, j, k, scale, up_row, down_row;
    char prop;
    uchar sum, added_bits_u, added_bits_c, added_bits_l, current_bit;
    unsigned short int r = 0xe994;
    register unsigned short upper_row, center_row, lower_row;
    for(i = 0; i < CELL_ROWS; i++)
    {
        for(j = 0; j < _CALCULATED_HEX_COLUMNS; j++)
        {
            up_row = (i == 0) ? (CELL_ROWS - 1) : (i-1);
            down_row = (i == CELL_ROWS - 1) ? 0 : (i+1);
            upper_row = prev_generation[up_row*_CALCULATED_HEX_COLUMNS + j];
            center_row = prev_generation[i*_CALCULATED_HEX_COLUMNS + j];
            lower_row = prev_generation[down_row*_CALCULATED_HEX_COLUMNS + j];
            if(j != 0)
            {
                added_bits_u = prev_generation[up_row*_CALCULATED_HEX_COLUMNS + j-1]>>15;
                added_bits_c = prev_generation[i*_CALCULATED_HEX_COLUMNS + j-1]>>15;
                added_bits_l = prev_generation[down_row*_CALCULATED_HEX_COLUMNS + j-1]>>15;
            } else {
                added_bits_u = prev_generation[up_row*_CALCULATED_HEX_COLUMNS + _CALCULATED_HEX_COLUMNS-1]>>15;
                added_bits_c = prev_generation[i*_CALCULATED_HEX_COLUMNS + _CALCULATED_HEX_COLUMNS-1]>>15;
                added_bits_l = prev_generation[down_row*_CALCULATED_HEX_COLUMNS + _CALCULATED_HEX_COLUMNS-1]>>15;
            }
            current_bit = center_row&1;
            sum = ((r >> ((upper_row & 3) << 1)) & 3) + added_bits_u + ((r >> ((center_row & 2) << 1)) & 3) + added_bits_c + ((r >> ((lower_row & 3) << 1)) & 3) + added_bits_l;
            if(sum == 3 || (current_bit == 1 && sum == 2))
                set_alife(current_generation, j<<4, i);
            else set_dead(current_generation, j<<4, i);
            for(k = 1; k < 15; k++)
            {
                prop = rand()%100;
                current_bit = (center_row&2) >> 1;
                sum = ((r >> ((upper_row & 7) << 1)) & 3) + ((r >> ((center_row & 5) << 1)) & 3) + ((r >> ((lower_row & 7) << 1)) & 3);
                upper_row >>= 1;
                center_row >>= 1;
                lower_row >>= 1;
                
                scale = (j<<4)+k;
                if((sum == 3 || (current_bit == 1 && sum == 2)) && (prop < PROPABILITY_OF_LIFE*100))
                    set_alife(current_generation, scale, i);
                else set_dead(current_generation, scale, i);
            }
            if(j != _CALCULATED_HEX_COLUMNS - 1)
            {
                added_bits_u = prev_generation[up_row*_CALCULATED_HEX_COLUMNS + j+1]&1;
                added_bits_c = prev_generation[i*_CALCULATED_HEX_COLUMNS + j+1]&1;
                added_bits_l = prev_generation[down_row*_CALCULATED_HEX_COLUMNS + j+1]&1;
            } else {
                added_bits_u = prev_generation[up_row*_CALCULATED_HEX_COLUMNS]&1;
                added_bits_c = prev_generation[i*_CALCULATED_HEX_COLUMNS]&1;
                added_bits_l = prev_generation[down_row*_CALCULATED_HEX_COLUMNS]&1;
            }
            current_bit = center_row>>1;
            sum = ((r >> ((upper_row & 3) << 1)) & 3) + added_bits_u + ((r >> ((center_row & 1) << 1)) & 3) + added_bits_c + ((r >> ((lower_row & 3) << 1)) & 3) + added_bits_l;
            if((sum == 3 || (current_bit == 1 && sum == 2)) && (prop < PROPABILITY_OF_LIFE*100))
                set_alife(current_generation, (j<<4)+15, i);
            else set_dead(current_generation, (j<<4)+15, i);
        }
    }
}

int main(int argc, const char * argv[]) {
    uchar cell_area[_ALLOCATED_MEMORY_IN_USE];
    uchar previous_generation[_ALLOCATED_MEMORY_IN_USE];
    uchar *cur_gen = cell_area;
    uchar *next_gen = previous_generation;
    set_all_dead(cell_area);
    set_all_dead(previous_generation);
    /*
    set_alife(cell_area, 390, 303);
    set_alife(cell_area, 391, 303);
    set_alife(cell_area, 392, 303);
    set_alife(cell_area, 392, 302);
    set_alife(cell_area, 391, 301);
    int num_of_generations;
    clock_t begin = clock();
    
    for(num_of_generations = 0; num_of_generations < 300; ++num_of_generations)
    {
        generation_v_2(cur_gen, next_gen);
        uchar *tmp = cur_gen;
        cur_gen = next_gen;
        next_gen = tmp;
    }
    
    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("%f\n", time_spent);
    //show(cur_gen);
    */
    
    //GUI
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *win = SDL_CreateWindow("Game of life", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, CELL_COLUMNS, CELL_ROWS, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    const int FRAMES_PER_SECOND = 30;
    int i, j;
    srand(time(NULL));
    for(i = 0; i < _ALLOCATED_MEMORY_IN_USE; i++)
        cur_gen[i] = rand()%255;
    for(i = 0; i < CELL_ROWS; i++)
        for(j = 0; j < CELL_COLUMNS; j++)
        {
            if(isAlife(cur_gen, j, i) == 1)
            {
                SDL_RenderDrawPoint(renderer, j, i);
            }
        }
    SDL_RenderPresent(renderer);
    Uint32 start;
    char running = 1;
    clock_t begin;
    clock_t end;
    double time_spent;
    while(running != 0)
    {
        start = SDL_GetTicks();
        begin = clock();
        SDL_Event e;
        while(SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = 0;
            }
        }
        generation_v_2(cur_gen, next_gen);
        uchar *tmp = cur_gen;
        cur_gen = next_gen;
        next_gen = tmp;
        for(i = 0; i < CELL_ROWS; i++)
            for(j = 0; j < CELL_COLUMNS; j++)
            {
                if(isAlife(cur_gen, j, i) == 1)
                {
                    SDL_SetRenderDrawColor(renderer, 150, 255, 100, 255);
                    SDL_RenderDrawPoint(renderer, j, i);
                } else {
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                    SDL_RenderDrawPoint(renderer, j, i);
                }
            }
        SDL_RenderPresent(renderer);
        
        end = clock();
        time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
        printf("%f\n", 1/time_spent);
        
        if(1000/FRAMES_PER_SECOND > SDL_GetTicks() - start)
            SDL_Delay(1000/FRAMES_PER_SECOND - (SDL_GetTicks() - start));
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();
    
    return 0;
}
