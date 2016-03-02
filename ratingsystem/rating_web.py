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
_FIELD_ID = r"id"
_FIELD_MAC = r"mac"
_FIELD_MARK = r"mark"
_FIELD_RESULT = r"result"
_FIELD_UP = r"up"
_FIELD_DOWN = r"down"
_FIELD_STATE = r"state"

_VALUE_OPERATION_MARK = r"mark"
_VALUE_OPERATION_QUERY = r"query"
_VALUE_OPERATION_MYMARK = r"mymark"
_VALUE_OPERATION_STAT = r"statistics"

_VALUE_RESULT_OK = r"ok"

_VALUE_MARK_UP = 1
_VALUE_MARK_DOWN = 0

_VALUE_STAT_INSTALLED = r"installed"
_VALUE_STAT_DELETED = r"deleted"

_PREFIX_TITLE = r"t_"


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
            _VALUE_OPERATION_MYMARK,
            _VALUE_OPERATION_STAT]:
        raise ValueError(
            "Invalid value for '{0}' field".format(_FIELD_OPERATION))

    _ensure_field_present(form, _FIELD_ID)

    if operation == _VALUE_OPERATION_MARK:
        _ensure_field_present(form, _FIELD_MAC)
        _ensure_field_present(form, _FIELD_MARK)

    if operation == _VALUE_OPERATION_MYMARK:
        _ensure_field_present(form, _FIELD_MAC)

    if operation == _VALUE_OPERATION_STAT:
        _ensure_field_present(form, _FIELD_MAC)
        _ensure_field_present(form, _FIELD_STATE)

def _validate_id(string_id):
    """ Check whether project JSON contains
    given id (download/parse/check)
    """
    if not project_parser.ProjectParser(
            project_fetcher.fetch()).has_id(int(string_id)):
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

def _validate_state(mark):
    """ Check state. Only installed/deleted available for now """
    if mark not in [_VALUE_STAT_INSTALLED, _VALUE_STAT_DELETED]:
        raise ValueError("Invalid state. Expected: {0}".format(
            "|".join(str(x) for x in[_VALUE_STAT_INSTALLED, _VALUE_STAT_DELETED])))


def _add_mark(form):
    """ Validate all needed fields and
    add mark via database module
    """
    text_id = _fetch_field(form, _FIELD_ID)
    _validate_id(text_id)

    mac = _fetch_field(form, _FIELD_MAC)
    _validate_mac(mac)

    mark = int(_fetch_field(form, _FIELD_MARK))
    _validate_mark(mark)

    with database.Database() as db:
        db.add_mark(text_id, mac, mark)

def _add_statistics(form):
    """ Validate all needed fields and
    add installation statistics via database module
    """
    text_id = _fetch_field(form, _FIELD_ID)
    _validate_id(text_id)

    mac = _fetch_field(form, _FIELD_MAC)
    _validate_mac(mac)

    state = _fetch_field(form, _FIELD_STATE)
    _validate_state(state)

    with database.Database() as db:
        db.add_statistics(text_id, mac, state)


def _query(form, result):
    """ Validate all needed fields and
    query title information via database module
    """
    text_id = _fetch_field(form, _FIELD_ID)

    result[_FIELD_UP] = 0
    result[_FIELD_DOWN] = 0

    with database.Database() as db:
        try:
            summary = db.get_summary(text_id)
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
    text_id = _fetch_field(form, _FIELD_ID)

    mac = _fetch_field(form, _FIELD_MAC)
    _validate_mac(mac)

    result[_FIELD_MARK] = -1

    with database.Database() as db:
        try:
            result[_FIELD_MARK] = db.find_mark(
                text_id, mac)
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
    if operation == _VALUE_OPERATION_STAT:
        _add_statistics(form)

except Exception as e:
    result[_FIELD_RESULT] = str(e)
finally:
    print(json.dumps(result))
