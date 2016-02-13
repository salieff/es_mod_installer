# -*- coding: utf-8 -*-

""" Raiting system tests.

IMPORTANT: configure (edit config.py) before
run this tests.

IMPORTANT: run tests on empty database only.
"""

import unittest
import config
import project_fetcher
import project_parser
import database

try:
    import urllib.request as urllib2
except ImportError:
    import urllib2


_test_json = """
{
    "packs":
    [
        {
            "idmod": 2
        },
        {
            "idmod": 3
        }
    ]
}
"""


class TestProjectFetcher(unittest.TestCase):

    def test_url_fetched(self):
        request = urllib2.Request(
            config.project_json_url,
            headers={'User-Agent': config.user_agent})
        fetched = urllib2.urlopen(
            request).read().decode(config.project_json_encoding)
        self.assertEqual(fetched, project_fetcher.fetch())


class TestProjectParser(unittest.TestCase):

    def test_title_found(self):
        self.assertTrue(project_parser.ProjectParser(
            _test_json).has_id(2))

    def test_title_not_found(self):
        self.assertFalse(project_parser.ProjectParser(
            _test_json).has_id(4))

    def test_found_in_real_project(self):
        self.assertTrue(project_parser.ProjectParser(
            project_fetcher.fetch()).has_id(66))

    def test_not_found_in_real_project(self):
        self.assertFalse(project_parser.ProjectParser(
            project_fetcher.fetch()).has_id(66666666))


