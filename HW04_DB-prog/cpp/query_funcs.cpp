#include "query_funcs.h"
#include <sstream>

void execute(connection *C, string sql) {
  work W(*C);
  W.exec(sql);
  W.commit();
}

void add_player(connection *C, int team_id, int jersey_num, string first_name,
                string last_name, int mpg, int ppg, int rpg, int apg,
                double spg, double bpg) {
  work W(*C);
  stringstream sql;
  sql << "INSERT INTO player (team_id, uniform_num, first_name, last_name, "
         "mpg, ppg, rpg, apg, spg, bpg) VALUES ("
      << team_id << ", " << jersey_num << ", " << W.quote(first_name) << ", "
      << W.quote(last_name) << ", " << mpg << ", " << ppg << ", " << rpg << ", "
      << apg << ", " << spg << ", " << bpg << "); ";
  W.exec(sql.str());
  W.commit();
}

void add_team(connection *C, string name, int state_id, int color_id, int wins,
              int losses) {
  stringstream sql;
  sql << "INSERT INTO team (name, state_id, color_id, wins, losses) VALUES ("
      << work(*C).quote(name) << ", " << state_id << ", " << color_id << ", "
      << wins << ", " << losses << "); ";
  execute(C, sql.str());
}

void add_state(connection *C, string name) {
  string sql =
      "INSERT INTO state (name) VALUES (" + work(*C).quote(name) + "); ";
  execute(C, sql);
}

void add_color(connection *C, string name) {
  string sql =
      "INSERT INTO color (name) VALUES (" + work(*C).quote(name) + "); ";
  execute(C, sql);
}

void query1(connection *C, int use_mpg, int min_mpg, int max_mpg, int use_ppg,
            int min_ppg, int max_ppg, int use_rpg, int min_rpg, int max_rpg,
            int use_apg, int min_apg, int max_apg, int use_spg, double min_spg,
            double max_spg, int use_bpg, double min_bpg, double max_bpg) {
  stringstream sql;
  sql << "SELECT * FROM PLAYER";
  bool flag = false;

  if (use_mpg) {
    if (!flag)
      sql << " WHERE ";
    else
      sql << " AND ";
    sql << "(mpg between " << min_mpg << " AND " << max_mpg << ") ";
    flag = true;
  }

  if (use_ppg) {
    if (!flag)
      sql << " WHERE ";
    else
      sql << " AND ";
    sql << "(ppg between " << min_ppg << " AND " << max_ppg << ") ";
    flag = true;
  }

  if (use_rpg) {
    if (!flag)
      sql << " WHERE ";
    else
      sql << " AND ";
    sql << "(rpg between " << min_rpg << " AND " << max_rpg << ") ";
    flag = true;
  }

  if (use_apg) {
    if (!flag)
      sql << " WHERE ";
    else
      sql << " AND ";
    sql << "(apg between " << min_apg << " AND " << max_apg << ") ";
    flag = true;
  }

  if (use_spg) {
    if (!flag)
      sql << " WHERE ";
    else
      sql << " AND ";
    sql << "(spg between " << min_spg << " AND " << max_spg << ") ";
    flag = true;
  }

  if (use_bpg) {
    if (!flag)
      sql << " WHERE ";
    else
      sql << " AND ";
    sql << "(bpg between " << min_bpg << " AND " << max_bpg << ") ";
    flag = true;
  }

  sql << ";";

  /* Create a non-transactional object. */
  nontransaction N(*C);
  //  work W(*C);
  /* Execute SQL query */
  result R(N.exec(sql.str()));

  /* List down all the records */
  cout << "PLAYER_ID TEAM_ID UNIFORM_NUM FIRST_NAME LAST_NAME MPG PPG RPG APG "
          "SPG BPG\n";
  for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
    cout << c[0].as<int>() << " " << c[1].as<int>() << " " << c[2].as<int>()
         << " " << c[3].as<string>() << " " << c[4].as<string>() << " "
         << c[5].as<int>() << " " << c[6].as<int>() << " " << c[7].as<int>()
         << " " << c[8].as<int>() << " " << c[9].as<double>() << " "
         << c[10].as<double>() << "\n";
  }
  // W.commit();
}

void query2(connection *C, string team_color) {
  work W(*C);
  string sql = "SELECT TEAM.name FROM TEAM, COLOR where team.color_id = "
               "color.color_id and color.name = " +
               W.quote(team_color) + ";";
  W.commit();
  nontransaction N(*C);
  result R(N.exec(sql));
  cout << "NAME\n";
  for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
    cout << c[0].as<string>() << "\n";
  }
}

void query3(connection *C, string team_name) {
  work W(*C);
  string sql = "SELECT player.first_name, player.last_name FROM player, team "
               "where player.team_id = team.team_id and team.name = " +
               W.quote(team_name) + " ORDER BY ppg desc;";
  W.commit();
  nontransaction N(*C);
  result R(N.exec(sql));
  cout << "FIRST_NAME LAST_NAME\n";
  for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
    cout << c[0].as<string>() << " " << c[1].as<string>() << "\n";
  }
}

void query4(connection *C, string team_state, string team_color) {
  work W(*C);
  string sql =
      "select player.first_name, player.last_name, player.uniform_num"
      " from player, team, color, state where "
      "player.team_id = team.team_id AND team.color_id = color.color_id AND "
      "team.state_id = state.state_id AND state.name = " +
      W.quote(team_state) + " AND color.name = " + W.quote(team_color) + ";";
  W.commit();
  nontransaction N(*C);
  result R(N.exec(sql));
  cout << "FIRST_NAME LAST_NAME JERSEY_NUM\n";
  for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
    cout << c[0].as<string>() << " " << c[1].as<string>() << " "
         << c[2].as<int>() << "\n";
  }
}

void query5(connection *C, int num_wins) {
  work W(*C);
  stringstream sql;
  sql << "select player.first_name, player.last_name, team.name, team.wins "
         "from player, team where player.team_id = team.team_id and team.wins "
         "> "
      << num_wins << ";";
  W.commit();
  nontransaction N(*C);
  result R(N.exec(sql.str()));
  cout << "FIRST_NAME LAST_NAME TEAM_NAME WINS\n";
  for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
    cout << c[0].as<string>() << " " << c[1].as<string>() << " "
         << c[2].as<string>() << " " << c[3].as<int>() << "\n";
  }
}
