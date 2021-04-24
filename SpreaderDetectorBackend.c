/*
 * File Name: SpreaderDetectorBackend.c
 * Related Files: "SpreaderDetectorParams.h"
 *
 * Purpose:
 * 			Due to the Corona Virus pandemic spreading around the world in 2020, the Hebrew University
 * 			was asked to create a program that will update people that were potentially infected, to
 * 			the fact that they are in risk, and tell them what should they do next, according to that.
 * 			This program Analayzes data about meetings of people, and outputs recommendation due to it.
 *
 * Author's ID: 206174120
 * Author's Name: Oren Kovartovsky
 * University User Name: orenkov
 *
 * */

#include "SpreaderDetectorParams.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ARGS_COUNT 3
#define FILE_DO_NOT_EXIST -1

#define ERROR_ARGS "Usage: ./SpreaderDetectorBackend <Path to People.in> <Path to Meetings.in>\n"
#define ERROR_INPUT "Error in input files.\n"
#define ERROR_OUTPUT "Error in output file.\n"
#define ERROR_DEAFULT "Standard library error.\n"

#define INPUT_MAX_LINE_LEN 1024

#define PEOPLE_ID_INDEX 0
#define PEOPLE_NAME_INDEX 1

#define MEET_FIRST_ID_INDEX 0
#define MEET_SECOND_ID_INDEX 1
#define MEET_DISTANCE_INDEX 2
#define MEET_TIME_INDEX 3
#define INFECTOR_OLD 0
#define INFECTOR_NEW 1
#define MAX_SEVERITY 1

/*
 * ***************************
 * 	STRUCTS AND ENUMS
 * ***************************
 */
/**
 * @brief Status Code Enum
 *
 * @details Used as: (1) Return Value all over the program, determines if the function succeeded it's action or not.
 * if the function didn't success, the StatusCode Return Value will represent exactly why.
 * @details (2) used by the function error(errorCode), in order to determine the kind of error needed to be
 * printed to stderr.
 */
typedef enum _StatusCode
{
	STATUS_CODE_SUCCESS, 		/**< When a function Success it's action */
	STATUS_CODE_FAIL,		/**< When a function Defaultly Fails it's action */
	STATUS_CODE_ARGS_ERROR,		/**< When a function Fails on Arguments Error */
	STATUS_CODE_INPUT_ERROR,	/**< When a function Fails on Input Error (open or close file) */
	STATUS_CODE_OUTPUT_ERROR,	/**< When a function Fails on Output Error (open or close file) */
	STATUS_CODE_EMPTY_FILE		/**< When a function Detects an empty input file */
	
} StatusCode;

/**
 * @brief A struct represents a Person which might be a coronavirus carrier.
 * @details Used to store relevant data about the person.
 */
typedef struct _Person
{
	int id; 				/**< 9 digits number, represents ID of a person */
	float severity;				/**< Severity, value between 0 to 1 (included) */
	char name[INPUT_MAX_LINE_LEN];		/**< Person's Name */
} Person;

/**
 * @brief A struct represents a Meeting between 2 people that might have coronavirus.
 * @details Used to store the data conveniently during the program.
 */
typedef struct _Meeting
{
	int infectorID;		/**< 9 digits number, represents ID of the potential infector */
	int infectedID;		/**< 9 digits number, represents ID of the potential infected */
	float distance;		/**< distance measured between them. bounded by MIN_DISTANCE */
	float time;		/**< time measured in which they were next to each other. bounded by MAX_TIME. */
} Meeting;


/*
 * ***********************
 * 	DECLARATIONS
 * ***********************
 */
//	****** GENERATING FILES or MESSAGES FUNCTIONS ******
/**
 * @brief A function that generates an empty output file.
 * @details output file name is: OUTPUT_FILE.
 * @details used whenever input is valid - but people's input file is empty.
 * @param[out] STATUS_CODE_SUCCESS if file was generated successfully.
 * @param[out] StatusCode - If ERROR occurred, ERROR/FAILURE returns depends on the error.
 */
StatusCode generateEmptyOutputFile();

/**
 * @brief A function that determines which message a person should get, according to the chances
 * of him being a carrier.
 * @note uses a #define statements from "SpreaderDetectorParams.h" file
 * @return The relevant message
 */
