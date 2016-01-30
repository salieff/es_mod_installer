from app import db

class Person(db.Model):
    __tablename__ = 'person'
    id = db.Column(db.Integer, primary_key = True)
    alias = db.Column(db.String)
    name = db.Column(db.String)
    primary_role = db.Column(db.Boolean)
    modpersons = db.relationship("ModPerson", backref="person")

class ModVote(db.Model):
    __tablename__ = 'mod_vote'
    id = db.Column(db.Integer, primary_key = True)
    modid = db.Column(db.Integer)
    votes = db.Column(db.Integer)
    modpersons = db.relationship("ModPerson", backref="modvote")
    modotherpersons = db.relationship("ModOtherPerson", backref="modvote")

class ModPerson(db.Model):
    __tablename__ = 'mod_person'
    id = db.Column(db.Integer, primary_key = True)
    modid = db.Column(db.Integer, db.ForeignKey("mod_vote.modid"))
    personid = db.Column(db.Integer, db.ForeignKey("person.id"))
    votes = db.Column(db.Integer)

class ModOtherPerson(db.Model):
    __tablename__ = 'mod_other_person'
    id = db.Column(db.Integer, primary_key = True)
    modid = db.Column(db.Integer, db.ForeignKey("mod_vote.modid"))
    name = db.Column(db.String)
    votes = db.Column(db.Integer)
