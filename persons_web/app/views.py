# -*- coding: utf-8 -*-

from flask import render_template, redirect, url_for, request
from app import app, db, models
from config import JSON_INDEX_PATH
import requests

@app.route('/')
@app.route('/index')
def index():
    selected_modid = request.args.get('selected_modid', '1')
    primary_persons = models.Person.query.filter(models.Person.primary_role == True)
    secondary_persons = models.Person.query.filter(models.Person.primary_role != True)
    json_request = requests.get(JSON_INDEX_PATH)
    return render_template("index.html", primary_persons = primary_persons, secondary_persons = secondary_persons, mod_packs = json_request.json()['packs'], selected_modid = selected_modid)

def beautyOtherPersons(op_str):
    op_arr = op_str.split(',')
    op_arr = [' '.join(s.split()).lower() for s in op_arr]
    op_arr = filter(None, op_arr)
    op_arr = set(op_arr)
    return op_arr

#@app.route('/sendpersons', methods = ['GET', 'POST'])
@app.route('/sendpersons', methods = ['POST'])
def sendpersons():
    has_data = False
    mod_id = request.form['modId']

    for key in request.form.keys():
        if key.startswith('personId_'):
            has_data = True
            pers_id = request.form[key]
            modperson = models.ModPerson.query.filter(models.ModPerson.modid == mod_id).filter(models.ModPerson.personid == pers_id).first()
            if modperson:
                modperson.votes += 1;
            else:
                mp = models.ModPerson(modid = mod_id, personid = pers_id, votes = 1)
                db.session.add(mp)

    beauty_persons_other = beautyOtherPersons(request.form['persons_other'])
    for bpo in beauty_persons_other:
        has_data = True
        modop = models.ModOtherPerson.query.filter(models.ModOtherPerson.modid == mod_id).filter(models.ModOtherPerson.name.ilike(bpo)).first()
        if modop:
            modop.votes += 1;
        else:
            mop = models.ModOtherPerson(modid = mod_id, name = bpo, votes = 1)
            db.session.add(mop)

    if has_data:
        modvotes = models.ModVote.query.filter(models.ModVote.modid == mod_id).first()
        if modvotes:
            modvotes.votes += 1;
        else:
            mv = models.ModVote(modid = mod_id, votes = 1)
            db.session.add(mv)

        db.session.commit()

        json_request = requests.get(JSON_INDEX_PATH)
        mod_packs = json_request.json()['packs']
        mod_name = [mod['title'] for mod in mod_packs if int(mod['idmod']) == int(mod_id)][0]
        #return render_template("modstatistics.html", mod_name = mod_name)
        return redirect(url_for('modsinfo', _anchor = 'modheader_'+mod_id))

    return redirect(url_for('index', selected_modid = mod_id))

class ModWithPersons:
    def __init__(self, modid, name):
        self.modid = modid
        self.name = name
        self.persons = []
        self.votes = 0

        modpers = models.ModPerson.query.filter(models.ModPerson.modid == self.modid).order_by(models.ModPerson.votes.desc())
        for mp in modpers:
            self.persons.append({'alias': mp.person.alias, 'name': mp.person.name, 'trust': mp.votes * 100 / mp.modvote.votes})
            self.votes = mp.modvote.votes

        modotherpers = models.ModOtherPerson.query.filter(models.ModOtherPerson.modid == self.modid).order_by(models.ModOtherPerson.votes.desc())
        for mop in modotherpers:
            self.persons.append({'alias': "unknown", 'name': mop.name.title(), 'trust': mop.votes * 100 / mop.modvote.votes})
            self.votes = mop.modvote.votes

@app.route('/modsinfo')
def modsinfo():
    json_request = requests.get(JSON_INDEX_PATH)
    mods_info = []

    for mod in json_request.json()['packs']:
        mwp = ModWithPersons(modid = mod['idmod'], name = mod['title'])
        if mwp.votes > 0:
            mods_info.append(mwp)

    return render_template("modsinfo.html", mods_info = mods_info)