const char* severityToMessage(float severityLevel);

/**
 * @brief A function that generates an output file of the potential carriers, sorted by urgency of treatment.
 * @details output file name is: OUTPUT_FILE.
 * @details output file is divided to 3 sections: Hospitalization needed, Quarantine needed, No serious.
 * @details each of the section is sorted by severity level.
 * @param[in] peopleArray a pointer to the peopleArray, which contains the relevant data about them.
 * @param[in] peopleCounter number of different people which were recorded, and appears in the array.
 * @param[out] STATUS_CODE_SUCCESS if file was generated successfully.
 * @param[out] StatusCode - If ERROR occurred, ERROR/FAILURE returns depends on the error.
 */
StatusCode generateSeverityFile(Person *peopleArray, unsigned int peopleCounter);

/**
 * @brief A function that writes errors to stderr.
 * @details being used in the program at the moment error detected.
 * @details the function holds many kinds of errors. according to the errorCode given to the function,
 * the relevant message is written to stderr.
 * @param[in] errorCode The relevant StatusCode, representing the error occurred.
 */
void error(StatusCode errorCode);


//	****** COMPARISON FUNCTIONS ******
/**
 * @brief compares between 2 Person objects, according to their ID.
 *
 * @details ID is 9 digits unsigned int, and the comparision uses the order theory '<'.
 * @param[in] personA a void pointer, represents pointer to 1st person to compare.
 * @param[in] personB personA a void pointer, represents pointer to 2nd person to compare.
 * @return integer: 0 if equal, negative integer if personA has bigger ID than personB,
 * and positive integer if personA has smaller ID than personB.
 */
int personCompareByID(const void* personA, const void* personB);

/**
 * @brief compares between 2 Person objects, according to their severity field.
 *
 * @details the severity field is a **float** one.
 * @param[in] personA a void pointer, represents pointer to 1st person to compare.
 * @param[in] personB personA a void pointer, represents pointer to 2nd person to compare.
 * @return integer: 0 if equal, negative integer if personA has bigger ID than personB,
 * and positive integer if personA has smaller ID than personB.
 */
int personCompareBySeverity(const void* personA, const void* personB);


//	****** PARSING / CALCULATING / PROCESSING DATA FUNCTIONS ******
/**
 * @brief A function that parsing the lines from the input peopleFile.
 *
 * @note The function handle with a certain given input (Extracting 2 arguments only from the line,
 * in a certain order). Therefor, if the format or the needed details are to be changed, more tokens-handling
 * will be needed.
 * @param[in] personReceiver holds all the variables that gonna be 'fed' with the data in the line.
 * @param[in] lineToRead a pointer to the beginning of the line gonna be parsed by the function.
 * @param[out] STATUS_CODE_SUCCESS If data was stored successfully.
 * @param[out] StatusCode - If ERROR occurred, ERROR/FAILURE returns depends on the error.
 */
StatusCode parsePersonLine(Person *personReceiver, char *lineToRead);

/**
 * @brief A function that parsing the lines from the input meetingsFile.
 *
 * @param[in] meetingReceiver holds all the variables that gonna be 'fed' with the data in the line.
 * @param[in] lineToRead a pointer to the beginning of the line gonna be parsed by the function.
 * @param[in] infectorStatus a pointer to a varible that holds the status of the current infector (NEW or OLD).
 * if it is old, it won't be searched again in the array.
 * @param[in] curInfector the ID of the current infector examined by the program.
 * @param[out] STATUS_CODE_SUCCESS If data was stored successfully.
 * @param[out] StatusCode - If ERROR occurred, ERROR/FAILURE returns depends on the error.
 */
StatusCode parseMeetingLine(Meeting *meetingReceiver, char *lineToRead, unsigned int *infectorStatus, int *curInfector);

/**
 * @brief calculates the chances of infection in coronavirus between 2 people, according to given data.
 *
 * @details Special function, was developed by one of the staff members of Emzeti-Shem University in the US.
 * @details Using data given about a meeting of 2 people, and determines the chances of infection in corona.
 * @param[in] distance distance measured between the people.
 * @param[in] time measured in which they were next to each other.
 * @return float value between 0 to 1, represents the chances.
 */
float crna(float distance, float time);

