#include <stdio.h>
#include <stdlib.h>
typedef struct
{
    unsigned int rows;
    unsigned int columns;
    char ***data;
} DatabaseResult;

int main(){

DatabaseResult result;
    unsigned int row =4;
    result.rows = row;
    result.columns = 5;
    
    result.data = calloc(result.rows + 1, sizeof(char **));
    for(int i = 0; i < row; i++){
        result.data[i] = calloc(result.columns + 1, sizeof(char*));
    }
    result.data[0][0] = "test";
    printf(" %s", result.data[3][4]);
    return 0;
}