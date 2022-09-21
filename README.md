# RollDiff
A clone of the [rdiff](https://linux.die.net/man/1/rdiff) utility

# Build
1. clone this repo or download the source and unpack it to folder *RollDiff*
2. position yourself inside the *RollDiff* folder and type: **cmake -S src -B build**
3. build the project inside the *build* folder

# Running the tool 
- create signature for old file
RollDiffApp signature old-file signature-file

- create delta using signature file and new file
RollDiffApp delta signature-file new-file delta-file

- generate new file using old file and delta
RollDiffApp patch old-file delta-file patched-file

- test
diff -s patched-file new-file