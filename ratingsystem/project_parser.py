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
        self.titles = []
        data = json.loads(project_json)
        for pack in data['packs']:
            self.titles.append(pack['title'].strip())

    def has_title(self, title):
        """ Find given title in parsed list """
        return title in self.titles
