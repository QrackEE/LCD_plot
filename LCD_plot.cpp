#include "LCD_plot.h"

plot_normal::plot_normal(Adafruit_TFTLCD* tft, uint16_t figX, uint16_t figY, uint16_t figWidth, uint16_t figHeight)
{
	figOrigin.x = figX;
	figOrigin.y = figY;
	figxSize = figWidth;
	figySize = figHeight;
	plotOrigin.x = figOrigin.x+10;
	plotOrigin.y = figOrigin.y+figHeight-10;
	xSize = figWidth - 19;
	ySize = figHeight - 18;
	
	grid.x = grid.y = 10;
	gridOn = false;
	plotScale.x = plotScale.y = 1;

	lcd = tft;

	// draw figure
	lcd->setRotation(1);
	lcd->fillRect(figOrigin.x,figOrigin.y,figWidth,figHeight, WHITE);
	uint16_t yOriginPlot = plotOrigin.y+figHeight-10;

	clear();
	// draw arrows
	lcd->drawFastHLine(plotOrigin.x-1, plotOrigin.y-figHeight+16, 3, BLACK);
	lcd->drawFastHLine(plotOrigin.x-2, plotOrigin.y-figHeight+17, 5, BLACK);
	lcd->drawFastVLine(plotOrigin.x+figWidth-17, plotOrigin.y-1, 3, BLACK);
	lcd->drawFastVLine(plotOrigin.x+figWidth-18, plotOrigin.y-2, 5, BLACK);
	
}

void plot_normal::printxLabel(String label)
{
	lcd->setTextSize(1);
	lcd->setCursor(figOrigin.x+20, figOrigin.y+figySize-8);
	lcd->print(label);
}

void plot_normal::printyLabel(String label)
{
	lcd->setTextSize(1);
	lcd->setRotation(0);
	uint16_t xOriginRotated = lcd->width()-figOrigin.y-figySize;
	uint16_t yOriginRotated = figOrigin.x;
	lcd->setCursor(xOriginRotated+10,yOriginRotated+1);
	lcd->print(label);
	lcd->setRotation(1);
}

void plot_normal::drawGrid(uint8_t x, uint8_t y, scalemask scaled)
{
	if(x > 0)
		grid.x = x;
	if(y > 0)
		grid.y = y;
	
	if(scaled != SCALE_KEEP)
	{
		if(scaled & SCALE_X)
			grid.x *= plotScale.x;
		if(scaled & SCALE_Y)
			grid.y /= plotScale.y;
	}

	uint16_t temp;
	for(int i = 1; temp = (i*grid.y), temp < ySize ;i++)
	{
		lcd->drawFastHLine(plotOrigin.x+1, plotOrigin.y - temp, xSize, 0xA504);
		lcd->drawPixel(plotOrigin.x-1, plotOrigin.y - temp, BLACK);
	}
	for(int i = 1; temp = i*grid.x, temp < xSize ;i++)
	{
		lcd->drawFastVLine(plotOrigin.x+temp, plotOrigin.y-ySize, ySize, 0xA504);
		lcd->drawPixel(plotOrigin.x+temp, plotOrigin.y +1, BLACK);
	}

	gridOn = true;
}

void plot_normal::clear()
{
	lcd->setRotation(1);
	lcd->fillRect(plotOrigin.x+1,plotOrigin.y-ySize,xSize,ySize, YELLOW);
	lcd->drawFastVLine(plotOrigin.x, plotOrigin.y-figySize+15, figySize-15, BLACK);
	lcd->drawFastHLine(plotOrigin.x, plotOrigin.y, figxSize-15, BLACK);
	// clear grid marks
	lcd->drawFastHLine(plotOrigin.x, plotOrigin.y+1, xSize, WHITE);
	lcd->drawFastVLine(plotOrigin.x-1, plotOrigin.y-ySize, ySize, WHITE);
}

