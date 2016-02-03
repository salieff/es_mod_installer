#!/usr/bin/env python
# -*- coding: utf-8 -*-

""" CGI gate for rating system.
Only this file should be available for web.
"""

import config
import project_fetcher
import project_parser
import database
import cgi
import cgitb
import json
import sys
import hashlib

_FIELD_OPERATION = r"operation"
_FIELD_TITLE = r"title"
_FIELD_MAC = r"mac"
_FIELD_MARK = r"mark"
_FIELD_RESULT = r"result"
_FIELD_UP = r"up"
_FIELD_DOWN = r"down"

_VALUE_OPERATION_MARK = r"mark"
_VALUE_OPERATION_QUERY = r"query"
_VALUE_OPERATION_MYMARK = r"mymark"

_VALUE_RESULT_OK = r"ok"

_VALUE_MARK_UP = 1
_VALUE_MARK_DOWN = 0

_PREFIX_TITLE = r"t_"


def _hash_title(title):
    """ Convert title to t_<md5sum> form """
    string_hash = hashlib.md5(
        title.encode('utf-8')).hexdigest()
    return _PREFIX_TITLE + string_hash


def _fetch_field(form, name):
    """ Find and decode (if needed field in form) """
    fetched = form.getvalue(name)
    if sys.version_info >= (3, 0):
        return fetched
    return fetched.decode(
        config.incoming_requests_encoding)


def _ensure_field_present(form, name):
    """ Raise if field isn't sent with form """
    if name not in form:
        raise ValueError(
            "Field '{0}' is missing".format(name))


def _validate_fields(form):
    """ Validate required field for each operation type.
    Fields:
    operation: always needed
    title: always needed
    mac: required for mark/mymark operations
    mark: required only for mark operation
    """
    _ensure_field_present(form, _FIELD_OPERATION)

    operation = _fetch_field(form, _FIELD_OPERATION)
    if operation not in [
            _VALUE_OPERATION_MARK,
            _VALUE_OPERATION_QUERY,
            _VALUE_OPERATION_MYMARK]:
        raise ValueError(
            "Invalid value for '{0}' field".format(_FIELD_OPERATION))

    _ensure_field_present(form, _FIELD_TITLE)

    if operation == _VALUE_OPERATION_MARK:
        _ensure_field_present(form, _FIELD_MAC)
        _ensure_field_present(form, _FIELD_MARK)

    if operation == _VALUE_OPERATION_MYMARK:
        _ensure_field_present(form, _FIELD_MAC)


def _validate_title(title):
    """ Check whether project JSON contains
    given title (download/parse/check)
    """
    if not project_parser.ProjectParser(
            project_fetcher.fetch()).has_title(title):
        raise ValueError("Given title not found in project JSON")


def _validate_mac(mac):
    """ Check MAC length. TODO: check format """
    if len(mac) != 17:
        raise ValueError("Invalid MAC address. Expected: XX:XX:XX:XX:XX:XX")


def _validate_mark(mark):
    """ Check mark. Only 1/0 available for now """
    if mark not in [_VALUE_MARK_UP, _VALUE_MARK_DOWN]:
        raise ValueError("Invalid mark. Expected: {0}".format(
            "|".join(str(x) for x in[_VALUE_MARK_UP, _VALUE_MARK_DOWN])))


def _add_mark(form):
    """ Validate all needed fields and
    add mark via database module
    """
    text_title = _fetch_field(form, _FIELD_TITLE)
    _validate_title(text_title)

    title = _hash_title(
        _fetch_field(form, _FIELD_TITLE))

    mac = _fetch_field(form, _FIELD_MAC)
    _validate_mac(mac)

    mark = int(_fetch_field(form, _FIELD_MARK))
    _validate_mark(mark)

    with database.Database() as db:
        db.add_mark(title, mac, mark)


def _query(form, result):
    """ Validate all needed fields and
    query title information via database module
    """
    title = _hash_title(
        _fetch_field(form, _FIELD_TITLE))

    result[_FIELD_UP] = 0
    result[_FIELD_DOWN] = 0

    with database.Database() as db:
        try:
            summary = db.get_summary(title)
            result[_FIELD_UP] = summary[0]
            result[_FIELD_DOWN] = summary[1]
        except ValueError:
            pass
    return result


def _my_mark(form, result):
    """ Validate all needed fields and
    query mark for given title and mac
    via database module
    """
    title = _hash_title(
        _fetch_field(form, _FIELD_TITLE))

    mac = _fetch_field(form, _FIELD_MAC)
    _validate_mac(mac)

    result[_FIELD_MARK] = -1

    with database.Database() as db:
        try:
            result[_FIELD_MARK] = db.find_mark(
                title, mac)
        except ValueError:
            pass
    return result

cgitb.enable()
result = {_FIELD_RESULT: _VALUE_RESULT_OK}

print("Content-type: text/plain\n\n")
try:
    form = cgi.FieldStorage()
    _validate_fields(form)

    operation = _fetch_field(form, _FIELD_OPERATION)

    if operation == _VALUE_OPERATION_MARK:
        _add_mark(form)
    if operation == _VALUE_OPERATION_QUERY:
        _query(form, result)
    if operation == _VALUE_OPERATION_MYMARK:
        result = _my_mark(form, result)

except Exception as e:
    result[_FIELD_RESULT] = str(e)
finally:
    print(json.dumps(result))
