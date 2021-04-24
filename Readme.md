# SpreaderDetector
My final project in the C-programming language course.

## Background
The COVID19 started spreading in Israel in February 2020.
Most of the closed spaces have been declared as the main places to get infected in, but after the 1st quarentine - no actual solution was found in order to deal with the infections in these spaces.

Therefor, the **SpreaderDetector** - a software that is meant to identify chains of infections, evalute the chances of one to get infected, and update the potential infected people, was developed.
The program is based on data that is extracted out of videos.

The main part of the program is the **SpreaderDetectorBackend**.<br />
<br />

## The SpreaderDetectorBackend
The main part of the SpreaderDetector. 
### The Input
Made of 2 input files.
#### People.in
includes information about people that were cought on videos (ID, name, age).
#### Meetings.in
includes information about the meetings that were 'cought' in the videos.
The file structure:
- **First row**: contains the ID of the spreader.
- **The next rows are 'divided' into batches**: The first batch describes the encounters of the spreader. The next batch will describe the encounters of the first person the spreader met. The batch after will describe the encounters of the second person the spreader met, and so on and so forth..
- The rows structure: 
```bash
<infector_id><infected_id><distance><time>
```

### Processing The Data
According to the data about the people encounters, the program will calculate each person's chances of being infected, and will output a list of the poeple, sorted by the urgency of the case (A person that was probably infected, will be defined as the most urgent case).

### The Output
The output of the program will be a file named:
```bash
SpreaderDetectorAnalysis.out
```
<br />

## Running The Program
The program will run with the following command:
```bash
$ ./SpreaderDetectorBackend <Path to People.in> <Path to Meetings.in>
```

## Attached Files
- **SpreaderDetectorBackend.c** - The main program.
- **SpreaderDetectorParams.h** - Contains built-in constant parameters of the program,
- **in-out-example** - A directory contain an example of input and expected output of the program.
<br />


