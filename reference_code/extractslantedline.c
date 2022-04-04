#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "resources/left_slanted_walls.h"

int main(){
    for(int i = 0; i < 320 ; i++){
        for(int j = 0; j < 240 ; j++){
            if(left_slanted_walls[i][j] != 0 ){
                int x = i + 7;
                printf("{%d, %d} \n", &x, &j);
            }
        }
    }
}