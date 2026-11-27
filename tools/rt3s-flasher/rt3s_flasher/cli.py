"""rt3s-flasher: wgrywanie firmware OpenGD77 / SP7TQ do Retevis RT3S (MD-UV380) z Linuksa."""
import argparse
import os
import sys
from pathlib import Path

from . import CREDITS, __version__, core, dfu, job, settings

EPILOG = '''przykłady:
  rt3s-flasher                                  okno programu
  rt3s-flasher donor PLIK_MD9600_V26.45.bin     zapamiętaj dawcę kodeka (raz)
  rt3s-flasher flash OpenGD77_MDUV380.bin              wgraj (polski jest wbudowany)
  rt3s-flasher flash OpenGD77_MDUV380.bin --dry-run   bez radia: tylko sprawdzenie plików
  rt3s-flasher list                             czy radio jest widoczne

autor: Greg M0SQQ (GregDoma/Retevis-RT3S); rt3s-flasher --version pokazuje kredyty
'''


def _progress_printer():
    state = {'phase': None}

    def progress(phase, done, total):
        if phase == 'erase':
            line = f'Kasuję pamięć: {done} z {total}'
        elif phase == 'write':
            line = f'Zapisuję: {done * 100 // total:3d}% ({done} z {total} KB)'
        else:
            line = 'Zapis zakończony.'
        end = '\n' if phase != state['phase'] and state['phase'] else ''
        sys.stdout.write(f'{end}\r{line}   ')
        sys.stdout.flush()
        state['phase'] = phase
        if phase == 'done':
            sys.stdout.write('\n')
    return progress


def _donor(args):
    if args.no_dmr:
        return None, None
    if args.donor:
        data = job.read_file(args.donor, 'dawcy')
        if not core.is_donor(data):
            raise core.FirmwareError(f'{args.donor} to nie jest firmware MD-9600 V26.45 (suma SHA-256 się nie zgadza)')
        return Path(args.donor), data
    return settings.load_donor()


def cmd_flash(args):
    donor, donor_data = _donor(args)
    p = job.prepare(args.firmware, args.lang, donor, donor_data, args.model)
    print('Do wgrania:')
    for line in p.info:
        print(f'  {line}')
    for line in p.warnings:
        print(f'  UWAGA: {line}')

    if args.save_image:
        Path(args.save_image).write_bytes(p.image.data)
        print(f'Zaszyfrowany obraz zapisany: {args.save_image}')
    if args.dry_run:
        print('Próba bez radia (--dry-run): nic nie zostało wysłane.')
        return 0

    if not args.yes:
        if not sys.stdin.isatty():
            print('Brak potwierdzenia: dodaj --yes, gdy uruchamiasz bez terminala.')
            return 2
        if input('Wgrać do radia? [t/N] ').strip().lower() not in ('t', 'tak', 'y', 'yes'):
            print('Przerwano.')
            return 1

    dev = dfu.open_radio()
    print(f'Radio: {dfu.describe(dev)}')
    try:
        dfu.Dfu(dev).flash(p.image.data, _progress_printer())
    finally:
        dfu.release(dev)
    print('Gotowe. Radio uruchomi się samo; jeśli nie, wyłącz je i włącz.')
    return 0


def cmd_list(args):
    radios = dfu.find_radios()
    if not radios:
        print('Brak radia w trybie aktualizacji (0483:df11).')
        return 1
    for d in radios:
        print(dfu.describe(d))
    return 0


def cmd_donor(args):
    if args.forget:
        print('Zapomniano plik dawcy.' if settings.forget_donor() else 'Nie było zapamiętanego pliku dawcy.')
        return 0
    if args.path:
        try:
            target = settings.remember_donor(args.path)
        except ValueError as e:
            print(e)
            return 1
        print(f'Plik dawcy zapamiętany ({target}). DMR będzie działać.')
        return 0
    path, _ = settings.load_donor()
    print(f'Plik dawcy: {path}' if path else 'Brak pliku dawcy: firmware będzie wgrywany tylko z FM.')
    return 0 if path else 1


def cmd_gui(args):
    from . import gui
    return gui.main()


def build_parser():
    ap = argparse.ArgumentParser(prog='rt3s-flasher', description=__doc__, epilog=EPILOG,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--version', action='version', version=f'%(prog)s {__version__}\n{CREDITS}')
    sub = ap.add_subparsers(dest='cmd')

    f = sub.add_parser('flash', help='wgraj firmware do radia')
    f.add_argument('firmware', help='plik firmware .bin (OpenGD77_MDUV380*.bin)')
    f.add_argument('-j', '--lang', metavar='PLIK.gla', help='tylko starsze buildy bez wbudowanego polskiego: drugi język z pliku .gla')
    f.add_argument('-d', '--donor', metavar='PLIK', help='dawca kodeka (domyślnie zapamiętany plik)')
    f.add_argument('--no-dmr', action='store_true', help='bez kodeka AMBE (tylko FM)')
    f.add_argument('-m', '--model', default=core.DEFAULT_PLATFORM, choices=list(core.PLATFORMS),
                   help=f'model radia (domyślnie {core.DEFAULT_PLATFORM} = RT3S)')
    f.add_argument('-n', '--dry-run', action='store_true', help='tylko przygotuj obraz, bez radia')
    f.add_argument('--save-image', metavar='PLIK', help='zapisz zaszyfrowany obraz (do porównań)')
    f.add_argument('-y', '--yes', action='store_true', help='bez pytania o potwierdzenie')
    f.set_defaults(func=cmd_flash)

    sub.add_parser('list', help='pokaż radia w trybie aktualizacji').set_defaults(func=cmd_list)

    d = sub.add_parser('donor', help='zapamiętaj / pokaż / zapomnij plik dawcy kodeka')
    d.add_argument('path', nargs='?', help='oryginalny firmware MD-9600 V26.45 (.bin)')
    d.add_argument('--forget', action='store_true', help='usuń zapamiętaną kopię')
    d.set_defaults(func=cmd_donor)

    sub.add_parser('gui', help='okno programu').set_defaults(func=cmd_gui)
    return ap


def main(argv=None):
    ap = build_parser()
    args = ap.parse_args(argv)
    if not args.cmd:
        if os.environ.get('DISPLAY') or os.environ.get('WAYLAND_DISPLAY'):
            return cmd_gui(args)
        ap.print_help()
        return 0
    try:
        return args.func(args)
    except (core.FirmwareError, dfu.UsbProblem) as e:
        print(f'Błąd: {e}', file=sys.stderr)
        return 1
    except dfu.DfuError as e:
        print(f'\nRadio przerwało zapis ({e}). Bootloader jest nienaruszony: wyłącz radio, wejdź '
              f'jeszcze raz w tryb aktualizacji i wgraj ponownie.', file=sys.stderr)
        return 1
    except KeyboardInterrupt:
        print('\nPrzerwano.', file=sys.stderr)
        return 130


if __name__ == '__main__':
    sys.exit(main())