/**
 * @brief A function that calculates the chances of infection for each person.
 *
 * @details the function uses previous data about the people, if exists - in order to make the calculations
 * faster.
 * @param[in] meetingFile the files that contains the data about meetings, used to calculate
 * infection chances.
 * @param[in] peopleArray an array of all the people that were recorded.
 * @param[in] peopleCounter the amount of the people that were recorded.
 * @param[out] STATUS_CODE_SUCCESS If calculation was succesfull.
 * @param[out] StatusCode - If ERROR occurred, ERROR/FAILURE returns depends on the error.
 */
StatusCode calculateSeverities(FILE *meetingFile, Person *peopleArray, unsigned int peopleCounter);

/**
 * @brief The function reads, process, and sorts (By ID) the peopleFile.
 *
 * @note Including OPEN and CLOSE of the file.
 * @note The function ALLOCATES MEMORY being stored in peopleArray - a var declared outside, and being used
 * later in the program.
 * @note peopleArray's memory is NOT released in that function, even in failure.
 * @details Using Dynamic Array Method while allocating new memory, to save running time complexity.
 * @param[in] meetingFilePath argv path for the file.
 * @param[in] peopleArray pointer to the array of people (The Data Structure being used). Being allocated.
 * @param[in] peopleCounter number of people in the DS, used for sorting, calculation, and later in program.
 * @param[out] STATUS_CODE_SUCCESS If processing was successful.
 * @param[out] STATUS_CODE_EMPTY_FILE If the people's file is empty.
 * @param[out] StatusCode - If ERROR occurred, ERROR/FAILURE returns depends on the error.
 */
StatusCode peopleProcessAndSort(char* peopleFilePath, Person **peopleArray, unsigned int *peopleCounter);

/**
 * @brief a function that reads, processes and calculate data from meetingFile.
 *
 * @note The function OPENS and CLOSE meetingFile.
 * @note peopleArray's memory is NOT released in that function, even in failure.
 * @param[in] meetingFilePath argv path for the file.
 * @param[in] peopleArray pointer to the array of people (The Data Structure being used).
 * @param[in] peopleCounter number of people in the DS, used for sorting and calculation.
 * @param[out] STATUS_CODE_SUCCESS If processing was successful.
 * @param[out] StatusCode - If ERROR occurred, ERROR/FAILURE returns depends on the error.
 */
StatusCode meetingsProcess(char* meetingFilePath, Person **peopleArray, const unsigned int *peopleCounter);

/**
 * @brief The function that 'holds' all the relevant actions/functions together, in order to proccess
 * and determine the order of the potential infected people.
 *
 * @details the reads data, sorts it and using calculations in order to create the expected output File.
 * @note varible peopleArray - The DS being used in the program.
 * The memory allocated to it is released in this func, even though it was allocated in a sub-function.
 * peopleArray is being by other sub-functions as well.
 * @param[out] STATUS_CODE_SUCCESS If processing was successfull.
 * @param[out] StatusCode - If ERROR occurred, ERROR/FAILURE returns depends on the error.
 */
StatusCode spreaderDetector(char* peopleFile, char* meetingFile);


/*
 * ***********************
 * 	DEFINITIONS
 * ***********************
 */
/**
 * @brief A function that generates an empty output file.
 *
 * @details output file name is: OUTPUT_FILE.
 * @details used whenever input is valid - but people's input file is empty.
 * @param[out] STATUS_CODE_SUCCESS if file was generated successfully.
 * @param[out] StatusCode - If ERROR occurred, ERROR/FAILURE returns depends on the error.
 */
StatusCode generateEmptyOutputFile()
{
	FILE* outputFile = fopen(OUTPUT_FILE, "w");
	if (outputFile == NULL)
	{
		error(STATUS_CODE_OUTPUT_ERROR);
		return STATUS_CODE_OUTPUT_ERROR;
	}
	
	if (EOF == fclose(outputFile))
	{
		error(STATUS_CODE_OUTPUT_ERROR);
		return STATUS_CODE_OUTPUT_ERROR;
	}
	return STATUS_CODE_SUCCESS;
}


/**
 * @brief A function that determines which message a person should get, according to the chances
 * of him being a carrier.
 * @note uses a #define statements from "SpreaderDetectorParams.h" file
 * @return The relevant message
 */
