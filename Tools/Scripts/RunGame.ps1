
. $PSScriptRoot/SetupEnv.ps1

# Compile C++ for Development.
& $ubt -project="$uprojectPath" $projectName Win64 Development `
	-WaitMutex `
	-FromMsBuild `

& $unreal $uprojectPath -game -log
