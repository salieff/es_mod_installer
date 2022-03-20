# -*- coding: utf-8 -*-

""" Raiting system config file """


project_json_url = r'http://191.ru/es/project2.json'
""" Url containing json with
all the plugins titles
"""

project_json_encoding = r'utf-8'
""" Encoding used to decode
json as string
"""

user_agent = r'Raiting system'
""" User agent used for fetching project json data
from url above
"""

incoming_requests_encoding = r'utf-8'
""" Encoding used to decode
incoming data: titles and so on
"""

database_host = r'localhost'
""" Host where database
is located
"""

database_name = r'rating_system'
""" Databse name used for connection
to the database. Must have all the
priveledges to edit specified db
"""

database_user = r'rating_system'
""" User name for connection
to the database
"""

database_password = r'rating_system'
""" Password for connection
to the database
"""