void plot_normal::plot(int *data, uint16_t from, uint16_t to, uint16_t colour)
{
/*
	bool gridWasOn = gridOn;
	clear();
	if(gridWasOn)
		drawGrid(0,0,SCALE_KEEP);
*/

	uint16_t dataSize = to-from+1;
	
	uint16_t i = 0;
	if(dataSize*plotScale.x > xSize)
	{
		i = to - xSize/plotScale.x;
	}



	int dataCurr, dataPrev;
	bool disp = true;
	bool slopeDisp = true;
	uint16_t tempX = 0;
	for(i=0; i<dataSize; i++)
	{
		tempX = plotOrigin.x+i*plotScale.x;
		dataCurr = data[i+from]/plotScale.y;
		if(dataCurr < 0)
		{
			dataCurr = 0;
			disp = false;
		}
		else if(dataCurr > ySize)
		{
			dataCurr = ySize;
			disp = false;
		}
		else
		{
			slopeDisp = true;
			disp = true;
		}
			

		dataCurr = plotOrigin.y-dataCurr;
		if(i>0)
		{
			lcd->drawFastHLine(tempX-plotScale.x,plotOrigin.y,plotScale.x,RED);
			if(slopeDisp)
				lcd->drawLine(tempX-plotScale.x, dataPrev, tempX, dataCurr, colour  );
		}

		if(disp)
			lcd->drawPixel(tempX, dataCurr, colour+0xF800);
		else slopeDisp = false;

		
		dataPrev = dataCurr;
	}
}

Scale plot_normal::autoscale(int *data, uint16_t from, uint16_t to, scalemask scale)
{
	
	if(scale & SCALE_X)
	{
		plotScale.x = xSize / (to-from);
	}
	if(scale & SCALE_Y)
	{
		// sweep through all data to be plotted and find optimum scaling factor

		for(uint16_t i = from; i<to; i++)
		{
			if(data[i]/plotScale.y > ySize)
				plotScale.y = data[i]/ySize + 1;
		}
	}

}

void plot_normal::setScale(Scale scale)
{
	setScale(scale.x, scale.y);
}

void plot_normal::setScale(uint8_t x, uint8_t y)
{
	if(x > 0)
		plotScale.x = x;
	if(y>0);
		plotScale.y = y;
}

Scale plot_normal::getScale()
{
	return plotScale;
}

Point plot_normal::getFigOrigin()
{
	return figOrigin;
}

Point plot_normal::getPlotOrigin()
{
	return plotOrigin;
}


// ============================ SWEEP ============================
plot_sweep::plot_sweep(Adafruit_TFTLCD* tft, uint16_t figX, uint16_t figY, uint16_t figWidth, uint16_t figHeight) : plot_normal(tft, figX, figY, figWidth, figHeight)
{
	lastx = 0;
}

void plot_sweep::plot(int *data, uint16_t from, uint16_t to, uint16_t colour)
{
	// plot all data from input array simultaneously
	if(to-from > 7)
		to = from+7;

	if(lastx+plotScale.x > xSize)
	{
		lastx = 0;
		bool gridWasOn = gridOn;
		clear();
		if(gridWasOn)
		drawGrid(0,0, SCALE_KEEP);
	}

	static int dataPrev[8] = {plotOrigin.y};
	uint8_t disp = 0xFF;
	uint8_t slopeDisp = 0xFF;

	for(int i = 0; i<=to-from; i++)
	{
		colour += 0xAB45;
		int ldata = data[i]/plotScale.y;
		if(ldata < 0)
		{
			ldata = 0;
			disp &= ~(1 << i);
		}
		else if(ldata > ySize)
		{
			ldata = ySize;
			disp &= ~(1 << i);
		}
		else
		{
			slopeDisp |= (1 << i);
			disp |= (1 << i);
		}

		ldata = plotOrigin.y-ldata;

		if(lastx>0 && (slopeDisp & (1 << i)))
			lcd->drawLine(plotOrigin.x+lastx-plotScale.x, dataPrev[i], plotOrigin.x+lastx, ldata, colour );

		if(disp & (1 << i))
			lcd->drawPixel(plotOrigin.x+lastx, ldata, colour+0xF800 );
		else slopeDisp &= ~(1 << i);


		lcd->drawFastHLine(plotOrigin.x+lastx,plotOrigin.y,plotScale.x,RED);
		
		// Vertical red line
		//	lcd->drawFastVLine(plotOrigin.x+lastx, plotOrigin.y-ySize, ySize, RED);
	
		dataPrev[i] = ldata;
	}
	lastx+= plotScale.x;

}

