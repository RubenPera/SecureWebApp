#include "Header.h"

DatabaseResult init_DatabaseResult(unsigned int rows, unsigned int columns)
{
    DatabaseResult result;

    result.rows = rows;
    result.columns = columns;
    result.data = calloc(result.rows + 1, sizeof(char **));
    for (int i = 0; i < result.rows; i++)
    {
        result.data[i] = calloc(result.columns + 1, sizeof(char *));
    }
    return result;
}

char *get_DatabaseResult(DatabaseResult result, unsigned int row, unsigned int column)
{
    if (row > (result.rows - 1))
    {
        kore_log(2, "Provided row is out of range");
        return NULL;
    }
    else if (column > (result.columns - 1))
    {
        kore_log(2, "Provided column is out of range");
        return NULL;
    }
    else
    {
        return result.data[row][column];
    }
}
void set_DatabaseResult(DatabaseResult result, unsigned int row, unsigned int column, char *value)
{
    if (row > (result.rows - 1))
    {
        kore_log(2, "Provided row is out of range");
    }
    else if (column > (result.columns - 1))
    {
        kore_log(2, "Provided column is out of range");
    }
    else
    {
        result.data[row][column] = value;
    }
}