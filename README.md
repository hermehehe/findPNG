# findPNG
Traverses through directories and finds all valid PNGS.

It detects a png file if it has the unqiue 8 byte signature in the beginning (137 80 78 71 13 10 26 10). It then checks for CRC errors. If both tests are succesful the file is a valid, uncorrupted png. Given a directory, it will test every file and sudirectory inside for any valid pngs and print a list of file paths for each. 

One thing I struggled with the most was traversing through subdirectories and reconstructing a file path. After doing some searching to find the IS_DIR() function, it was a matter of testing and adjusting the string concatenation till I got the correct relative path.
