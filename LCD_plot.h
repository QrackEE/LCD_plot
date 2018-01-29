#include <TFTLCD-mcufriend.h> // Hardware-specific library
#include "Arduino.h"



struct Point {
	uint16_t x;
	uint16_t y;
};

enum scalemask { SCALE_NONE = 0, SCALE_X = 1, SCALE_Y = 2, SCALE_XY = 3, SCALE_KEEP = 4 };

class plot_normal
{
public:
	plot_normal(Adafruit_TFTLCD* tft, uint16_t figX, uint16_t figY, uint16_t figWidth, uint16_t figHeight);
	void printxLabel(String label);
	void printyLabel(String label);
	void drawGrid(uint8_t x, uint8_t y, scalemask scaled);

	void plot(int *data, uint16_t from, uint16_t to, uint16_t colour);
	void clear();


	void setScale(Scale scale);
	void setScale(uint8_t x, uint8_t y);
	Scale autoscale(int *data, uint16_t from, uint16_t to, scalemask mask);

	Scale getScale();
	Point getFigOrigin();
	Point getPlotOrigin();
private:

protected:

	Adafruit_TFTLCD* lcd;


	uint16_t figxSize, figySize;
	Point figOrigin, plotOrigin;
	uint16_t xSize, ySize;

	Scale grid;
	Scale plotScale;

	bool gridOn;
};

class plot_sweep : public plot_normal
{
public:
	plot_sweep(Adafruit_TFTLCD* tft, uint16_t figX, uint16_t figY, uint16_t figWidth, uint16_t figHeight);
	void plot(int *data, uint16_t from, uint16_t to, uint16_t colour);
	void plot(int data, uint16_t colour);

	void clear();
private:
	uint16_t lastx;
	
protected:
	

};

class plot_running : public plot_normal
{
public:
	plot_running(Adafruit_TFTLCD* tft, uint16_t figX, uint16_t figY, uint16_t figWidth, uint16_t figHeight);
	~plot_running();

	void plot(int *data, uint16_t from, uint16_t to, uint16_t colour);
	void plot(int data, uint16_t colour);

	void clear();
private:
	int *buffer;
	uint16_t buffIndex;
protected:

};