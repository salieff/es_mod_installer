# -*- coding: utf-8 -*-

import config
import MySQLdb

""" Classes and functions for mysql/mariadb handling """

_sql_query_if_table_exists = """
SELECT COUNT(*)
FROM information_schema.tables
WHERE table_name = '{0}';
"""

_sql_query_create_title_table = """
CREATE TABLE `{0}`.`{1}` ( `Mac` CHAR(17) NOT NULL ,
`Mark` BOOLEAN NOT NULL ) ENGINE = InnoDB;
""".format(config.database_name, "{0}")


_sql_query_create_summary_table = """
CREATE TABLE `{0}`.`summary` ( `Title` CHAR(34) NOT NULL ,
`Up` INT NOT NULL , `Down` INT NOT NULL ) ENGINE = InnoDB;
""".format(config.database_name)


_sql_query_delete_table = """
DROP TABLE `{0}`.`{1}`;
""".format(config.database_name, "{0}")


_sql_query_add_mark = """
INSERT INTO `{0}` (`Mac`, `Mark`)
VALUES ('{1}', '{2}')
"""

_sql_query_set_mark = """
UPDATE `{0}` SET `Mark` = '{2}'
WHERE `Mac` = '{1}'
"""

_sql_query_find_mark_by_mac = """
SELECT * FROM `{0}`
WHERE `Mac` = '{1}'
"""

_sql_query_add_summary = """
INSERT INTO `summary` (`Title`, `Up`, `Down`)
VALUES ('{0}', '{1}', '{2}')
"""

_sql_query_add_summary_upvote = """
UPDATE `summary` SET `Up` = `Up` + 1
WHERE `Title` = '{0}'
"""

_sql_query_add_summary_downvote = """
UPDATE `summary` SET `Down` = `Down` + 1
WHERE `Title` = '{0}'
"""

_sql_query_summary_change_to_upvote = """
UPDATE `summary` SET `Up` = `Up` + 1, `Down` = `Down` - 1
WHERE `Title` = '{0}'
"""

_sql_query_summary_change_to_downvote = """
UPDATE `summary` SET `Up` = `Up` - 1, `Down` = `Down` + 1
WHERE `Title` = '{0}'
"""

_sql_query_find_summary_by_title = """
SELECT * FROM `summary`
WHERE `Title` = '{0}'
"""

_UP_VOTE = 1
_DWON_VOTE = 0


class Database:
    """ Class for handling
    database operations
    """

    def __init__(self):
        """ Connect to the database on creation """
        self.db = MySQLdb.connect(
            host=config.database_host,
            db=config.database_name,
            user=config.database_user,
            passwd=config.database_password)

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.db.commit()
        self.db.close()

    def _execute(self, query):
        """ Execute sql query by internal means """
        with self.db.cursor() as cursor:
            cursor.execute(query)
            return cursor.fetchall()

    def _if_table_exists(self, name):
        """ Check if the table with given name exists """
        query = _sql_query_if_table_exists.format(name)
        return self._execute(query)[0][0] > 0

    def _create_title_table(self, name):
        """ Create  table for storing marks with given name
        Fields: mac, mark
        """
        query = _sql_query_create_title_table.format(name)
        self._execute(query)

    def _create_summary_table(self):
        """ Create  table for storing marks summary
        for completely all titles
        Fields: title, up, down
        """
        query = _sql_query_create_summary_table
        self._execute(query)

    def _delete_table(self, name):
        """ Drop table """
        query = _sql_query_delete_table.format(name)
        self._execute(query)

    def _add_mark(self, name, mac, mark):
        """ Add mark to existing table (title) """
        query = _sql_query_add_mark.format(name, mac, mark)
        self._execute(query)

    def _set_mark(self, name, mac, mark):
        """ Change existing mark """
        query = _sql_query_set_mark.format(name, mac, mark)
        self._execute(query)

    def _if_mark_exists(self, title, mac):
        """ Check if mark for this title
        and mac exists
        """
        query = _sql_query_find_mark_by_mac.format(title, mac)
        return len(self._execute(query)) > 0

    def _find_mark(self, title, mac):
        """ Query mark from existing table """
        query = _sql_query_find_mark_by_mac.format(title, mac)
        return self._execute(query)[0]

    def _find_mark_raw(self, title, mac):
        """ Query mark from existing table.
        Returns query result as is. Used for better
        speed not to call _if_mark_exists before
        """
        query = _sql_query_find_mark_by_mac.format(title, mac)
        return self._execute(query)

    def _add_summary(self, title, up, down):
        """ Add summary table row """
        query = _sql_query_add_summary.format(title, up, down)
        self._execute(query)

    def _add_summary_upvote(self, title):
        """ Increment upvotes count for given title """
        query = _sql_query_add_summary_upvote.format(title)
        self._execute(query)

    def _add_summary_downvote(self, title):
        """ Increment downvotes count for given title """
        query = _sql_query_add_summary_downvote.format(title)
        self._execute(query)

    def _change_summary_to_upvote(self, title):
        """ Increment upvotes count for given title,
        decrement downvotes count. Like user changes his mind
        """
        query = _sql_query_summary_change_to_upvote.format(title)
        self._execute(query)

    def _change_summary_to_downvote(self, title):
        """ Increment downvotes count for given title,
        decrement upvotes count. Like user changes his mind
        """
        query = _sql_query_summary_change_to_downvote.format(title)
        self._execute(query)

    def _if_summary_exists(self, title):
        """ Check if summary row exists for given title """
        query = _sql_query_find_summary_by_title.format(title)
        return len(self._execute(query)) > 0

    def _find_summary(self, title):
        """ Query summary information for given title """
        query = _sql_query_find_summary_by_title.format(title)
        return self._execute(query)[0]

    def _find_summary_raw(self, title):
        """ Query summary information for given title.
        Returns query result as is. Used for better
        speed not to call _if_summary_exists before
        """
        query = _sql_query_find_summary_by_title.format(title)
        return self._execute(query)

    def _get_summary_increment_method(self, mark):
        if mark == _UP_VOTE:
            return self._add_summary_upvote
        return self._add_summary_downvote

    def _get_summary_update_method(self, mark):
        if mark == _UP_VOTE:
            return self._change_summary_to_upvote
        return self._change_summary_to_downvote

    def add_mark(self, title, mac, mark):
        """ Full cycle public add mark method.
        Creates all tables if don't exist,
        add mark, add/change summary information,
        changes mark if already exists
        """
        if not self._if_table_exists("summary"):
            self._create_summary_table()

        if not self._if_summary_exists(title):
            self._add_summary(title, 0, 0)

        if not self._if_table_exists(title):
            self._create_title_table(title)

        if not self._if_mark_exists(title, mac):
            self._add_mark(title, mac, mark)
            self._get_summary_increment_method(mark)(title)
            return

        _, old_mark = self._find_mark(title, mac)
        if mark == old_mark:
            return

        self._set_mark(title, mac, mark)
        self._get_summary_update_method(mark)(title)

    def find_mark(self, title, mac):
        """ Find mark by title and mac
        address. Raises if not found.
        Return 0/1
        """
        if not self._if_table_exists(title):
            raise ValueError("Not found")

        query_result = self._find_mark_raw(title, mac)
        if len(query_result) == 0:
            raise ValueError("Not found")

        return query_result[0][1]

    def get_summary(self, title):
        """ Find summary information for
        given title. Raises if not found
        """
        if not self._if_table_exists("summary"):
            raise ValueError("Not found")

        query_result = self._find_summary_raw(title)
        if len(query_result) == 0:
            raise ValueError("Not found")

        return (query_result[0][1], query_result[0][2])
