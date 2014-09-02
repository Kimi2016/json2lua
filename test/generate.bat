
for /R %%I in (*.json) do (
    json2lua.exe %%I
)

pause

