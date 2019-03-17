#!/usr/bin/env python3
import os
os.environ.setdefault("DJANGO_SETTINGS_MODULE", "orm.settings")

import django
if django.VERSION >= (1, 7):#自动判断版本
    django.setup()
    
from ncaa.models import Player, Team, State, Color

def add_player(team_id, uniform_num, first_name, last_name, mpg, ppg, rpg, apg, spg, bpg):
   Player.objects.get_or_create(team_id=Team.objects.get(team_id = team_id), uniform_num=uniform_num, first_name=first_name, last_name=last_name, mpg=mpg, ppg=ppg, rpg=rpg, apg=apg, spg=spg, bpg=bpg)

def add_color(name):
    Color.objects.get_or_create(name = name)

def add_state(name):
    State.objects.get_or_create(name = name)

def add_team(name, state_id, color_id, wins, losses):
    Team.objects.get_or_create(name = name, state_id = State.objects.get(state_id = state_id), color_id = Color.objects.get(color_id = color_id), wins = wins, losses = losses)

def query1(use_mpg,  min_mpg,  max_mpg, use_ppg,  min_ppg,  max_ppg, use_rpg,  min_rpg,  max_rpg, use_apg,  min_apg,  max_apg, use_spg,  min_spg,  max_spg, use_bpg,  min_bpg,  max_bpg):
    rslt = Player.objects.all()
    if use_mpg:
        rslt = rslt.filter(mpg__lte = max_mpg, mpg__gte = min_mpg)
    if use_ppg:
       rslt = rslt.filter(ppg__lte = max_ppg, ppg__gte = min_ppg)
    if use_rpg:
       rslt = rslt.filter(rpg__lte = max_rpg, rpg__gte = min_rpg)
    if use_apg:
       rslt = rslt.filter(apg__lte = max_apg, apg__gte = min_apg)
    if use_spg:
       rslt = rslt.filter(spg__lte = max_spg, spg__gte = min_spg)
    if use_bpg:
       rslt = rslt.filter(bpg__lte = max_bpg, bpg__gte = min_bpg)
    print("PLAYER_ID TEAM_ID UNIFORM_NUM FIRST_NAME LAST_NAME MPG PPG RPG APG SPG BPG")
    for player in rslt:
        print(player)

    
def query2(teamcolor):
    rslt = Team.objects.filter(color_id__name = teamcolor)
    print("NAME")
    for team in rslt:
        print(team.name)
    return

def query3(team_name):
    rslt = Player.objects.filter(team_id__name = team_name).order_by('-ppg').values_list("first_name", "last_name")
    print("FIRST_NAME LAST_NAME")
    for player in rslt:
        print(player[0], player[1])
    return

def query4(team_state, team_color):
    rslt = Player.objects.filter(team_id__state_id__name = team_state, team_id__color_id__name = team_color).reverse().values_list("first_name", "last_name", "uniform_num")
    print("FIRST_NAME LAST_NAME UNIFORM_NUM")
    for player in rslt:
        print(player[0], player[1], player[2])
    return

def query5(num_wins):
    rslt = Player.objects.filter(team_id__wins__gt = num_wins)
    print("FIRST_NAME LAST_NAME TEAM_NAME WINS")
    for player in rslt:
        print(player.first_name, player.last_name, player.team_id.name, player.team_id.wins)
    return

    
def main():
    query1(1, 35, 40, 0, 0, 0, 0, 5, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0)
    query1(0, 35, 40, 0, 0, 0, 1, 5, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0)
    query1(0, 35, 40, 0, 0, 0, 0, 5, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0)
    query2("Orange")
    query3("FloridaState")
    query3("Duke")
    query4("NC", "Red")
    query4("NC", "DarkBlue")
    query5(13)
    add_player(1, 1, "yyyy", "xxxx", 1, 1, 1, 1, 1, 1);
    query1(0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
    query2("b")
    query3("haha")
    query4("NC", "Red")
    query5(5)
    add_player(1, 60, "xxxxx", "yyyy", 20, 20, 10, 10, 5.3, 5.3)
    add_team("team_baldwin", 10, 3, 20, 0)
    add_state("N/A")
    add_color("perry-winkle")
    query1(1, 10, 30, 0, 0, 40, 0, 0, 6, 0, 0, 5, 0, 0.0, 10.0, 0, 0.0, 10.0)
    query2("Gold")
    query3("FloridaState")
    query4("FL", "Maroon")
    query5(8)

if __name__ == "__main__":
    main()
