from flask import render_template, redirect, url_for, request
from app import app

@app.route('/')
@app.route('/index')
def main_index():
    return render_template("index.html")

@app.route('/sendpersons', methods = ['POST'])
def sendpersons():
    print(request.form)
    print(request.form.get('olga', 0))
    return redirect(url_for('main_index'))
