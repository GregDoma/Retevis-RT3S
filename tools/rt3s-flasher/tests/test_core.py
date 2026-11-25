import os
import tempfile
import unittest
from pathlib import Path

from rt3s_flasher import ciphers, core

from . import helpers

ROOT = helpers.ROOT


class CiphersTest(unittest.TestCase):
    def test_same_as_opengd77_loader(self):
        import importlib.util
        spec = importlib.util.spec_from_file_location('gen', Path(__file__).parents[1] / 'scripts' / 'gen_ciphers.py')
        gen = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(gen)
        t = gen.loader_tables()
        self.assertEqual(ciphers.MD9600_CIPHER, bytes(t['MD9600_ENCODE_CIPHER']))
        self.assertEqual(ciphers.MDUV380_CIPHER, bytes(t['MDUV380_ENCODE_CIPHER']))
        self.assertEqual(ciphers.MD380_CIPHER, bytes(t['MD380_ENCODE_CIPHER']))
        self.assertEqual(ciphers.DM1701_CIPHER, bytes(t['DM1701_ENCODE_CIPHER']))
        self.assertEqual(ciphers.DONOR_SHA256, t['FW2645_SHA256_Checksum'])

    def test_codec_address_matches_linker_script(self):
        ld = (ROOT / 'MDUV380_firmware' / 'STM32F405VGTX_FLASH.ld').read_text()
        import re
        m = re.search(r'\.codec_bin_section_1\s+(0x[0-9A-Fa-f]+)', ld)
        self.assertIsNotNone(m)
        self.assertEqual(int(m.group(1), 16), core.ADDRESSES[0] + core.CODEC_TARGET)


class SameAsLoaderTest(unittest.TestCase):
    """Obraz i plan zapisu muszą być bajt w bajt takie jak w loaderze OpenGD77."""

    @classmethod
    def setUpClass(cls):
        cls.loader = helpers.reference_loader()
        cls.tmp = tempfile.TemporaryDirectory()
        d = Path(cls.tmp.name)
        cls.fw = d / 'fw.bin'
        cls.fw.write_bytes(helpers.firmware())
        cls.short = d / 'short.bin'
        cls.short.write_bytes(helpers.firmware(size=0x50000, seed=5))
        cls.gla = d / 'Polish.gla'
        cls.gla.write_bytes(helpers.language())
        cls.bad_gla = d / 'Old.gla'
        cls.bad_gla.write_bytes(helpers.language(version=0x29))
        cls.donor = d / 'donor.bin'
        cls.donor.write_bytes(helpers.donor())

    @classmethod
    def tearDownClass(cls):
        cls.tmp.cleanup()

    def check(self, model, fw, gla=None, donor=None):
        ref = helpers.reference_ops(self.loader, model, fw, gla, donor or '')
        img = core.build_image(fw.read_bytes(), platform=model,
                               language=gla.read_bytes() if gla else None,
                               donor=donor.read_bytes() if donor else None)
        self.assertGreater(sum(op.startswith('write') for op in ref), 0)
        self.assertEqual(helpers.our_ops(img.data), ref)
        return img

    def test_fm_only(self):
        img = self.check('MD-UV380', self.fw)
        self.assertFalse(img.dmr)
        self.assertIsNone(img.language)

    def test_language_and_codec(self):
        img = self.check('MD-UV380', self.fw, self.gla, self.donor)
        self.assertTrue(img.dmr)
        self.assertEqual(img.language, 'merged')

    def test_language_mismatch(self):
        img = self.check('MD-UV380', self.fw, self.bad_gla, self.donor)
        self.assertEqual(img.language, 'mismatch')

    def test_other_models(self):
        for model in ('MD-2017', 'DM-1701', 'MD-380', 'MD-9600'):
            with self.subTest(model=model):
                self.check(model, self.fw, self.gla, self.donor)

    def test_too_short_for_codec(self):
        img = self.check('MD-UV380', self.short, None, self.donor)
        self.assertFalse(img.dmr)

    @unittest.skipUnless(os.environ.get('RT3S_REAL_FIRMWARE'), 'RT3S_REAL_FIRMWARE nie ustawione')
    def test_real_firmware(self):
        """Opcjonalnie: prawdziwe pliki, RT3S_REAL_FIRMWARE=plik.bin [RT3S_REAL_LANG=Polish.gla] [RT3S_DONOR=...]"""
        fw = Path(os.environ['RT3S_REAL_FIRMWARE'])
        gla = Path(os.environ['RT3S_REAL_LANG']) if os.environ.get('RT3S_REAL_LANG') else None
        donor = Path(os.environ['RT3S_DONOR']) if os.environ.get('RT3S_DONOR') else None
        img = self.check('MD-UV380', fw, gla, donor)
        if gla:
            self.assertEqual(img.language, 'merged')


class ImageTest(unittest.TestCase):
    def test_codec_is_decrypted_donor(self):
        fw, donor = helpers.firmware(), helpers.donor()
        img = core.build_image(fw, donor=donor)
        plain = core.encrypt(img.data, 'MD-UV380')   # XOR drugi raz = odszyfrowanie
        at = core.CODEC_TARGET
        shift = at % 1024
        for j in (0, 1, 1000, core.CODEC_LENGTH - 1):
            self.assertEqual(plain[at + j], donor[core.CODEC_DONOR_OFFSET + j] ^ ciphers.MD9600_CIPHER[(j + shift) % 1024])
        self.assertEqual(plain[:at], fw[:at])
        self.assertEqual(plain[at + core.CODEC_LENGTH:], fw[at + core.CODEC_LENGTH:])

    def test_language_slot(self):
        fw, gla = helpers.firmware(), helpers.language()
        plain = core.encrypt(core.build_image(fw, language=gla).data, 'MD-UV380')
        self.assertEqual(plain[helpers.SLOT_AT:helpers.SLOT_AT + len(gla)], gla)
        self.assertEqual(core.firmware_language_version(fw), helpers.LANG_VERSION)
        self.assertEqual(core.language_version(gla), helpers.LANG_VERSION)

    def test_ignore_language(self):
        fw = bytearray(helpers.firmware())
        fw[0x100:0x108] = b'IGNRLANG'
        self.assertEqual(core.build_image(bytes(fw), language=helpers.language()).language, 'ignored')

    def test_bad_language_file(self):
        self.assertEqual(core.build_image(helpers.firmware(), language=b'x' * 100).language, 'badfile')

    def test_rejects_official_firmware(self):
        with self.assertRaises(core.FirmwareError):
            core.build_image(core.OFFICIAL_HEADER + b'\0' * 1000)

    def test_rejects_too_large(self):
        with self.assertRaises(core.FirmwareError):
            core.build_image(b'\0' * (core.MAX_FIRMWARE + 1))

    def test_largest_image_fits_block_numbers(self):
        plan = core.write_plan(b'\0' * core.MAX_FIRMWARE)
        for s, last in zip(plan, core.BLOCK_ENDS):
            self.assertEqual(s.blocks[-1][0], last)
            self.assertEqual(len(s.blocks) * core.BLOCK_SIZE, s.size)

    def test_unknown_model(self):
        with self.assertRaises(core.FirmwareError):
            core.build_image(helpers.firmware(), platform='GD-77')


if __name__ == '__main__':
    unittest.main()
