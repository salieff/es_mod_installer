from app import db

class Person(db.Model):
    id = db.Column(db.Integer, primary_key = True)
    alias = db.Column(db.String)
    name = db.Column(db.String)
    primary_role = db.Column(db.Boolean)

class ModVote(db.Model):
    id = db.Column(db.Integer, primary_key = True)
    modid = db.Column(db.Integer)
    votes = db.Column(db.Integer)

class ModPerson(db.Model):
    id = db.Column(db.Integer, primary_key = True)
    modid = db.Column(db.Integer)
    personid = db.Column(db.Integer, db.ForeignKey("person.id"))
    votes = db.Column(db.Integer)

class ModOtherPerson(db.Model):
    id = db.Column(db.Integer, primary_key = True)
    modid = db.Column(db.Integer)
    name = db.Column(db.String)
    votes = db.Column(db.Integer)
