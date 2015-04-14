
/*
 * This sketch is designed to interface with SH1106 based OLED LCD over
 * an I2C interface
 *
 * Based in part off of code from: http://wenku.baidu.com/view/43ebc40aba1aa8114431d940.html
 and http://blog.l3l4.com/iot/kitchenscale01/
 Screen is 128x64 pixels, X is 0-127, Y is 0-63

 // For getting Variables to output as string, then char?
  int a=1;     //declaring integer
  char b[2];   //declaring character array
  String str;  //declaring string

  str=String(a); //converting integer into a string
  str.toCharArray(b,2); //passing the value of the string to the character array
  Serial.print("Value of b is \t");  //display that value
  Serial.println(b);

 */

#include "application.h"
#include "sh1106/sh1106.h"

String VAR=           "136.5";  //print a string or variable in your display
char  charVAR[10];  //need a holder for the variable

volatile boolean g_writeValue = false;
volatile int g_displayValue = 0;

sh1106_lcd *glcd = NULL;
void setup()
{
  Serial.begin(9600);

  glcd = sh1106_lcd::getInstance();

  if (glcd != NULL)
  {
    glcd->ClearScreen();
  }
  delay(2000);
}

// Quick sketch showing functionality of library
void loop()
{
    glcd->ClearScreen();
    glcd->DrawRectangle(14, 0, 114, 48, 2); // draw a box from 0, 0 to 100, 48
    glcd->Show();
    delay(500);
    glcd->FillRectangle(25, 25, 40, 40);
    glcd->Show();
    delay(500);
    //  Print the existing alphabet
    glcd->PrintLine(" !\"#$%&'()*+,-./");
    glcd->PrintLine("0123456789:;<=>?@");
    glcd->PrintLine("ABCDEFGHIJKLMNOPQ");
    glcd->PrintLine("RSTUVWXYZ[\ ]^_`");
    glcd->PrintLine("abcdefghijklmnopq");
    glcd->PrintLine("rstuvwxyz{|}~ Fin");
    delay(5000);
    glcd->ClearScreen();

    VAR.toCharArray(charVAR, 6); // Convert string to char for display output.
    //glcd->Print("Usage: ");
    //glcd->Print(charVAR);
    char str[80];
    strcpy (str," Variable=");
    strcat (str,charVAR);
    strcat (str," !");
    glcd->PrintLine("");
    glcd->PrintLine("");
    glcd->PrintLine(str);   //"Usage:"+charVAR+" Watts");
    glcd->DrawRectangle(2, 12, 114, 26, 1); // draw a box around the text
    glcd->Show();

    // add some dots to the edge of the screen
    glcd->DrawPixel(0, 0, true);
    glcd->DrawPixel(127, 0, true);
    glcd->DrawPixel(127, 63, true);
    glcd->DrawPixel(0, 63, true);
    glcd->Show();
    delay(5000);

    glcd->ClearScreen();
    glcd->DrawLine(10, 32, 113, 32); //Straight across L > R
    glcd->DrawLine(63, 0, 63, 63);  // Straight down
    glcd->Show();
    delay(500);
    glcd->DrawLine(0, 0, 63, 32);  // Diag
    glcd->Show();
    //glcd->DrawLine(63, 32, 123, 0);  // Diag up dosn't work
    glcd->DrawLine(31, 0, 63, 32);  // Diag
    glcd->DrawLine(0, 16, 63, 32);  // Diag
    glcd->Show();
    delay(3000);
}
