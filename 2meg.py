
with open('FCMDC.bin', 'rb') as rom_in:
    rom = rom_in.read()

with open('FCMD2MEG.bin', 'wb') as rom_out:
    chunk_len = len(rom)
    out_len = 0
    iters = 0
    while out_len < 2*1024*1024:
        rom_out.write(rom)
        out_len = out_len + chunk_len
        iters += 1

    print(f'{chunk_len} image * {iters} = {out_len}')

