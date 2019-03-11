#include "exerciser.h"

void exercise(connection *C)
{
  // select * from player where (mpg between 35 and 40) and (rpg between 8 and 10);
  query1(C, 1, 35, 40, 0, 0, 0, 0, 5, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  query2(C, "Orange");
  query3(C, "FloridaState");
  query3(C, "Duke");
}
