echo off
setlocal

pushd LevelEditor\schemas
call GenSchemaDef.bat
popd

pushd LevelEditorNativeRendering\LvEdRenderingEngine\Bridge
call GenSchemaObjects.cmd
popd

endlocal

pause