#include <time.h>
#include "bool.h"
/* Naive implementation of mktime for compiling under Turbo C */
/* (doesn't handle leap seconds or argument normalisation)    */
bool isLeapYear(int y){
   bool retVal = false;
   
   if((y % 400) == 0)
      retVal = true;
   else if((y % 100) == 0)
      retVal = false;
   else if((y % 4) == 0)
      retVal = true;
   else
      retVal = false;
   return retVal;
}

time_t mktime(struct tm *theDate){
   time_t retVal = 0;
   const time_t secsPerMin  = 60;
   const time_t secsPerHour = secsPerMin * 60;
   const time_t secsPerDay  = secsPerHour * 24;
   const time_t secsPerYear = secsPerDay * 365;
   const time_t secsPerLeapYear = secsPerDay * 366;
   int currYear;
   int currMonth;
   int i;

   currYear = 1900 + theDate->tm_year;
   for(i = 1970; i < currYear; i++){
      if(isLeapYear(i)){
         retVal += secsPerLeapYear;
      }
      else{
         retVal += secsPerYear;
      }
   }
   currMonth = 1 + theDate->tm_mon;
   for(i = 1; i < currMonth; i++){
	  switch(i){
         case 1:
         case 3:
         case 5:
         case 7:
         case 8:
         case 10:
         case 12:
            retVal += secsPerDay * 31;
            break;
         case 4:
         case 6:
         case 9:
         case 11:
            retVal += secsPerDay * 30;
            break;
         case 2:
            if(isLeapYear(currYear)){
               retVal += secsPerDay * 29;
            }
            else{
               retVal += secsPerDay * 28;
            }
            break;
      }
   }
   retVal += (theDate->tm_mday - 1) * secsPerDay;
   retVal += (theDate->tm_hour) * secsPerHour;
   retVal += (theDate->tm_min) * secsPerMin;
   retVal += theDate->tm_sec;
   
   return retVal;
}

