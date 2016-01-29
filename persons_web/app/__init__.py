import os
from flask import Flask
from flask.ext.sqlalchemy import SQLAlchemy
from config import basedir

app = Flask(__name__)
app.config.from_object('config')
db = SQLAlchemy(app)

from app import views, models

#import requests
#r = requests.get('http://191.ru/es/project1.json')
#print(r.json())
