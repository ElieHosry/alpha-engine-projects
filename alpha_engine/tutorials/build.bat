@echo off

rem pulling Assets and snippets from relevant folders
powershell Compress-Archive -Path "..\resources\*" -DestinationPath "docs\res\Assets.zip"
powershell Compress-Archive -Path "..\build\mvs2022\snippets\*" -DestinationPath "docs\res\snippets.zip"

mkdocs build 
