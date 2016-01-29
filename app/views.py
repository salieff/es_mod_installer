# -*- coding: utf-8 -*-

from flask import render_template, redirect, url_for, request
from app import app
from config import JSON_INDEX_PATH

primary_persons = [{'alias': "semen", 'name': "Семён"},
                   {'alias': "alisa", 'name': "Алиса"},
                   {'alias': "lena", 'name': "Лена"},
                   {'alias': "miku", 'name': "Мику"},
                   {'alias': "pioneer", 'name': "Пионер"},
                   {'alias': "slavya", 'name': "Славя"},
                   {'alias': "ulyana", 'name': "Ульяна"},
                   {'alias': "yulya", 'name': "Юля"}]

secondary_persons = [{'alias': "viola", 'name': "Виола"},
                     {'alias': "jenya", 'name': "Женя"},
                     {'alias': "tolik", 'name': "Толик"},
                     {'alias': "shurik", 'name': "Шурик"},
                     {'alias': "elektronik", 'name': "Электроник"},
                     {'alias': "olga", 'name': "Ольга Дмитриевна"}]

person_names = []

@app.route('/')
@app.route('/index')
def index():
    return render_template("index.html", primary_persons = primary_persons, secondary_persons = secondary_persons, json_path = JSON_INDEX_PATH)

@app.route('/sendpersons', methods = ['GET', 'POST'])
def sendpersons():
    print(request.form)
    print(request.form.get('olga', 0))
    mod_id = request.form.get('modId', -1)
    return render_template("modstatistics.html", mod_id = mod_id, json_path = JSON_INDEX_PATH)