class TestDatabase(unittest.TestCase):

    def test_login(self):
        with database.Database():
            pass

    def test_table_doesnt_exist(self):
        with database.Database() as db:
            self.assertFalse(db._if_table_exists("testtable"))

    def test_create_check_delete_check_table(self):
        with database.Database() as db:
            self.assertFalse(db._if_table_exists("testtable"))
            db._create_title_table("testtable")
            self.assertTrue(db._if_table_exists("testtable"))
            db._delete_table("testtable")
            self.assertFalse(db._if_table_exists("testtable"))

    def test_add_and_check_mark(self):
        with database.Database() as db:
            db._create_title_table("testtable")
            db._add_mark("testtable", "00:16:ea:58:09:26", 1)
            self.assertTrue(db._if_mark_exists(
                "testtable", "00:16:ea:58:09:26"))
            self.assertFalse(db._if_mark_exists(
                "testtable", "00:16:ea:58:09:27"))
            db._delete_table("testtable")

    def test_check_mark_values(self):
        with database.Database() as db:
            db._create_title_table("testtable")
            db._add_mark("testtable", "00:16:ea:58:09:26", 1)
            mac, mark = db._find_mark("testtable", "00:16:ea:58:09:26")
            self.assertEqual(mac, "00:16:ea:58:09:26")
            self.assertEqual(mark, 1)
            db._delete_table("testtable")

    def test_check_mark_reset(self):
        with database.Database() as db:
            db._create_title_table("testtable")
            db._add_mark("testtable", "00:16:ea:58:09:26", 1)
            db._set_mark("testtable", "00:16:ea:58:09:26", 0)
            mac, mark = db._find_mark("testtable", "00:16:ea:58:09:26")
            self.assertEqual(mark, 0)
            db._delete_table("testtable")

    def test_add_summary_table(self):
        with database.Database() as db:
            db._create_summary_table()
            self.assertTrue(db._if_table_exists("summary"))
            db._delete_table("summary")
            self.assertFalse(db._if_table_exists("summary"))

    def test_add_summary(self):
        with database.Database() as db:
            db._create_summary_table()
            db._add_summary('testtitle', 12, 13)
            self.assertTrue(db._if_summary_exists('testtitle'))
            self.assertFalse(db._if_summary_exists('anothertesttitle'))
            db._delete_table("summary")

    def test_check_summary_values(self):
        with database.Database() as db:
            db._create_summary_table()
            db._add_summary('testtitle', 12, 13)
            title, up, down = db._find_summary('testtitle')
            self.assertEqual(title, 'testtitle')
            self.assertEqual(up, 12)
            self.assertEqual(down, 13)
            db._delete_table("summary")

    def test_add_summary_upvote(self):
        with database.Database() as db:
            db._create_summary_table()
            db._add_summary('testtitle', 12, 14)
            db._add_summary_upvote('testtitle')
            title, up, down = db._find_summary('testtitle')
            self.assertEqual(up, 13)
            self.assertEqual(down, 14)
            db._delete_table("summary")

    def test_add_summary_downvote(self):
        with database.Database() as db:
            db._create_summary_table()
            db._add_summary('testtitle', 12, 14)
            db._add_summary_downvote('testtitle')
            title, up, down = db._find_summary('testtitle')
            self.assertEqual(up, 12)
            self.assertEqual(down, 15)
            db._delete_table("summary")

    def test_change_summary_to_upvote(self):
        with database.Database() as db:
            db._create_summary_table()
            db._add_summary('testtitle', 12, 14)
            db._change_summary_to_upvote('testtitle')
            title, up, down = db._find_summary('testtitle')
            self.assertEqual(up, 13)
            self.assertEqual(down, 13)
            db._delete_table("summary")

    def test_change_summary_to_downvote(self):
        with database.Database() as db:
            db._create_summary_table()
            db._add_summary('testtitle', 12, 14)
            db._change_summary_to_downvote('testtitle')
            title, up, down = db._find_summary('testtitle')
            self.assertEqual(up, 11)
            self.assertEqual(down, 15)
            db._delete_table("summary")

    def test_public_add_mark_to_different_titles(self):
        with database.Database() as db:
            db.add_mark("testtitle", "00:16:ea:58:09:26", 1)
            db.add_mark("testtitle2", "00:16:ea:58:09:26", 0)

            self.assertTrue(db._if_table_exists("testtitle"))
            self.assertTrue(db._if_table_exists("testtitle2"))
            self.assertTrue(db._if_table_exists("summary"))

            mac, mark = db._find_mark("testtitle", "00:16:ea:58:09:26")
            self.assertEqual(mac, "00:16:ea:58:09:26")
            self.assertEqual(mark, 1)

            mac, mark = db._find_mark("testtitle2", "00:16:ea:58:09:26")
            self.assertEqual(mac, "00:16:ea:58:09:26")
            self.assertEqual(mark, 0)

            title, up, down = db._find_summary('testtitle')
            self.assertEqual(title, 'testtitle')
            self.assertEqual(up, 1)
            self.assertEqual(down, 0)

            title, up, down = db._find_summary('testtitle2')
            self.assertEqual(title, 'testtitle2')
            self.assertEqual(up, 0)
            self.assertEqual(down, 1)

            db._delete_table("summary")
            db._delete_table("testtitle")
            db._delete_table("testtitle2")

    def test_public_add_two_marks_to_one_title(self):
        with database.Database() as db:
            db.add_mark("testtitle", "00:16:ea:58:09:26", 1)
            db.add_mark("testtitle", "00:16:ea:58:09:27", 0)

            self.assertTrue(db._if_table_exists("testtitle"))
            self.assertTrue(db._if_table_exists("summary"))

            mac, mark = db._find_mark("testtitle", "00:16:ea:58:09:26")
            self.assertEqual(mac, "00:16:ea:58:09:26")
            self.assertEqual(mark, 1)

            mac, mark = db._find_mark("testtitle", "00:16:ea:58:09:27")
            self.assertEqual(mac, "00:16:ea:58:09:27")
            self.assertEqual(mark, 0)

            title, up, down = db._find_summary('testtitle')
            self.assertEqual(title, 'testtitle')
            self.assertEqual(up, 1)
            self.assertEqual(down, 1)

            db._delete_table("summary")
            db._delete_table("testtitle")

    def test_public_reset_mark_to_downvote(self):
        with database.Database() as db:
            db.add_mark("testtitle", "00:16:ea:58:09:26", 1)
            db.add_mark("testtitle", "00:16:ea:58:09:26", 0)

            mac, mark = db._find_mark("testtitle", "00:16:ea:58:09:26")
            self.assertEqual(mac, "00:16:ea:58:09:26")
            self.assertEqual(mark, 0)

            title, up, down = db._find_summary('testtitle')
            self.assertEqual(title, 'testtitle')
            self.assertEqual(up, 0)
            self.assertEqual(down, 1)

            db._delete_table("summary")
            db._delete_table("testtitle")

    def test_public_reset_mark_to_upvote(self):
        with database.Database() as db:
            db.add_mark("testtitle", "00:16:ea:58:09:26", 0)
            db.add_mark("testtitle", "00:16:ea:58:09:26", 1)

            mac, mark = db._find_mark("testtitle", "00:16:ea:58:09:26")
            self.assertEqual(mac, "00:16:ea:58:09:26")
            self.assertEqual(mark, 1)

            title, up, down = db._find_summary('testtitle')
            self.assertEqual(title, 'testtitle')
            self.assertEqual(up, 1)
            self.assertEqual(down, 0)

            db._delete_table("summary")
            db._delete_table("testtitle")

    def test_public_similar_marks(self):
        with database.Database() as db:
            db.add_mark("testtitle", "00:16:ea:58:09:26", 1)
            db.add_mark("testtitle", "00:16:ea:58:09:26", 1)

            mac, mark = db._find_mark("testtitle", "00:16:ea:58:09:26")
            self.assertEqual(mac, "00:16:ea:58:09:26")
            self.assertEqual(mark, 1)

            title, up, down = db._find_summary('testtitle')
            self.assertEqual(title, 'testtitle')
            self.assertEqual(up, 1)
            self.assertEqual(down, 0)

            db._delete_table("summary")
            db._delete_table("testtitle")

    def test_public_find_mark(self):
        with database.Database() as db:
            db.add_mark("testtitle", "00:16:ea:58:09:26", 1)

            with self.assertRaises(ValueError):
                db.find_mark("testtitle2", "00:16:ea:58:09:26")
            with self.assertRaises(ValueError):
                db.find_mark("testtitle", "00:16:ea:58:09:27")
            self.assertEqual(db.find_mark("testtitle", "00:16:ea:58:09:26"), 1)

            db._delete_table("summary")
            db._delete_table("testtitle")

    def test_public_get_summary(self):
        with database.Database() as db:
            with self.assertRaises(ValueError):
                db.get_summary("testtitle")

            db.add_mark("testtitle", "00:16:ea:58:09:26", 0)
            db.add_mark("testtitle", "00:16:ea:58:09:27", 1)
            db.add_mark("testtitle", "00:16:ea:58:09:28", 1)

            db.add_mark("testtitle2", "00:16:ea:58:09:26", 0)
            db.add_mark("testtitle2", "00:16:ea:58:09:27", 1)
            db.add_mark("testtitle2", "00:16:ea:58:09:28", 0)

            with self.assertRaises(ValueError):
                db.get_summary("testtitle3")
            self.assertEqual(db.get_summary("testtitle"), (2, 1))
            self.assertEqual(db.get_summary("testtitle2"), (1, 2))

            db._delete_table("summary")
            db._delete_table("testtitle")
            db._delete_table("testtitle2")


if __name__ == '__main__':
    unittest.main()
