#include "mid_lcd.h"
#include "hw_lcdinit.h"
#include "mid_font.h"

/*
*   Function: Fill the specified area with color
*   Parameters: xsta,ysta---start coordinates
*               xend,yend---end coordinates
*               color--------color to fill
*   Return value: None
*/
void TFT_Fill(uint16_t xsta,uint16_t ysta,uint16_t xend,uint16_t yend,uint16_t color)
{
    uint16_t i=0,j=0;
    TFT_Address_Set(xsta,ysta,xend-1,yend-1);    // Set display range
    for(i=ysta;i<yend;i++)
    {
        for(j=xsta;j<xend;j++)
        {
            TFT_WR_DATA(color);
        }
    }
}

/*
*   Function: Draw a point at specified position
*   Parameters: x,y---point coordinates
*               color--------color to draw
*   Return value: None
*/
void TFT_DrawPoint(uint16_t x,uint16_t y, uint16_t color)
{
    TFT_Address_Set(x,y,x,y);   // Set display range
    TFT_WR_DATA(color);
}

/*
*   Function: Draw a line
*   Parameters: x1,y1   start coordinates
*               x2,y2   end coordinates
*               color   color to draw
*   Return value: None
*/
void TFT_DrawLine(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,uint16_t color)
{
	uint16_t t=0;
	int xerr=0,yerr=0,delta_x=0,delta_y=0,distance=0;
	int incx=0,incy=0,uRow=0,uCol=0;
	delta_x=x2-x1; // Calculate coordinate increments
	delta_y=y2-y1;
	uRow=x1;// Starting point of the line
	uCol=y1;
	if(delta_x>0){
        incx=1; // Set single-step direction
    }
	else if (delta_x==0){
        incx=0;// Vertical line
    }
	else {
        incx=-1;
        delta_x=-delta_x;
    }
	if(delta_y>0){
        incy=1;
    }
	else if (delta_y==0){
        incy=0;// Horizontal line
    }
	else {
        incy=-1;
        delta_y=-delta_y;
    }
	if(delta_x>delta_y){
        distance=delta_x; // Select the basic increment axis
    }
	else {
        distance=delta_y;
    }
	for(t=0;t<distance+1;t++)
	{
		TFT_DrawPoint(uRow,uCol,color);// Draw point
		xerr+=delta_x;
		yerr+=delta_y;
		if(xerr>distance)
		{
			xerr-=distance;
			uRow+=incx;
		}
		if(yerr>distance)
		{
			yerr-=distance;
			uCol+=incy;
		}
	}
}

/*
*   Function: Display a character
*   Parameters: x,y---start coordinates
*               num character to display
*               fc font color
*               bc font background color
*               sizey font size
*               mode:  0 non-overlay mode  1 overlay mode
*   Return value: None
*/
void TFT_ShowChar(uint16_t x,uint16_t y,uint8_t num,uint16_t fc,uint16_t bc,uint8_t sizey,uint8_t mode)
{
	uint8_t temp,sizex,t,m=0;
	uint16_t i,TypefaceNum;// Number of bytes occupied by one character
	uint16_t x0=x;
	sizex=sizey/2;
	TypefaceNum=(sizex/8+((sizex%8)?1:0))*sizey;
	num=num-' ';     // Get offset value
	TFT_Address_Set(x,y,x+sizex-1,y+sizey-1);  // Set cursor position
	for(i=0;i<TypefaceNum;i++)
	{
		if(sizey==16){
            temp=ascii_1608[num][i];		     // Call 16x8 font
		}
        else if(sizey==24){
            temp=ascii_2412[num][i];		     // Call 12x24 font
        }
		else {
            return;
        }
		for(t=0;t<8;t++)
		{
			if(!mode)// Non-overlay mode
			{
				if(temp&(0x01<<t)){
                    TFT_WR_DATA(fc);
                }
				else {
                    TFT_WR_DATA(bc);
                }
				m++;
				if(m%sizex==0)
				{
					m=0;
					break;
				}
			}
			else// Overlay mode
			{
				if(temp&(0x01<<t)){
                    TFT_DrawPoint(x,y,fc);// Draw a point
                }
				x++;
				if((x-x0)==sizex)
				{
					x=x0;
					y++;
					break;
				}
			}
		}
	}
}

