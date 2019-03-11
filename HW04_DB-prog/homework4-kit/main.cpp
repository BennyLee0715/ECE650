#include <iostream>
#include <pqxx/pqxx>
#include <string>
#include <fstream>
#include <sstream>

#include "exerciser.h"

using namespace std;
using namespace pqxx;

connection *buildConn()
{
  //Parameters: database name, user name, user password
  connection *C = new connection("dbname=acc_bball user=postgres password=passw0rd");
  if (C->is_open())
  {
    cout << "Opened database successfully: " << C->dbname() << endl;
  }
  else
  {
    cout << "Can't open database" << endl;
  }
  return C;
}

void buildTables(string filename, connection *C)
{
  /* Create SQL statement */
  string sql, tmp;
  std::ifstream ifs;
  ifs.open(filename.c_str(), std::ifstream::in);
  while (std::getline(ifs, tmp))
  {
    sql += tmp;
  }
  ifs.close();

  /* Create a transactional object. */
  work W(*C);

  /* Execute SQL query */
  W.exec(sql);
  W.commit();
  cout << "Table created successfully" << endl;
}

void buildState(string filename, connection *C)
{
  string state_id, name, tmp;
  std::ifstream ifs;
  ifs.open(filename.c_str(), std::ifstream::in);
  while (std::getline(ifs, tmp))
  {
    stringstream ss;
    ss << tmp;
    ss >> state_id >> name;
    add_player(C, name);
  }
  ifs.close();
}

void buildColor(string filename, connection *C)
{
  string color_id, name, tmp;
  std::ifstream ifs;
  ifs.open(filename.c_str(), std::ifstream::in);
  while (std::getline(ifs, tmp))
  {
    stringstream ss;
    ss << tmp;
    ss >> color_id >> name;
    add_color(C, name);
  }
  ifs.close();
}

void buildTeam(string filename, connection *C)
{
  string sql, tmp, name;
  int team_id, state_id, color_id, wins, losses;
  std::ifstream ifs;
  ifs.open(filename.c_str(), std::ifstream::in);
  while (std::getline(ifs, tmp))
  {
    stringstream ss;
    ss << tmp;
    ss >> team_id >> name >> state_id >> color_id >> wins >> losses;
    add_team(C, name, state_id, color_id, wins, losses);
  }
  ifs.close();
}

void buildPlayer(string filename, connection *C)
{
  string tmp, first_name, last_name;
  int player_id, team_id, uniform_num, mpg, ppg, rpg, apg;
  double spg, bpg;
  std::ifstream ifs;
  ifs.open(filename.c_str(), std::ifstream::in);
  while (std::getline(ifs, tmp))
  {
    stringstream ss;
    ss << tmp;
    ss >> player_id >> team_id >> uniform_num >> first_name >> last_name >> mpg >> ppg >> rpg >> apg >> spg >> bpg;
    add_player(C, team_id, uniform_num, first_name, last_name, mpg, ppg, rpg, apg, spg, bpg);
  }
  ifs.close();
}

int main(int argc, char *argv[])
{

  try
  {
    //Allocate & initialize a Postgres connection object
    //Establish a connection to the database
    connection *C = buildConn();

    //TODO 1: create PLAYER, TEAM, STATE, and COLOR tables in the ACC_BBALL database
    buildTables("creation.sql", C);

    //TODO 2: load each table with rows from the provided source txt files
    buildState("state.txt", C);
    buildColor("color.txt", C);
    buildTeam("team.txt", C);
    buildPlayer("player.txt", C);

    // Test
    exercise(C);

    //Close database connection
    C->disconnect();
  }
  catch (const std::exception &e)
  {
    cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}
