#!/home/salieff/es_person_checker/bin/python

#VENV_DIR = '/home/salieff/es_person_checker'
#activate_this = os.path.join(VENV_DIR, 'bin', 'activate_this.py')
#execfile(activate_this, dict(__file__=activate_this))

#from flipflop import WSGIServer
from flup.server.fcgi import WSGIServer
from app import app

if __name__ == '__main__':
    WSGIServer(app).run()
