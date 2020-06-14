
with open('fcbanner.ans', 'rb') as ansi:
  data = ansi.read()
  
with open('b3_fcbanner.asm', 'w') as h:
  h.write('\tDEF\tbandata\n')
  h.write('\n')
  h.write('bandata:\n')
  h.write('\tBYTE\t0x0A\n')
  h.write('\tBYTE\t0x0A\n')
  for b in data:
    h.write('\tBYTE\t0x%02x\n' % (b))
  h.write('\tBYTE\t0x00\n')