const char* severityToMessage(float severityLevel)
{
	// Higest Risk:
	if (severityLevel >= MEDICAL_SUPERVISION_THRESHOLD)
	{
		return (MEDICAL_SUPERVISION_THRESHOLD_MSG);
	}
	
	// Med-Level Risk:
	else if ((severityLevel < MEDICAL_SUPERVISION_THRESHOLD) &&
			 (severityLevel) >= REGULAR_QUARANTINE_THRESHOLD)
	{
		return (REGULAR_QUARANTINE_MSG);
	}
	
	// Low-Level Risk:
	else
	{
		return (CLEAN_MSG);
	}
}

/**
 * @brief A function that generates an output file of the potential carriers, sorted by urgency of treatment.
 * @details output file name is: OUTPUT_FILE.
 * @details output file is divided to 3 sections: Hospitalization needed, Quarantine needed, No serious.
 * @details each of the section is sorted by severity level
 * @param[in] peopleArray a pointer to the peopleArray, which contains the relevant data about them.
 * @param[in] peopleCounter number of different people which were recorded, and appears in the array.
 * @param[out] STATUS_CODE_SUCCESS if file was generated successfully.
 * @param[out] StatusCode - If ERROR occurred, ERROR/FAILURE returns depends on the error.
 */
StatusCode generateSeverityFile(Person *peopleArray, const unsigned int peopleCounter)
{
	//	Create an empty file:
	FILE* outputFile = fopen(OUTPUT_FILE, "w");
	if (outputFile == NULL)
	{
		error(STATUS_CODE_OUTPUT_ERROR);
		return STATUS_CODE_OUTPUT_ERROR;
	}
	
	// Initialize resources to iterate over the People-Array:
	int index = ((int) peopleCounter - 1);
	Person curPerson = {0};
	while (index >= 0)
	{
		curPerson = *(peopleArray + index);
		
		if (0 > fprintf(outputFile, severityToMessage(curPerson.severity),
			curPerson.name, (unsigned long) curPerson.id))
		{
			fclose(outputFile);
			error(STATUS_CODE_FAIL);
			return STATUS_CODE_FAIL;
		}
		index--;
	}
	
	if (EOF == fclose(outputFile))
	{
		error(STATUS_CODE_OUTPUT_ERROR);
		return STATUS_CODE_OUTPUT_ERROR;
	}
	return STATUS_CODE_SUCCESS;
}


/**
 * @brief A function that writes errors to stderr.
 * @details being used in the program at the moment error detected.
 * @details the function holds many kinds of errors. according to the errorCode given to the function,
 * the relevant message is written to stderr.
 * @param[in] errorCode The relevant StatusCode, representing the error occurred.
 */
void error(StatusCode errorCode)
{
	switch (errorCode)
	{
		case STATUS_CODE_ARGS_ERROR:
			fprintf(stderr, ERROR_ARGS);
			break;
		case STATUS_CODE_INPUT_ERROR:
			fprintf(stderr, ERROR_INPUT);
			break;
		case STATUS_CODE_OUTPUT_ERROR:
			fprintf(stderr, ERROR_OUTPUT);
			break;
		default:
			fprintf(stderr, ERROR_DEAFULT);
	}
}


/**
 * @brief compares between 2 Person objects, according to their ID.
 * @details ID is 9 digits unsigned int, and the comparision uses the order theory '<'.
 * @param[in] personA a void pointer, represents pointer to 1st person to compare.
 * @param[in] personB personA a void pointer, represents pointer to 2nd person to compare.
 * @return integer: 0 if equal, negative integer if personA has bigger ID than personB,
 * and positive integer if personA has smaller ID than personB.
 */
int personCompareByID(const void* personA, const void* personB)
{
	const Person *pA = (Person*) personA;
	const Person *pB = (Person*) personB;
	return ((pA->id) - (pB->id));
}


/**
 * @brief compares between 2 Person objects, according to their severity field.
 * @details the severity field is a **float** one.
 * @param[in] personA a void pointer, represents pointer to 1st person to compare.
 * @param[in] personB personA a void pointer, represents pointer to 2nd person to compare.
 * @return integer: 0 if equal, negative integer if personA has bigger ID than personB,
 * and positive integer if personA has smaller ID than personB.
 */