/*
*   Function: Display a string
*   Parameters: x,y---start coordinates
*               *p string to display
*               fc font color
*               bc font background color
*               sizey font size
*               mode:  0 non-overlay mode  1 overlay mode
*   Return value: None
*/
void TFT_ShowString(uint16_t x,uint16_t y,const uint8_t *p,uint16_t fc,uint16_t bc,uint8_t sizey,uint8_t mode)
{
	while(*p!='\0')
	{
		TFT_ShowChar(x,y,*p,fc,bc,sizey,mode);
		x+=sizey/2;
		p++;
	}
}

/******************************************************************************
      Function description: Display Chinese characters
      Input data: x,y display coordinates
                *s Chinese characters to display
                fc font color
                bc font background color
                sizey font size optional 16 24 32
                mode:  0 non-overlay mode  1 overlay mode
      Return value: None
******************************************************************************/
void TFT_ShowChinese(uint16_t x,uint16_t y,uint8_t *s,uint16_t fc,uint16_t bc,uint8_t sizey,uint8_t mode)
{
	while(*s!=0)
	{
		if(sizey==12) {
            TFT_ShowChinese12x12(x,y,s,fc,bc,sizey,mode);
        }
		else if(sizey==16) {
            TFT_ShowChinese16x16(x,y,s,fc,bc,sizey,mode);
        }
		else if(sizey==24) {
            TFT_ShowChinese24x24(x,y,s,fc,bc,sizey,mode);
        }
		s+=2;
		x+=sizey;
	}
}

/******************************************************************************
      Function description: Display single 12x12 Chinese character
      Input data: x,y display coordinates
                *s Chinese character to display
                fc font color
                bc font background color
                sizey font size
                mode:  0 non-overlay mode  1 overlay mode
      Return value: None
******************************************************************************/

