#include <iostream>
#include <windows.h>

/////////////////////////////////////////////////////////

float SimpleRandom()
{
  return float(rand() % RAND_MAX) / RAND_MAX;
}

float MyFunc(float x)
{
  return fabs(cos(3.0F*x)*sin(5.0F*x));
}

float mutateRnd(const float rnd, const float step)
{
  float sign   = (rand() % 2 + 1) % 2 ? 1.0F : -1.0F;
  float offset = rnd + sign*step;

  if      (offset > 1.0F) offset = rnd - sign * step;
  else if (offset < 0.0F) offset = rnd + sign * step;

  return offset;
}

float ScaleVal(const float val, const float min, const float max)
{
  return min + val * (max - min); 
}

float Clamp(const float val, const float min, const float max)
{
  return fmax(fmin(val, max), min);
}


void DrawLine(HWND hWnd, HDC hDC, const int startX, const int startY, const int endX, const int endY, const COLORREF color)
{
  SelectObject(hDC, GetStockObject(DC_PEN));
  SetDCPenColor(hDC, color);

  MoveToEx(hDC, startX, startY, nullptr);
  LineTo(hDC, endX, endY);
}

/////////////////////////////////////////////////////////

int main()
{
  constexpr int sizeGraph  = 300;

  //Init console

  HWND hWnd                = GetConsoleWindow();
  HDC hDC                  = GetDC(hWnd);
  COLORREF funcColor        = RGB(250, 250, 250);
  COLORREF refColor         = RGB(100, 255, 0);

  const int screenWidth    = GetSystemMetrics(SM_CXSCREEN);
  const int screenHeight   = GetSystemMetrics(SM_CYSCREEN);
  const int windowWidth    = sizeGraph * 4;
  const int windowHeight   = sizeGraph * 3;
  const int posXwindow     = (screenWidth - windowWidth) / 2;
  const int posYwindow     = (screenHeight - windowHeight) / 2;

  SetWindowPos(hWnd, HWND_TOP, posXwindow, posYwindow, windowWidth, windowHeight, NULL);
  
  /////////////////////////////////////
  // uniform sampling
  /////////////////////////////////////
  
  srand(time(NULL));

  constexpr float reference = 0.431035F;
  float summUniform         = 0.0F;
  float summImportance      = 0.0F;
  constexpr int maxSamples  = 1000;
  int summLocalSamples      = 0;

  int draw1startY = 170 + sizeGraph;
  int draw2startY = 250 + sizeGraph * 2;


  for (size_t i = 0; i < maxSamples; ++i)
  {
    const float rnd            = SimpleRandom();
    const float funcResUniform = MyFunc(rnd);

    const float alpha          = 1.0F / float(i + 1);
    summUniform                = summUniform * (1.0f - alpha) + funcResUniform * alpha;

    // Draw function
    int draw1startX = 20 + rnd * (float)sizeGraph;
    int draw1endX   = draw1startX;
    int draw1endY   = draw1startY - funcResUniform * (float)sizeGraph;
    DrawLine(hWnd, hDC, draw1startX, draw1startY, draw1endX, draw1endY, funcColor);
    
    //Draw mean
    draw1startX     = 20 + sizeGraph + ((float)i / (float)maxSamples) * (float)sizeGraph * 3.5F;
    draw1endX       = draw1startX;    
    draw1endY       = draw1startY - summUniform * sizeGraph;
    DrawLine(hWnd, hDC, draw1startX, draw1startY, draw1endX, draw1endY, funcColor);

    
    /////////////////////////////////////
    // local sampling
    /////////////////////////////////////

    const float width   = 0.1F; // [0 - 1]
    const float min     = int(rnd / width) * width;
    const float max     = fmin(min + width, 1.0F);
    int localSamples    = 0;
    int maxLocalSamples = 1;
    float localSumm     = 0.0F;

    for (size_t j = 0; j < maxLocalSamples; ++j)
    {
      const float scaleRnd  = ScaleVal(SimpleRandom(), min, max);

      const float funcLum   = MyFunc(scaleRnd);
      const float clampFunc = Clamp(funcLum, 0.0F, 1.0F);
      maxLocalSamples       = fmax(10 * clampFunc, 1);

      localSumm            += funcLum;
      localSamples++;

      // Draw function
      int draw2startX = 20 + scaleRnd * (float)sizeGraph;
      int draw2endX   = draw2startX;
      int draw2endY   = draw2startY - funcLum * (float)sizeGraph;
      DrawLine(hWnd, hDC, draw2startX, draw2startY, draw2endX, draw2endY, funcColor);
    }    

    localSumm        /= (float)localSamples;
    summImportance    = summImportance * (1.0f - alpha) + localSumm * alpha;
    summLocalSamples += localSamples;
    
    //Draw mean
    int draw2startX   = 20 + sizeGraph + ((float)i / (float)maxSamples) * (float)sizeGraph * 3.5F;
    int draw2endX     = draw2startX;
    int draw2endY     = draw2startY - summImportance * sizeGraph;
    DrawLine(hWnd, hDC, draw2startX, draw2startY, draw2endX, draw2endY, funcColor);
  }

  //Draw reference    
  const int draw1RefposY = draw1startY - reference * sizeGraph;
  const int draw2RefposY = draw2startY - reference * sizeGraph;
  DrawLine(hWnd, hDC, 20, draw1RefposY, 20 + sizeGraph * 4.5, draw1RefposY, refColor);
  DrawLine(hWnd, hDC, 20, draw2RefposY, 20 + sizeGraph * 4.5, draw2RefposY, refColor);

  ReleaseDC(hWnd, hDC);

  std::cout << "reference      = " << reference      << " (1M samples)" << std::endl;
  std::cout << "summUniform    = " << summUniform    << " (" << maxSamples       << " samples)" << std::endl;
  std::cout << "summImportance = " << summImportance << " (" << summLocalSamples << " samples)" << std::endl;

  return 0;
}