int personCompareBySeverity(const void* personA, const void* personB)
{
	const Person *pA = (Person*) personA;
	const Person *pB = (Person*) personB;
	float severityA = pA->severity;
	float severityB = pB->severity;
	if (severityA < severityB)
	{
		return -1;
	}
	else if (severityA > severityB)
	{
		return 1;
	}
	return 0;
}



/**
 * @brief A function that parsing the lines from the input peopleFile.
 * @note The function handle with a certain given input (Extracting 2 arguments only from the line,
 * in a certain order). Therefor, if the format or the needed details are to be changed, more tokens-handling
 * will be needed.
 * @param[in] personReceiver holds all the variables that gonna be 'fed' with the data in the line.
 * @param[in] lineToRead a pointer to the beginning of the line gonna be parsed by the function.
 * @param[out] STATUS_CODE_SUCCESS If data was stored successfully.
 * @param[out] StatusCode - If ERROR occurred, ERROR/FAILURE returns depends on the error.
 */
StatusCode parsePersonLine(Person *personReceiver, char *lineToRead)
{
	short index = 0;
	char *checkPtr = NULL;
	char *chunk = strtok(lineToRead, " ");
	while (chunk != NULL)
	{
		if (index == PEOPLE_ID_INDEX)
		{
			strncpy(personReceiver->name, chunk, sizeof(personReceiver->name));
		}
		
		else if (index == PEOPLE_NAME_INDEX)
		{
			personReceiver->id = (int) strtol(chunk, &checkPtr, 10);
			if (personReceiver->id == 0) // Input can't be 0 by assumption.
			{
				error(STATUS_CODE_FAIL);
				return STATUS_CODE_FAIL;
			}
		}
		chunk = strtok(NULL, " ");
		index ++;
	}
	return STATUS_CODE_SUCCESS;
}


/**
 * @brief A function that parsing the lines from the input meetingsFile.
 * @param[in] meetingReceiver holds all the variables that gonna be 'fed' with the data in the line.
 * @param[in] lineToRead a pointer to the beginning of the line gonna be parsed by the function.
 * @param[in] infectorStatus a pointer to a varible that holds the status of the current infector (NEW or OLD).
 * if it is old, it won't be searched again in the array.
 * @param[in] curInfector the ID of the current infector examined by the program.
 * @param[out] STATUS_CODE_SUCCESS If data was stored successfully.
 * @param[out] StatusCode - If ERROR occurred, ERROR/FAILURE returns depends on the error.
 */
StatusCode parseMeetingLine(Meeting *meetingReceiver, char *lineToRead,
							unsigned int *infectorStatus, int *curInfector)
{
	short index = 0;
	char *checkPtr = NULL;
	char *chunk = strtok(lineToRead, " ");
	
	//	Reading the line using tokens, assigning values to Meeting struct.
	while (chunk != NULL)
	{
		if (index == MEET_FIRST_ID_INDEX)
		{
			meetingReceiver->infectorID = (int) strtol(chunk, &checkPtr, 10);
			if (meetingReceiver->infectorID == 0)	// Input can't be 0 by assumption.
			{
				error(STATUS_CODE_FAIL);
				return STATUS_CODE_FAIL;
			}
			checkPtr = NULL;
			
			//	Check if Infector is NEW:
			if (*curInfector != meetingReceiver->infectorID)
			{
				*curInfector = meetingReceiver->infectorID;
				*infectorStatus = INFECTOR_NEW;
			}
		}
		
		else if (index == MEET_SECOND_ID_INDEX)
		{
			meetingReceiver->infectedID = (int) strtol(chunk, &checkPtr, 10);
			if (meetingReceiver->infectedID == 0)	// Input can't be 0 by assumption.
			{
				error(STATUS_CODE_FAIL);
				return STATUS_CODE_FAIL;
			}
			checkPtr = NULL;
		}
		
		else if (index == MEET_DISTANCE_INDEX)
		{
			meetingReceiver->distance = strtof(chunk, &checkPtr);
		}
		
		else if (index == MEET_TIME_INDEX)
		{
			meetingReceiver->time = strtof(chunk, &checkPtr);
		}
		chunk = strtok(NULL, " ");
		index++;
	}
	return STATUS_CODE_SUCCESS;
}




