from app import db

association_table = db.Table('association',
    db.Column('left_id', db.Integer, db.ForeignKey('user.id')),
    db.Column('right_id', db.Integer, db.ForeignKey('looser.id'))
)

class User(db.Model):
    id = db.Column(db.Integer, primary_key = True)
    nickname = db.Column(db.String)
    loosers = db.relationship("Looser", secondary=association_table, back_populates="users")

class Looser(db.Model):
    id = db.Column(db.Integer, primary_key = True)
    description = db.Column(db.String)
    users = db.relationship("User", secondary=association_table, back_populates="loosers")
