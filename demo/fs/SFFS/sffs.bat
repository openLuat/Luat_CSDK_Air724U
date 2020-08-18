set HEXPATH=%1
set FSADDR=%2
set FSSIZE=%3
set SFFS_ROOT=%cd:\=/%/root
set SFFS_JSON=sffs.json
set SFFSIMG=%HEXPATH:\=/%/oldpac/%/sffs.img
set SFFSCMD=dtools.exe fbdevgen

echo {"type":"FBD2","offset":"%FSADDR%","size":"%FSSIZE%","erase_block":"0x8000","logic_block":"0x200","partiton":[{"plain_file":[{"file":"sffs_file.txt","local_file":"%SFFS_ROOT%/file.txt"},{"file":"sffs_dir/sub_sffs_file.txt","local_file":"%SFFS_ROOT%/sub_file.txt"}],"lzma_block_size":"0x8000","offset":"0","count":"0"}]} > %SFFS_JSON%

%SFFSCMD% %SFFS_JSON% %SFFSIMG%