/**
 * @brief calculates the chances of infection in coronavirus between 2 people, according to given data.
 * @details Special function, was developed by one of the staff members of Emzeti-Shem University in the US.
 * @details Using data given about a meeting of 2 people, and determines the chances of infection in corona.
 * @param[in] distance distance measured between the people.
 * @param[in] time measured in which they were next to each other.
 * @return float value between 0 to 1, represents the chances.
 */
float crna(float distance, float time)
{
	float maxTime = MAX_TIME;
	float minDistance = MIN_DISTANCE;
	return ((time * minDistance) / (distance * maxTime));
}

/**
 * @brief A function that calculates the chances of infection for each person.
 * @details the function uses previous data about the people, if exists - in order to make the calculations
 * faster.
 * @param[in] meetingFile the files that contains the data about meetings, used to calculate
 * infection chances.
 * @param[in] peopleArray an array of all the people that were recorded.
 * @param[in] peopleCounter the amount of the people that were recorded.
 * @param[out] STATUS_CODE_SUCCESS If calculation was succesfull.
 * @param[out] StatusCode - If ERROR occurred, ERROR/FAILURE returns depends on the error.
 */
StatusCode calculateSeverities(FILE *meetingFile, Person *peopleArray, unsigned int peopleCounter)
{
	//	Initialize Resources:
	char lineToRead[INPUT_MAX_LINE_LEN] = {0};
	char *ptr = NULL;
	unsigned int infectorStatus = INFECTOR_OLD;		// Used to determine when new search needed.
	Meeting meetingReceiver = {0};				// Used to receive data from each line.
	Person personKey = {0};					// Used for binary search on People's Array.
	
	//	Read First Line (of the verified carrier). if file is empty, EXIT with no error.
	if (fgets(lineToRead, INPUT_MAX_LINE_LEN, meetingFile) == NULL)
	{
		return STATUS_CODE_SUCCESS;
	}
	
	lineToRead[strlen(lineToRead) - 1] = '\0';
	int infectorID = (int) strtol(lineToRead, &ptr, 10);
	personKey.id = infectorID;
	memset(lineToRead, 0, sizeof(lineToRead));
	
	// Searching the carrier and initializing his severity level:
	Person *infector = (Person*) bsearch(&personKey, peopleArray, peopleCounter, sizeof(Person), personCompareByID);
	infector->severity = MAX_SEVERITY;
	Person *infected = NULL;
	
	//	Parsing Line by Line
	while (fgets(lineToRead, INPUT_MAX_LINE_LEN, meetingFile))
	{
		if (parseMeetingLine(&meetingReceiver, lineToRead, &infectorStatus, &infectorID) == STATUS_CODE_FAIL)
		{
			return STATUS_CODE_FAIL;
		}
		
		personKey.id = meetingReceiver.infectedID;
		infected = (Person*) bsearch(&personKey, peopleArray, peopleCounter, sizeof(Person), personCompareByID);
		
		//	If The Infector changed, search for the new one in the array:
		if (infectorStatus == INFECTOR_NEW)
		{
			personKey.id = meetingReceiver.infectorID;
			infector = (Person*) bsearch(&personKey, peopleArray, peopleCounter, sizeof(Person), personCompareByID);
			infectorStatus = INFECTOR_OLD;
		}
		
		//	Calculating Severity of the infected person:
		infected->severity = ((infector->severity) * (crna(meetingReceiver.distance, meetingReceiver.time)));
		
		memset(&meetingReceiver, 0, sizeof(meetingReceiver));
		memset(lineToRead, 0, sizeof(lineToRead));
	}
	
	return STATUS_CODE_SUCCESS;
}


/**
 * @brief The function reads, process, and sorts (By ID) the peopleFile.
 * @note Including OPEN and CLOSE of the file.
 * @note The function ALLOCATES MEMORY being stored in peopleArray - a var declared outside, and being used
 * later in the program.
 * @note peopleArray's memory is NOT released in that function, even in failure.
 * @details Using Dynamic Array Method while allocating new memory, to save running time complexity.
 * @param[in] meetingFilePath argv path for the file.
 * @param[in] peopleArray pointer to the array of people (The Data Structure being used). Being allocated.
 * @param[in] peopleCounter number of people in the DS, used for sorting, calculation, and later in program.
 * @param[out] STATUS_CODE_SUCCESS If processing was successful.
 * @param[out] STATUS_CODE_EMPTY_FILE If the people's file is empty.
 * @param[out] StatusCode - If ERROR occurred, ERROR/FAILURE returns depends on the error.
 */
