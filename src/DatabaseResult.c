#include "Header.h"

DatabaseResult init_DatabaseResult(unsigned int rows, unsigned int columns){
    DatabaseResult result;

    result.rows = rows;
    result.columns = columns;
    result.data = (char ***)malloc((rows + 1) * sizeof(char**));

    return result;
}

