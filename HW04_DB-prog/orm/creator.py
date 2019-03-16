#!/usr/bin/env python3
import os
os.environ.setdefault("DJANGO_SETTINGS_MODULE", "orm.settings")

import django
if django.VERSION >= (1, 7):#自动判断版本
    django.setup()
    
def createColor():
    from ncaa.models import Color
    f = open('data/color.txt')
    ColorList = []
    for line in f:
       color_id, name= line.split(' ')
       color = Color(color_id=color_id, name = name)
       ColorList.append(color)
       print("color name: ", name)
    f.close()
    Color.objects.bulk_create(ColorList)
    return

def createState():
    from ncaa.models import State
    f = open('data/state.txt')
    StateList = []
    for line in f:
        state_id, name= line.split(' ')
        state = State(state_id=state_id, name = name)
        StateList.append(state)
    f.close()
    State.objects.bulk_create(StateList)
    return

def createTeam():
    from ncaa.models import Team, State, Color
    f = open('data/team.txt')
    TeamList = []
    for line in f:
        team_id, name, state_id, color_id, wins, losses= line.split(' ')
        team = Team(team_id = team_id, name = name, state_id = State.objects.get(state_id = state_id), color_id = Color.objects.get(color_id = color_id), wins = wins, losses = losses)
        TeamList.append(team)
    f.close()
    Team.objects.bulk_create(TeamList)
    return

def createPlayer():
    from ncaa.models import Player, Team
    f = open('data/player.txt')
    PlayerList = []
    for line in f:
        player_id, team_id, uniform_num, first_name, last_name, mpg, ppg, rpg, apg, spg, bpg= line.split(' ')
        player = Player(player_id=player_id, team_id=Team.objects.get(team_id = team_id), uniform_num=uniform_num, first_name=first_name, last_name=last_name, mpg=mpg, ppg=ppg, rpg=rpg, apg=apg, spg=spg, bpg=bpg)
        PlayerList.append(player)
    f.close()
    Player.objects.bulk_create(PlayerList)
    return

def main():
    createColor()
    createState()
    createTeam()
    createPlayer()
    
 
if __name__ == "__main__":
    main()
    print('Done creating tables!')