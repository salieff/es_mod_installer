#!/usr/bin/env python
# -*- coding: utf-8 -*-

from config import SQLALCHEMY_DATABASE_URI
from app import db, models
import os.path

from sqlalchemy.engine import Engine
from sqlalchemy import event

@event.listens_for(Engine, "connect")
def set_sqlite_pragma(dbapi_connection, connection_record):
    cursor = dbapi_connection.cursor()
    cursor.execute("PRAGMA foreign_keys=ON")
    cursor.close()

db.drop_all()
db.create_all()

persons = [{'alias': "semen", 'name': u"Семён", 'primary_role': True},
           {'alias': "alisa", 'name': u"Алиса", 'primary_role': True},
           {'alias': "lena", 'name': u"Лена", 'primary_role': True},
           {'alias': "miku", 'name': u"Мику", 'primary_role': True},
           {'alias': "pioneer", 'name': u"Пионер", 'primary_role': True},
           {'alias': "slavya", 'name': u"Славя", 'primary_role': True},
           {'alias': "ulyana", 'name': u"Ульяна", 'primary_role': True},
           {'alias': "yulya", 'name': u"Юля", 'primary_role': True},
           {'alias': "viola", 'name': u"Виола", 'primary_role': False},
           {'alias': "jenya", 'name': u"Женя", 'primary_role': False},
           {'alias': "tolik", 'name': u"Толик", 'primary_role': False},
           {'alias': "shurik", 'name': u"Шурик", 'primary_role': False},
           {'alias': "elektronik", 'name': u"Электроник", 'primary_role': False},
           {'alias': "olga", 'name': u"Ольга Дмитриевна", 'primary_role': False}]

for pers in persons:
    p = models.Person(alias = pers['alias'], name = pers['name'], primary_role = pers['primary_role'])
    db.session.add(p)

db.session.commit()
