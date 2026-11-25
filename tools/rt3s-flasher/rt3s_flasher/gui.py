"""Okno programu (Tkinter): to samo co flasher WebUSB, ale bez przeglądarki."""
import queue
import threading
import tkinter as tk
from pathlib import Path
from tkinter import filedialog, messagebox, ttk

from . import __version__, core, dfu, job, settings

HOWTO = ('1. Wyłącz radio. Trzymając PTT i boczny przycisk nad nim, włącz je: ekran zostaje pusty, dioda miga.\n'
         '2. Podłącz kabel USB.\n'
         '3. Wybierz plik firmware (i ewentualnie język), kliknij „Wgraj”.')


class App:
    def __init__(self, root):
        self.root = root
        self.events = queue.Queue()
        self.busy = False
        self.donor_path, self.donor_data = settings.load_donor()

        root.title(f'RT3S flasher {__version__}')
        root.minsize(560, 0)
        main = ttk.Frame(root, padding=12)
        main.grid(sticky='nsew')
        root.columnconfigure(0, weight=1)
        main.columnconfigure(1, weight=1)

        ttk.Label(main, text=HOWTO, wraplength=540, justify='left').grid(row=0, column=0, columnspan=3,
                                                                          sticky='w', pady=(0, 10))

        self.firmware = tk.StringVar()
        self.lang = tk.StringVar()
        self.model = tk.StringVar(value=core.DEFAULT_PLATFORM)

        self._file_row(main, 1, 'Firmware (.bin):', self.firmware, self.pick_firmware)
        self._file_row(main, 2, 'Drugi język (.gla):', self.lang, self.pick_lang, clear=True)

        ttk.Label(main, text='Model:').grid(row=3, column=0, sticky='w')
        ttk.Combobox(main, textvariable=self.model, values=list(core.PLATFORMS), state='readonly',
                     width=12).grid(row=3, column=1, sticky='w', pady=2)

        ttk.Label(main, text='Kodek DMR:').grid(row=4, column=0, sticky='nw')
        self.donor_label = ttk.Label(main, wraplength=360, justify='left')
        self.donor_label.grid(row=4, column=1, sticky='w', pady=2)
        box = ttk.Frame(main)
        box.grid(row=4, column=2, sticky='nw')
        ttk.Button(box, text='Wybierz…', command=self.pick_donor).pack(side='left')
        self.forget_btn = ttk.Button(box, text='Zapomnij', command=self.forget_donor)
        self.forget_btn.pack(side='left', padx=(4, 0))

        self.summary = tk.Text(main, height=7, width=70, wrap='word', relief='flat',
                               background=root.cget('background'))
        self.summary.grid(row=5, column=0, columnspan=3, sticky='ew', pady=8)
        self.summary.configure(state='disabled')

        self.progress = ttk.Progressbar(main, maximum=100)
        self.progress.grid(row=6, column=0, columnspan=3, sticky='ew')
        self.status = ttk.Label(main, text='')
        self.status.grid(row=7, column=0, columnspan=3, sticky='w', pady=(4, 8))

        btns = ttk.Frame(main)
        btns.grid(row=8, column=0, columnspan=3, sticky='e')
        self.radio_label = ttk.Label(btns, text='')
        self.radio_label.pack(side='left', padx=(0, 12))
        self.flash_btn = ttk.Button(btns, text='Wgraj', command=self.flash)
        self.flash_btn.pack(side='left')

        for v in (self.firmware, self.lang, self.model):
            v.trace_add('write', lambda *_: self.refresh())
        root.protocol('WM_DELETE_WINDOW', self.close)
        self.refresh()
        self.poll_radio()
        self.root.after(100, self.pump)

    def _file_row(self, parent, row, label, var, command, clear=False):
        ttk.Label(parent, text=label).grid(row=row, column=0, sticky='w')
        ttk.Entry(parent, textvariable=var).grid(row=row, column=1, sticky='ew', pady=2, padx=(0, 4))
        box = ttk.Frame(parent)
        box.grid(row=row, column=2, sticky='w')
        ttk.Button(box, text='Wybierz…', command=command).pack(side='left')
        if clear:
            ttk.Button(box, text='Brak', command=lambda: var.set('')).pack(side='left', padx=(4, 0))

    # ------------------------------------------------------------------ pliki
    def pick_firmware(self):
        p = filedialog.askopenfilename(title='Firmware OpenGD77', filetypes=[('Firmware', '*.bin'), ('Wszystkie', '*')])
        if p:
            self.firmware.set(p)
            if not self.lang.get():
                guess = Path(p).with_name('Polish.gla')
                if guess.is_file():
                    self.lang.set(str(guess))

    def pick_lang(self):
        p = filedialog.askopenfilename(title='Paczka językowa', filetypes=[('Język OpenGD77', '*.gla'), ('Wszystkie', '*')])
        if p:
            self.lang.set(p)

    def pick_donor(self):
        p = filedialog.askopenfilename(title='Oryginalny firmware MD-9600 V26.45',
                                       filetypes=[('Firmware', '*.bin'), ('Wszystkie', '*')])
        if not p:
            return
        try:
            target = settings.remember_donor(p)
        except (ValueError, OSError) as e:
            messagebox.showerror('Plik dawcy', str(e))
            return
        self.donor_path, self.donor_data = target, target.read_bytes()
        self.refresh()

    def forget_donor(self):
        settings.forget_donor()
        self.donor_path, self.donor_data = settings.load_donor()
        self.refresh()

    # ------------------------------------------------------------------ stan
    def set_summary(self, lines):
        self.summary.configure(state='normal')
        self.summary.delete('1.0', 'end')
        self.summary.insert('end', '\n'.join(lines))
        self.summary.configure(state='disabled')

    def refresh(self):
        if self.donor_path:
            self.donor_label.configure(text=f'tak, dawca: {self.donor_path}')
            self.forget_btn.state(['!disabled'])
        else:
            self.donor_label.configure(text='brak pliku dawcy (MD-9600 V26.45): tylko FM')
            self.forget_btn.state(['disabled'])
        self.prepared = None
        if not self.firmware.get():
            self.set_summary(['Wybierz plik firmware.'])
        else:
            try:
                self.prepared = job.prepare(self.firmware.get(), self.lang.get() or None,
                                            self.donor_path, self.donor_data, self.model.get())
                self.set_summary(['Do wgrania:'] + [f'  {x}' for x in self.prepared.info]
                                 + [f'  UWAGA: {x}' for x in self.prepared.warnings])
            except core.FirmwareError as e:
                self.set_summary([f'Błąd: {e}'])
        self.update_button()

    def update_button(self):
        ok = self.prepared is not None and not self.busy
        self.flash_btn.state(['!disabled'] if ok else ['disabled'])

    def poll_radio(self):
        if not self.busy:
            try:
                n = len(dfu.find_radios())
                text = 'Radio: wykryte' if n == 1 else ('Radio: brak' if n == 0 else f'Radio: {n} urządzenia?')
            except dfu.UsbProblem as e:
                text = 'Radio: ?'
                self.status.configure(text=str(e))
            self.radio_label.configure(text=text)
        self.root.after(2000, self.poll_radio)

    # ------------------------------------------------------------------ zapis
    def flash(self):
        p = self.prepared
        if not p:
            return
        if not p.image.dmr and not messagebox.askyesno('Bez DMR', 'Brak kodeka AMBE: radio będzie działać '
                                                       'tylko na FM. Wgrać mimo to?'):
            return
        self.busy = True
        self.update_button()
        self.progress['value'] = 0
        threading.Thread(target=self.worker, args=(p.image.data,), daemon=True).start()

    def worker(self, image):
        def progress(phase, done, total):
            self.events.put(('progress', phase, done, total))
        dev = None
        try:
            dev = dfu.open_radio()
            self.events.put(('status', f'Radio: {dfu.describe(dev)}'))
            dfu.Dfu(dev).flash(image, progress)
            self.events.put(('done', 'Gotowe. Radio uruchomi się samo; jeśli nie, wyłącz je i włącz.'))
        except dfu.UsbProblem as e:
            self.events.put(('error', str(e)))
        except dfu.DfuError as e:
            self.events.put(('error', f'Radio przerwało zapis ({e}). Bootloader jest nienaruszony: wyłącz radio, '
                                      'wejdź jeszcze raz w tryb aktualizacji i wgraj ponownie.'))
        except Exception as e:   # noqa: BLE001 - pokaż każdy błąd w oknie
            self.events.put(('error', f'Błąd: {e}'))
        finally:
            if dev is not None:
                dfu.release(dev)

    def pump(self):
        try:
            while True:
                ev = self.events.get_nowait()
                kind = ev[0]
                if kind == 'progress':
                    _, phase, done, total = ev
                    if phase == 'erase':
                        self.progress['value'] = 10 * done / total
                        self.status.configure(text=f'Kasuję pamięć: {done} z {total}')
                    elif phase == 'write':
                        self.progress['value'] = 10 + 90 * done / total
                        self.status.configure(text=f'Zapisuję: {done * 100 // total}% ({done} z {total} KB)')
                elif kind == 'status':
                    self.status.configure(text=ev[1])
                else:
                    self.busy = False
                    self.update_button()
                    self.status.configure(text=ev[1])
                    if kind == 'done':
                        self.progress['value'] = 100
                        messagebox.showinfo('Gotowe', ev[1])
                    else:
                        messagebox.showerror('Błąd', ev[1])
        except queue.Empty:
            pass
        self.root.after(100, self.pump)

    def close(self):
        if self.busy and not messagebox.askyesno('Trwa zapis', 'Trwa wgrywanie firmware. Zamknięcie przerwie '
                                                 'zapis. Zamknąć mimo to?'):
            return
        self.root.destroy()


def main(smoke_test=False):
    root = tk.Tk()
    App(root)
    if smoke_test:   # CI: zbuduj okno i zamknij
        root.update()
        root.destroy()
        return 0
    root.mainloop()
    return 0
