# Generated by Django 2.1.7 on 2019-03-16 22:09

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('ncaa', '0001_initial'),
    ]

    operations = [
        migrations.AlterField(
            model_name='color',
            name='color_id',
            field=models.AutoField(primary_key=True, serialize=False),
        ),
        migrations.AlterField(
            model_name='player',
            name='player_id',
            field=models.AutoField(primary_key=True, serialize=False),
        ),
        migrations.AlterField(
            model_name='state',
            name='state_id',
            field=models.AutoField(primary_key=True, serialize=False),
        ),
        migrations.AlterField(
            model_name='team',
            name='team_id',
            field=models.AutoField(primary_key=True, serialize=False),
        ),
    ]