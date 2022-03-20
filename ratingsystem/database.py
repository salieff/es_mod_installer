# -*- coding: utf-8 -*-

import config
import MySQLdb

""" Classes and functions for mysql/mariadb handling """

_sql_query_if_table_exists = """
SELECT COUNT(*)
FROM information_schema.tables
WHERE table_name = '{0}' AND table_schema = '{1}';
"""

_sql_query_create_title_table = """
CREATE TABLE `{0}`.`{1}` ( `Udid` VARCHAR(255) NOT NULL ,
`Mark` BOOLEAN NOT NULL ) ENGINE = InnoDB;
""".format(config.database_name, "{0}")


_sql_query_create_summary_table = """
CREATE TABLE `{0}`.`summary` ( `Title` VARCHAR(512) NOT NULL ,
`Up` INT NOT NULL , `Down` INT NOT NULL ) ENGINE = InnoDB;
""".format(config.database_name)

_sql_query_create_statistics_table = """
CREATE TABLE `{0}`.`statistics` ( `ModId` INT NOT NULL ,
`Udid` VARCHAR(255) NOT NULL , InstallTime DATETIME, DeleteTime DATETIME ) ENGINE = InnoDB;
""".format(config.database_name)


_sql_query_delete_table = """
DROP TABLE `{0}`.`{1}`;
""".format(config.database_name, "{0}")


_sql_query_add_mark = """
INSERT INTO `{0}` (`Udid`, `Mark`)
VALUES ('{1}', '{2}')
"""

_sql_query_set_mark = """
UPDATE `{0}` SET `Mark` = '{2}'
WHERE `Udid` = '{1}'
"""