StatusCode peopleProcessAndSort(char* peopleFilePath, Person **peopleArray, unsigned int *peopleCounter)
{
	// ## OPEN INPUT FILE ## - Open peopleFile & Checking Existence:
	FILE* peopleFile = fopen(peopleFilePath, "r");
	if (peopleFile == NULL)
	{
		error(STATUS_CODE_INPUT_ERROR);
		return STATUS_CODE_INPUT_ERROR;
	}
	
	// ## INITIATION OF RESOURCES ##
	char lineToRead[INPUT_MAX_LINE_LEN] = {0};
	Person personReceiver = {0};
	Person *temporaryArrayPointer = NULL;
	long arrSize = 0, arrSizeWithBuffer = 0;
	
	// ## PROCCESSING INPUT ## - Start reading line by line, allocating Data in an dynamic array:
	while (fgets(lineToRead, INPUT_MAX_LINE_LEN, peopleFile))
	{
		// Replace the 'new line' from the string :
		lineToRead[strlen(lineToRead) - 1] = '\0';
		if (parsePersonLine(&personReceiver, lineToRead) != STATUS_CODE_SUCCESS)
		{
			if (EOF == fclose(peopleFile))
			{
				error(STATUS_CODE_INPUT_ERROR);
			}
			peopleFile = NULL;
			return STATUS_CODE_FAIL;
		}
		(*peopleCounter)++;
		
		// ## MEMORY ALLOCATION ## - (Dynamic Array)
		arrSize = (((long)*peopleCounter) * ((long)sizeof(Person)));
		if (arrSize >= arrSizeWithBuffer) // '>' only at first iteration.
		{
			arrSizeWithBuffer = (2 * arrSize);
			temporaryArrayPointer = (Person *) realloc(*peopleArray, arrSizeWithBuffer);
			if (temporaryArrayPointer == NULL)
			{
				error(STATUS_CODE_FAIL);
				if (EOF == fclose(peopleFile))
				{
					error(STATUS_CODE_INPUT_ERROR);
				}
				peopleFile = NULL;
				return STATUS_CODE_FAIL;
			}
			(*peopleArray) = temporaryArrayPointer;
			temporaryArrayPointer = NULL;
		}
		
		(*peopleArray)[(*peopleCounter) - 1] = personReceiver;
		
		memset(&personReceiver, 0, sizeof(personReceiver));
		memset(lineToRead, 0, sizeof(lineToRead));
	}
	
	//	## CLOSE INPUT FILE ## - Open peopleFile & Checking Existence:
	if (EOF == fclose(peopleFile))
	{
		error(STATUS_CODE_INPUT_ERROR);
		return STATUS_CODE_INPUT_ERROR;
	}
	peopleFile = NULL;
	
	//	## EMPTY PEOPLE FILE SCENARIO ##
	if ((*peopleCounter) == 0)
	{
		return STATUS_CODE_EMPTY_FILE;
	}
	
	//	## SORT 1 ## -  Array of People by ID:
	qsort(*peopleArray, *peopleCounter, sizeof(Person), personCompareByID);
	
	return STATUS_CODE_SUCCESS;
}

/**
 * @brief a function that reads, processes and calculate data from meetingFile.
 *
 * @note The function OPENS and CLOSE meetingFile.
 * @note peopleArray's memory is NOT released in that function, even in failure.
 * @param[in] meetingFilePath argv path for the file.
 * @param[in] peopleArray pointer to the array of people (The Data Structure being used).
 * @param[in] peopleCounter number of people in the DS, used for sorting and calculation.
 * @param[out] STATUS_CODE_SUCCESS If processing was successful.
 * @param[out] StatusCode - If ERROR occurred, ERROR/FAILURE returns depends on the error.
 */
