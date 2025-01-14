fName=$1
directory="/Users/nallison/Library/CloudStorage/OneDrive-MinutemanRegionalVocationalTechnicalSchoolDistrict/ProgrammingInClassPC/myC/raycasting/$fName"
cd $directory
clear
gcc $fName.c -o $fName -Wall -I /opt/homebrew/opt/sdl2/include -I /opt/SDL2_image-2.8.4/build/include -I /opt/SDL2_image-2.8.4/external/libpng/build/include -L /opt/homebrew/opt/sdl2/lib -L /opt/SDL2_image-2.8.4/build/lib -L /opt/SDL2_image-2.8.4/external/libpng/build/lib -lSDL2 -lSDL2_image -lpng -Wl,-rpath,/opt/SDL2_image-2.8.4/build || exit 1
$directory/$fName