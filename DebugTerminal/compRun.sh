fName=DebugTerminal
directory="/Users/nallison/Library/CloudStorage/OneDrive-MinutemanRegionalVocationalTechnicalSchoolDistrict/ProgrammingInClassPC/myC/$fName"
cd $directory
gcc $fName.c -o $fName -Wall || exit 1
$directory/$fName