import os
basedir = os.path.abspath(os.path.dirname(__file__))

SQLALCHEMY_DATABASE_URI = 'sqlite:///' + os.path.join(basedir, 'modpersons.db')
SQLALCHEMY_TRACK_MODIFICATIONS = True
JSON_INDEX_PATH = "http://191.ru/es/project1.json"
