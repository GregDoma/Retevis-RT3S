import contextlib
import io
import os
import tempfile
import unittest
from pathlib import Path
from unittest import mock

from rt3s_flasher import cli, core

from . import helpers


class CliTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.dir = Path(self.tmp.name)
        env = {'HOME': str(self.dir), 'XDG_DATA_HOME': str(self.dir / 'data')}
        self.env = mock.patch.dict(os.environ, env)
        self.env.start()
        self.fw = self.dir / 'OpenGD77_MDUV380.bin'
        self.fw.write_bytes(helpers.firmware())
        self.gla = self.dir / 'Polish.gla'
        self.gla.write_bytes(helpers.language())

    def tearDown(self):
        self.env.stop()
        self.tmp.cleanup()

    def run_cli(self, *argv):
        out, err = io.StringIO(), io.StringIO()
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
            code = cli.main(list(argv))
        return code, out.getvalue() + err.getvalue()

    def test_dry_run_fm_only(self):
        code, out = self.run_cli('flash', str(self.fw), '-j', str(self.gla), '--dry-run')
        self.assertEqual(code, 0, out)
        self.assertIn('drugi język: Polish', out)
        self.assertIn('tylko na FM', out)
        self.assertIn('nic nie zostało wysłane', out)

    def test_builtin_polish_skips_gla(self):
        fw = self.dir / 'OpenGD77_MDUV380_PL.bin'
        fw.write_bytes(helpers.firmware(builtin='Polski'))
        code, out = self.run_cli('flash', str(fw), '-n')
        self.assertEqual(code, 0, out)
        self.assertIn('języki wbudowane: English, Polski', out)
        self.assertNotIn('tylko angielski', out)
        code, out = self.run_cli('flash', str(fw), '-j', str(self.gla), '-n')
        self.assertEqual(code, 0, out)
        self.assertIn('nadpisałby', out)

    def test_save_image(self):
        out_img = self.dir / 'img.bin'
        code, out = self.run_cli('flash', str(self.fw), '-n', '--save-image', str(out_img))
        self.assertEqual(code, 0, out)
        self.assertEqual(out_img.read_bytes(), bytes(core.build_image(self.fw.read_bytes()).data))

    def test_wrong_extension(self):
        other = self.dir / 'fw.hex'
        other.write_bytes(b'x')
        code, out = self.run_cli('flash', str(other), '-n')
        self.assertEqual(code, 1)
        self.assertIn('.bin', out)

    def test_bad_donor_rejected(self):
        bad = self.dir / 'donor.bin'
        bad.write_bytes(helpers.donor())
        code, out = self.run_cli('donor', str(bad))
        self.assertEqual(code, 1)
        self.assertIn('SHA-256', out)
        code, out = self.run_cli('flash', str(self.fw), '-d', str(bad), '-n')
        self.assertEqual(code, 1)

    def test_no_donor(self):
        code, out = self.run_cli('donor')
        self.assertEqual(code, 1)
        self.assertIn('Brak pliku dawcy', out)

    def test_needs_confirmation_without_terminal(self):
        with mock.patch('sys.stdin', io.StringIO('')):
            code, out = self.run_cli('flash', str(self.fw))
        self.assertEqual(code, 2)
        self.assertIn('--yes', out)


if __name__ == '__main__':
    unittest.main()
