# -*- coding: utf-8 -*-

""" Contains class for parsing json data with all
plugins set to avoid bad titles occurances in database
"""

import json


class ProjectParser:
    """ Class for parsing json string and
    handling parse data
    """

    def __init__(self, project_json):
        """ Construct class, parse and store input
        json string.
        """
        self.ids = []
        data = json.loads(project_json)
        for pack in data['packs']:
            self.ids.append(pack['idmod'])

    def has_id(self, id):
        """ Find given title in parsed list """
        return id in self.ids
