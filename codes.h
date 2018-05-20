/* 
 * File:   codes.h
 * Author: Cindy Ding
 *
 * Created on January 18, 2018, 10:50 PM
 */

#ifndef CODES_H
#define	CODES_H

#ifdef	__cplusplus
extern "C" {
#endif


#ifdef	__cplusplus
}
#endif

/*initialize chose prescription, chose number of pills, chose frequency*/
char chose_prescription = 0;
char chose_pillNum = 0;
char chose_ToD = 0;
char chose_frequency = 0;

/*initialize prescription codes*/
const char * P_code1 = "R ";
const char * P_code2 = "F ";
const char * P_code3 = "L ";
const char * P_code4 = "RF ";
const char * P_code5 = "RL ";
const char * P_code6 = "FL ";
const char * P_code7 = "RFL ";

/*initialize number of pills*/
const char one = 1;
const char two = 2;
const char three = 3;

#endif	/* CODES_H */