StatusCode meetingsProcess(char* meetingFilePath, Person **peopleArray, const unsigned int *peopleCounter)
{
	
	// ## OPEN INPUT FILE ## - Open meetingFile:
	FILE* meetingFile = fopen(meetingFilePath, "r");
	if (meetingFile == NULL)
	{
		error(STATUS_CODE_INPUT_ERROR);
		return STATUS_CODE_INPUT_ERROR;
	}
	
	//	## CALCULATE SEVERITIES ## - of Every Person in The People-Array:
	if (calculateSeverities(meetingFile, *peopleArray, *peopleCounter) != STATUS_CODE_SUCCESS)
	{
		if (EOF == fclose(meetingFile))
		{
			error(STATUS_CODE_INPUT_ERROR);
		}
		meetingFile = NULL;
		return STATUS_CODE_FAIL;
	}
	
	//	## CLOSE INPUT FILE ## - close meetingFile ##
	if (EOF == fclose(meetingFile))
	{
		error(STATUS_CODE_INPUT_ERROR);
		return STATUS_CODE_INPUT_ERROR;
	}
	meetingFile = NULL;
	
	//	## SORT 2 ## -  Array of People by Severity:
	qsort(*peopleArray, *peopleCounter, sizeof(Person), personCompareBySeverity);
	
	return STATUS_CODE_SUCCESS;
}

/**
 * @brief The function that 'holds' all the relevant actions/functions together, in order to proccess
 * and determine the order of the potential infected people.
 * @details the reads data, sorts it and using calculations in order to create the expected output File.
 * @note varible peopleArray - The DS being used in the program.
 * The memory allocated to it is released in this func, even though it was allocated in a sub-function.
 * peopleArray is being by other sub-functions as well.
 * @param[out] STATUS_CODE_SUCCESS If processing was successfull.
 * @param[out] StatusCode - If ERROR occurred, ERROR/FAILURE returns depends on the error.
 */
StatusCode spreaderDetector(char* peopleFilePath, char* meetingFilePath)
{
	// ## INITIATE RESOURCES ##
	unsigned int peopleCounter = 0;
	Person *peopleArray = NULL;	// Will be dynamically allocated and used further in the program.
	
	// ## PROCESS PEOPLE ## - (PeopleFile READ) && (DataStruct Build) && (peopleArray SORT by ID)
	StatusCode retValPProcess = peopleProcessAndSort(peopleFilePath, &peopleArray, &peopleCounter);
	
	if (retValPProcess == STATUS_CODE_EMPTY_FILE)
	{
		if (generateEmptyOutputFile() != STATUS_CODE_SUCCESS)
		{
			return STATUS_CODE_FAIL;
		}
		return STATUS_CODE_SUCCESS;
	}
	else if (retValPProcess != STATUS_CODE_SUCCESS)
	{
		free(peopleArray);
		peopleArray = NULL;
		return STATUS_CODE_FAIL;
	}
	
	// ## PROCESS MEETINGS ## - (MeetingFile READ) && (Chances Calculations) && (peopleArray SORT by chances)
	if (meetingsProcess(meetingFilePath, &peopleArray, &peopleCounter) != STATUS_CODE_SUCCESS)
	{
		free(peopleArray);
		peopleArray = NULL;
		return STATUS_CODE_FAIL;
	}
	
	//	## GENERATE OUTPUT FILE ##
	if (generateSeverityFile(peopleArray, peopleCounter) != STATUS_CODE_SUCCESS)
	{
		free(peopleArray);
		peopleArray = NULL;
		return STATUS_CODE_FAIL;
	}
	
	//	## FREE Memory allocated in <peopleReadSort> function ##
	free(peopleArray);
	peopleArray = NULL;
	
	return STATUS_CODE_SUCCESS;
}


/**
 * @brief The main function. verify input and execute the program.
 */
int main(int argc, char *argv[])
{
	//	Check for Number of Arguments:
	if (argc != ARGS_COUNT)
	{
		error(STATUS_CODE_ARGS_ERROR);
		return EXIT_FAILURE;
	}
	
	//	Check if Input Files exists:
	if ((access(argv[1], F_OK) == FILE_DO_NOT_EXIST) || (access(argv[2], F_OK) == FILE_DO_NOT_EXIST))
	{
		error(STATUS_CODE_INPUT_ERROR);
		return EXIT_FAILURE;
	}
	
	//	Execute the Main Part of The Program:
	if 	(spreaderDetector(argv[1], argv[2]) != STATUS_CODE_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
