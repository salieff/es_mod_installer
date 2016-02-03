# -*- coding: utf-8 -*-

""" Contains function for fetching json
from project url
"""

import config
# python 2/3 compatibility
try:
    import urllib.request as urllib2
except ImportError:
    import urllib2


def fetch():
    request = urllib2.Request(
        config.project_json_url,
        headers={'User-Agent': config.user_agent})
    return urllib2.urlopen(request).read().decode(config.project_json_encoding)
