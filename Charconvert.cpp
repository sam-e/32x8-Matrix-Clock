/*
  Charconvert.h - Library for converting char variables.
  Created by Sam.Contarino, April 2, 2021.
  Released into the public domain.
*/

#include "Arduino.h"
#include "Charconvert.h"


int charToInt(int convert)
{
  switch(convert) {  
  case 48:  
    return 0; 
    break;  
  
  case 49:  
    return 1; 
    break;  

  case 50:  
    return 2; 
    break; 

  case 51:  
    return 3; 
    break;  
  
  case 52:  
    return 4; 
    break;  

  case 53:  
    return 5; 
    break; 

  case 54:  
    return 6; 
    break;  
  
  case 55:  
    return 7; 
    break;  

  case 56:  
    return 8; 
    break;

  case 57:  
    return 9; 
    break;  
  
  default:  
  // it contains the optional code  
  //if nothing matches in the above cases, the default statement runs  
  break;  
  }
}