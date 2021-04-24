#ifndef SPREADERDETECTORPARAMS_H
#define SPREADERDETECTORPARAMS_H

/**
 * Minimal distance two people can be in.
 */
#define MIN_DISTANCE 1.0f

/**
 * The length of the video, also the maximal time
 * two people can be seen together.
 */
#define MAX_TIME 30.0f

/**
 * The age which is considered the minimum age of the
 * population in risk to get infected.
 */
#define RISK_AGE 65.0f

/**
 * The output file name.
 */
#define OUTPUT_FILE "SpreaderDetectorAnalysis.out"

/**
 * The threshold which is required to be quarantined,
 * and the message to be printed at the end of it..
 */
#define REGULAR_QUARANTINE_THRESHOLD 0.1f
#define REGULAR_QUARANTINE_MSG "14-days-Quarantine Required: %s %lu.\n" // name id

/**
 * The threshold which is required to be hospitalized,
 * and the message to be printed at the end of it..
 */
#define MEDICAL_SUPERVISION_THRESHOLD  0.3f
#define MEDICAL_SUPERVISION_THRESHOLD_MSG "Hospitalization Required: %s %lu.\n" // name id

/**
 * The threshold which is required to be quarantined,
 * and the message to be printed at the end of it..
 */
#define CLEAN_MSG "No serious chance for infection: %s %lu.\n" // name id

/**
 * This message should be printed to stderr when a standard library error occurs.
 */
#define STANDARD_LIB_ERR_MSG "Standard library error.\n"


#endif //SPREADERDETECTORPARAMS_H