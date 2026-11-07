# Utilities

ForceCommand programs built with the GCC SDK. These are shipping utilities,
not examples or API tests; the example and test programs live in
[example/gcc](../../example/gcc).

## Building

```
make -C utilities/gcc/<directory>
```

`make subdirs` from the repository root builds these utilities along with the
examples. Each directory copies its finished program to `FC/BIN/`.

## Programs

| Directory / program | Description |
| --- | --- |
| [charset](charset) / `CHARSET` | Character set utility. |
| [diskimage](diskimage) / `DISKIMAGE` | Copy floppy disks to and from DIS/FIX 128 images. |
| [fcarc](fcarc) / `FCARC`, `FCUNARC` | Archive creation and extraction (FCARC and legacy ARC). See [fcarc/README.md](fcarc/README.md). |
| [fcmenu](fcmenu) / `FCMENU` | Menu utility driven by a display-variable data file. See [fcmenu/README.md](fcmenu/README.md). |
| [font](font) / `FONT`, `MDOSANSI`, `MDOSFONT`, `MSXFONT` | Font and character set conversion. See [font/README.md](font/README.md). |
| [ftp](ftp) / `FTP` | FTP client. |
| [telnet](telnet) / `TELNET` | Telnet client. |
| [virgil](virgil) / `VIRGIL99` | Gemini browser. See [virgil/README.md](virgil/README.md). |
