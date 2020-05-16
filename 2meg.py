
with open('FCMDC.bin', 'rb') as rom_in:
    rom64 = rom_in.read()

with open('FCMD2MEG.bin', 'wb') as rom_out:
    chunk_len = len(rom64)
    out_len = 0
    while out_len < 2*1024*1024:
        rom_out.write(rom64)
        out_len = out_len + chunk_len

