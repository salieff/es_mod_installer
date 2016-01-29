#!/usr/bin/python
from config import SQLALCHEMY_DATABASE_URI
from app import db
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