_sql_query_find_mark_by_udid = """
SELECT * FROM `{0}`
WHERE `Udid` = '{1}'
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

_sql_query_find_summary = """
SELECT * FROM `summary`
"""

_sql_query_find_summary_by_title = """
SELECT * FROM `summary`
WHERE `Title` = '{0}'
"""

_sql_query_add_statistics = """
INSERT INTO `statistics` (`ModId`, `Udid`, `InstallTime`)
VALUES ('{0}', '{1}', NOW())
"""

_sql_query_find_statistics_by_id_and_udid = """
SELECT * FROM `statistics`
WHERE `ModId` = '{0}' AND `Udid` = '{1}' AND `DeleteTime` IS NULL
"""
_sql_query_update_statistics = """
UPDATE `statistics` SET `DeleteTime` = NOW()
WHERE `ModId` = '{0}' AND `Udid` = '{1}' AND `DeleteTime` IS NULL
ORDER BY `InstallTime` DESC
LIMIT 1
"""

_sql_query_find_statistics_by_period = """
SELECT COUNT(*), SUM(IF(DeleteTime IS NULL, 1, 0)) FROM `statistics`
WHERE `ModId` = '{0}' AND TIMESTAMPDIFF(HOUR, InstallTime, NOW()) <= '{1}'
"""

_sql_query_find_statistics = """
SELECT COUNT(*), SUM(IF(DeleteTime IS NULL, 1, 0)) FROM `statistics`
WHERE `ModId` = '{0}'
"""

_sql_query_find_statistics_lifetime = """
SELECT
AVG(TIMESTAMPDIFF(HOUR, InstallTime, IF(DeleteTime IS NULL, NOW(), DeleteTime))),
MAX(TIMESTAMPDIFF(HOUR, InstallTime, IF(DeleteTime IS NULL, NOW(), DeleteTime)))
FROM `statistics`
WHERE `ModId` = '{0}' AND InstallTime IS NOT NULL AND DeleteTime IS NOT NULL AND InstallTime <> DeleteTime
"""

_sql_query_find_global_statistics = """
SELECT `ModId`,
COUNT(*), SUM(IF(`DeleteTime` IS NULL, 1, 0)),
SUM(IF(TIMESTAMPDIFF(HOUR, `InstallTime`, NOW()) <= 720, 1, 0)), SUM(IF(TIMESTAMPDIFF(HOUR, `InstallTime`, NOW()) <= 720 && `DeleteTime` IS NULL, 1, 0)),
SUM(IF(TIMESTAMPDIFF(HOUR, `InstallTime`, NOW()) <= 168, 1, 0)), SUM(IF(TIMESTAMPDIFF(HOUR, `InstallTime`, NOW()) <= 168 && `DeleteTime` IS NULL, 1, 0)),
IFNULL(AVG(IF(`DeleteTime` IS NOT NULL && `InstallTime` <> `DeleteTime`, TIMESTAMPDIFF(HOUR, `InstallTime`, `DeleteTime`), NULL)),0),
IFNULL(MAX(IF(`DeleteTime` IS NOT NULL && `InstallTime` <> `DeleteTime`, TIMESTAMPDIFF(HOUR, `InstallTime`, `DeleteTime`), NULL)),0)
FROM statistics
GROUP BY ModId
"""

_UP_VOTE = 1
_DOWN_VOTE = 0

_STAT_INSTALLED = r"installed"
_STAT_DELETED = r"deleted"

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
        query = _sql_query_if_table_exists.format(name, config.database_name)
        return self._execute(query)[0][0] > 0

    def _create_title_table(self, name):
        """ Create  table for storing marks with given name
        Fields: udid, mark
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

    def _create_statistics_table(self):
        """ Create  table for storing
        installation statistics
        Fields: modid, udid, installtime, deletetime
        """
        query = _sql_query_create_statistics_table
        self._execute(query)

    def _delete_table(self, name):
        """ Drop table """
        query = _sql_query_delete_table.format(name)
        self._execute(query)

    def _add_mark(self, name, udid, mark):
        """ Add mark to existing table (title) """
        query = _sql_query_add_mark.format(name, udid, mark)
        self._execute(query)

    def _set_mark(self, name, udid, mark):
        """ Change existing mark """
        query = _sql_query_set_mark.format(name, udid, mark)
        self._execute(query)

    def _if_mark_exists(self, title, udid):
        """ Check if mark for this title
        and udid exists
        """
        query = _sql_query_find_mark_by_udid.format(title, udid)
        return len(self._execute(query)) > 0

    def _find_mark(self, title, udid):
        """ Query mark from existing table """
        query = _sql_query_find_mark_by_udid.format(title, udid)
        return self._execute(query)[0]

    def _find_mark_raw(self, title, udid):
        """ Query mark from existing table.
        Returns query result as is. Used for better
        speed not to call _if_mark_exists before
        """
        query = _sql_query_find_mark_by_udid.format(title, udid)
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

    def _add_statistics(self, text_id, udid):
        """ Add statistics table row """
        query = _sql_query_add_statistics.format(text_id, udid)
        self._execute(query)

    def _if_statistics_exists(self, text_id, udid):
        """ Check if uncompleted statistics row exists for given id and udid """
        query = _sql_query_find_statistics_by_id_and_udid.format(text_id, udid)
        return len(self._execute(query)) > 0

    def _update_statistics(self, text_id, udid):
        """ Set deletion time to now for uncompleted statistics row
        with greatest installation time, given id and udid
        """
        query = _sql_query_update_statistics.format(text_id, udid)
        self._execute(query)

    def _find_statictics(self, title, period):
        """ Returns installations count, all and active,
        for specified period
        """
        if period is None:
            query = _sql_query_find_statistics.format(title)
        else:
            query = _sql_query_find_statistics_by_period.format(title, period)
        return self._execute(query)

    def _find_lifetime(self, title):
        """ Returns lifetime for mod, average and maximum
        """
        query = _sql_query_find_statistics_lifetime.format(title)
        return self._execute(query)

    def add_mark(self, title, udid, mark):
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

        if not self._if_mark_exists(title, udid):
            self._add_mark(title, udid, mark)
            self._get_summary_increment_method(mark)(title)
            return

        _, old_mark = self._find_mark(title, udid)
        if mark == old_mark:
            return

        self._set_mark(title, udid, mark)
        self._get_summary_update_method(mark)(title)

    def find_mark(self, title, udid):
        """ Find mark by title and udid
        address. Raises if not found.
        Return 0/1
        """
        if not self._if_table_exists(title):
            raise ValueError("Not found")

        query_result = self._find_mark_raw(title, udid)
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

    def get_global_summary(self, udid):
        """ Find summary information for all titles """
        if not self._if_table_exists("summary"):
            raise ValueError("Not found")

        stat_result = self._execute(_sql_query_find_summary)
        if len(stat_result) == 0:
            raise ValueError("Not found")

        results = []
        for record in stat_result:
            titleid = record[0]
            res = list(record)
            if udid:
                mark_result = self._find_mark_raw(titleid, udid)
                if len(mark_result) == 0:
                    res.append(-1)
                else:
                    res.append(mark_result[0][1])
            else:
                res.append(-1)
            results.append(res)

        return results

    def add_statistics(self, text_id, udid, state):
        """ Public add statistics method.
        Creates table if doesn't exist,
        and add statistics
        """
        if not self._if_table_exists("statistics"):
            self._create_statistics_table()

        if state == _STAT_INSTALLED:
            self._add_statistics(text_id, udid)
            return

        if not self._if_statistics_exists(text_id, udid):
            self._add_statistics(text_id, udid)

        self._update_statistics(text_id, udid)

    def get_statistics(self, title, period = None):
        """ Returns installations count, all and active,
        for specified period
        """
        if not self._if_table_exists("statistics"):
            raise ValueError("Not found")

        query_result = self._find_statictics(title, period)
        if len(query_result) == 0:
            raise ValueError("Not found")

        return (query_result[0][0], query_result[0][1])

    def get_lifetime(self, title):
        """ Returns lifetime for title, average and maximum
        """
        if not self._if_table_exists("statistics"):
            raise ValueError("Not found")

        query_result = self._find_lifetime(title)
        if len(query_result) == 0:
            raise ValueError("Not found")

        return (query_result[0][0], query_result[0][1])

    def get_global_statistics(self):
        """ Returns installations count, all and active,
        for all period, lasth month and last week, followed by lifetime average and maximum,
        for all mod-titles.
        """
        if not self._if_table_exists("statistics"):
            raise ValueError("Not found")

        query_result = self._execute(_sql_query_find_global_statistics)
        if len(query_result) == 0:
            raise ValueError("Not found")

        return query_result
