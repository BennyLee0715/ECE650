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
    return 1;
  }
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
  work W(C);

  /* Execute SQL query */
  W.exec(sql);
  W.commit();
  cout << "Table created successfully" << endl;
}

void buildState(string filename, connection *C) {
  string sql, tmp;
  std::ifstream ifs;
  ifs.open(filename.c_str(), std::ifstream::in);
  while (std::getline(ifs, tmp))
  {
    stringstream ss;
    ss << tmp;
    int state_id;
    string name;
    ss >> state_id >> name;
    ss.str("");
    ss << "INSERT INTO state (state_id, name) VALUES (" << state_id << ", '" << name << "'); "
    sql = ss.str();
    W.exec(sql);
    W.commit();
  }
  ifs.close();
}

void buildTeam(string filename, connection *C) {
  
}

void buildPlayer(string filename, connection *C) {
  
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