void plot_sweep::plot(int data, uint16_t colour)
{
	if(lastx+plotScale.x > xSize)
	{
		lcd->drawFastVLine(plotOrigin.x+lastx-plotScale.x+1, plotOrigin.y-ySize, ySize, MAGENTA);
		lastx = 0;
		lcd->drawFastHLine(plotOrigin.x, plotOrigin.y, figxSize-15, BLACK);
	}
	static int dataPrev = plotOrigin.y;
	bool disp = true;
	bool slopeDisp = true;
	data = data/plotScale.y;
	if(data < 0)
	{
		data = 0;
		disp = false;
	}
	else if(data > ySize)
	{
		data = ySize;
		disp = false;
	}
	else
	{
		slopeDisp = true;
		disp = true;
	}
	
	data = plotOrigin.y-data;
	// clear front
	if(lastx>0)
	{
		lcd->fillRect(plotOrigin.x+lastx-plotScale.x+1, plotOrigin.y-ySize, plotScale.x, ySize, YELLOW);
	// redraw Grid
		if(gridOn)
		{
			// Vertical
			for(int i = 0; i<plotScale.x; i++)
			{
				if( (lastx+i)%grid.x == 0 )
					lcd->drawFastVLine(plotOrigin.x+lastx+i, plotOrigin.y-ySize, ySize, 0xA504);
			}
			// Horizontal
	
			for(int i=0; i<ySize/grid.y; i++)
				lcd->drawFastHLine(plotOrigin.x+lastx-plotScale.x+1, plotOrigin.y - (i+1)*grid.y, plotScale.x, 0xA504);
		}
		// print new
		if(slopeDisp)
		{
			lcd->drawLine(plotOrigin.x+lastx-plotScale.x, dataPrev, plotOrigin.x+lastx, data, colour  );
			// Horizontal bar on x axe
			lcd->drawFastHLine(plotOrigin.x+lastx-plotScale.x,plotOrigin.y,plotScale.x,RED);
		}
	}

	if(disp)
		lcd->drawPixel(plotOrigin.x+lastx, data, colour+0xF800);
	else slopeDisp = false;
	
	
	// Vertical red line
	lcd->drawFastVLine(plotOrigin.x+lastx+1, plotOrigin.y-ySize, ySize, RED);
	lastx+= plotScale.x;
	
		
	dataPrev = data;
}

void plot_sweep::clear()
{
	plot_normal::clear();
	lastx = 0;
}

// ============================ RUNNING ============================
plot_running::plot_running(Adafruit_TFTLCD* tft, uint16_t figX, uint16_t figY, uint16_t figWidth, uint16_t figHeight) : plot_normal(tft, figX, figY, figWidth, figHeight)
{
	#ifdef _DEBUG
		Serial.println(xSize);
	#endif

	buffer = new int[xSize];

	if(buffer==nullptr)
	{
		#ifdef _DEBUG
			Serial.println(F("Not enough memory for running plot!"));
		#endif
		tft->fillRect(figX, figY, figWidth,figHeight, WHITE);
		tft->drawLine(figX, figY, figX+figWidth-1, figY+figHeight-1, RED);
		tft->drawLine(figX, figY+figHeight-1, figX+figWidth-1, figY, RED);
		tft->setTextColor(RED);
		tft->setTextSize(2);
		tft->setCursor(figX+figWidth/2-40, figY+figHeight/2-8);
		tft->print(F("LOW RAM"));
		while(1);
		}
	buffIndex = 0;
}

plot_running::~plot_running()
{
	delete []buffer;
}

void plot_running::plot(int *data, uint16_t from, uint16_t to, uint16_t colour)
{
	// plot all data from input array simultaneously
}

void plot_running::plot(int data, uint16_t colour)
{
	#ifdef _DEBUG
		Serial.print(buffIndex); Serial.print("/"); Serial.println(xSize);
	#endif
	if(buffIndex >= (xSize-1)/plotScale.x)
	{
		// move all data. We could use circular buffer, but well... it's more space consuming
		for(uint16_t i = 0; i<buffIndex; i++)
		{
			if(i+1<buffIndex)
			{
				int yFrom, yTo;
				if(buffer[i] < 0)
					yFrom = 0;
				else if(buffer[i]/plotScale.y > ySize)
					yFrom = ySize;
				else
					yFrom = buffer[i]/plotScale.y;
				yFrom = plotOrigin.y-yFrom;

				if(buffer[i+1] < 0)
					yTo = 0;
				else if(buffer[i+1]/plotScale.y > ySize)
					yTo = ySize;
				else
					yTo = buffer[i+1]/plotScale.y;
				yTo = plotOrigin.y-yTo;

				lcd->drawLine(plotOrigin.x+i*plotScale.x, yFrom, plotOrigin.x+(i+1)*plotScale.x, yTo, YELLOW);

			}
		
			buffer[i] = buffer[i+1];

		}
		lcd->drawFastVLine(plotOrigin.x, plotOrigin.y-figySize+15, figySize-15, BLACK);
		if(gridOn)
			drawGrid(0,0, SCALE_KEEP);
		buffIndex = (xSize-1)/plotScale.x;
	}
	buffer[buffIndex] = data;

	plot_normal::plot(buffer,0,buffIndex++,colour);
}

void plot_running::clear()
{
	plot_normal::clear();
	buffIndex = 0;
}