void TFT_ShowChinese12x12(uint16_t x,uint16_t y,uint8_t *s,uint16_t fc,uint16_t bc,uint8_t sizey,uint8_t mode)
{
	uint8_t i=0,j=0,m=0;
	uint16_t k=0;
	uint16_t HZnum=0;// Number of Chinese characters
	uint16_t TypefaceNum=0;// Number of bytes occupied by one character
	uint16_t x0=x;
	TypefaceNum=(sizey/8+((sizey%8)?1:0))*sizey;

	HZnum=sizeof(tfont12)/sizeof(typFNT_GB12);	// Count the number of Chinese characters
	for(k=0;k<HZnum;k++)
	{
		if((tfont12[k].Index[0]==*(s))&&(tfont12[k].Index[1]==*(s+1)))
		{
			TFT_Address_Set(x,y,x+sizey-1,y+sizey-1);
			for(i=0;i<TypefaceNum;i++)
			{
				for(j=0;j<8;j++)
				{
					if(!mode)// Non-overlay mode
					{
						if(tfont12[k].Msk[i]&(0x01<<j))
						{
							TFT_WR_DATA(fc);
						}
						else
						{
							TFT_WR_DATA(bc);
						}
						m++;
						if(m%sizey==0)
						{
							m=0;
							break;
						}
					}
					else// Overlay mode
					{
						if(tfont12[k].Msk[i]&(0x01<<j))
						{
							TFT_DrawPoint(x,y,fc);// Draw a point
						}
						x++;
						if((x-x0)==sizey)
						{
							x=x0;
							y++;
							break;
						}
					}
				}
			}
		}
		continue;  // Exit after finding the corresponding font library to prevent repeated font retrieval
	}
}
/******************************************************************************
      Function description: Display single 16x16 Chinese character
      Input data: x,y display coordinates
                *s Chinese character to display
                fc font color
                bc font background color
                sizey font size
                mode:  0 non-overlay mode  1 overlay mode
      Return value: None
******************************************************************************/
void TFT_ShowChinese16x16(uint16_t x,uint16_t y,uint8_t *s,uint16_t fc,uint16_t bc,uint8_t sizey,uint8_t mode)
{
	uint8_t i=0,j=0,m=0;
	uint16_t k=0;
	uint16_t HZnum=0;// Number of Chinese characters
	uint16_t TypefaceNum=0;// Number of bytes occupied by one character
	uint16_t x0=x;
  TypefaceNum=(sizey/8+((sizey%8)?1:0))*sizey;
	HZnum=sizeof(tfont16)/sizeof(typFNT_GB16);	// Count the number of Chinese characters
	for(k=0;k<HZnum;k++)
	{
		if ((tfont16[k].Index[0]==*(s))&&(tfont16[k].Index[1]==*(s+1)))
		{
			TFT_Address_Set(x,y,x+sizey-1,y+sizey-1);
			for(i=0;i<TypefaceNum;i++)
			{
				for(j=0;j<8;j++)
				{
					if(!mode)// Non-overlay mode
					{
						if(tfont16[k].Msk[i]&(0x01<<j)){
							TFT_WR_DATA(fc);
						}
						else {
							TFT_WR_DATA(bc);
						}
						m++;
						if(m%sizey==0)
						{
							m=0;
							break;
						}
					}
					else// Overlay mode
					{
						if(tfont16[k].Msk[i]&(0x01<<j))
						{
							TFT_DrawPoint(x,y,fc);// Draw a point
						}
						x++;
						if((x-x0)==sizey)
						{
							x=x0;
							y++;
							break;
						}
					}
				}
			}
		}
		continue;   // Exit after finding the corresponding font library to prevent repeated font retrieval
	}
}

/******************************************************************************
      Function description: Display single 24x24 Chinese character
      Input data: x,y display coordinates
                *s Chinese character to display
                fc font color
                bc font background color
                sizey font size
                mode:  0 non-overlay mode  1 overlay mode
      Return value: None
******************************************************************************/
void TFT_ShowChinese24x24(uint16_t x,uint16_t y,uint8_t *s,uint16_t fc,uint16_t bc,uint8_t sizey,uint8_t mode)
{
	uint8_t i=0,j=0,m=0;
	uint16_t k=0;
	uint16_t HZnum=0;// Number of Chinese characters
	uint16_t TypefaceNum=0;// Number of bytes occupied by one character
	uint16_t x0=x;
	TypefaceNum=(sizey/8+((sizey%8)?1:0))*sizey;
	HZnum=sizeof(tfont24)/sizeof(typFNT_GB24);	// Count the number of Chinese characters
	for(k=0;k<HZnum;k++)
	{
		if ((tfont24[k].Index[0]==*(s))&&(tfont24[k].Index[1]==*(s+1)))
		{
			TFT_Address_Set(x,y,x+sizey-1,y+sizey-1);
			for(i=0;i<TypefaceNum;i++)
			{
				for(j=0;j<8;j++)
				{
					if(!mode)// Non-overlay mode
					{
						if(tfont24[k].Msk[i]&(0x01<<j)){
                            TFT_WR_DATA(fc);
                        }
						else{
                            TFT_WR_DATA(bc);
                        }
						m++;
						if(m%sizey==0)
						{
							m=0;
							break;
						}
					}
					else// Overlay mode
					{
						if(tfont24[k].Msk[i]&(0x01<<j)){
                            TFT_DrawPoint(x,y,fc);// Draw a point
                        }
						x++;
						if((x-x0)==sizey)
						{
							x=x0;
							y++;
							break;
						}
					}
				}
			}
		}
		continue;  // Exit after finding the corresponding font library to prevent repeated font retrieval
	}
}

