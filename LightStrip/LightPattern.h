// TODO: Add ripple effect
enum Themes { NORMAL, HALLOWEEN, CHRISTMAS };
enum Directions { FORWARD, REVERSE };

#define MIN_CHASE_SPEED 60
#define MAX_CHASE_SPEED 30
#define CHASE_INTERVAL_MILIS 500
#define WIPE_INTERVAL_MILIS 50
#define RAINBOW_INTERVAL_MILIS 5
#define FADE_INTERVAL_MILIS 50
#define CLAP_INTERVAL_MILIS 50

class Pattern : public Adafruit_NeoPixel
{
  public:
    // Member Variables:
    String ActivePattern;
    Themes ActiveTheme;
    int ActiveColor;
    int ActiveBrightness;
    Directions Direction;

    bool SpeedChange = false;   // must be overridden
    bool Accelerating = false;  // must be overridden
    bool ColorChange = false;  // must be overridden
    bool ColorCycle = false;
    bool LockPattern = false;
    uint32_t WipeColor;

    // Active Patterns
    bool TheaterChaseEnabled = true;
    bool RainbowCycleEnabled = true;
    bool ColorWipeEnabled = true;
    bool CircleFadeEnabled = true;
    bool ClapEnabled = true;

    // Colors to transition between when using ColorCycle
    uint32_t ColorCollection[6] = {
      Color(10,10,10),       // Black
      Color(255,153,0),   // Orange
      Color(255,0,0),     // Red
      Color(255,255,255), // White
      Color(204,0,204),   // Purple
      Color(0,255,0)      // Green
    };
    uint32_t Color1;
    uint32_t Color1Wheel;
    uint32_t Color2;
    uint32_t Color2Wheel;

    int iPattern = 0;   // Index for the current pattern
    int iColor = 0;     // Index for the current cycle color
    int nPattern = 5;   // TODO
    
    int ColorTimeSeconds = 10; // Seconds before changing the color
    int ColorInterval = 30;     // Amount to change the color at interval
    int PatternInterval = 30; // Seconds before changing the pattern

    String AvailablePatterns[5] = {
      "TheaterChase",
      "RainbowCycle",
      "ColorWipe",
      "CircleFade",
      "Clap"
    };
    
    uint16_t TotalSteps = numPixels();        // total number of steps in the pattern
    uint16_t Index = 0;

    // pattern-specific variables
    uint16_t CircleFadeSize = 6;    // Size of the fade trail
    uint16_t ChaseSectionSize = 3;  // Size of the chase section

    unsigned long Interval;     // milliseconds between updates

    void (*OnComplete)();       // callback

    // Constructor - calls base-class constructor to initialize strip
    Pattern(uint16_t pixels, uint8_t pin, uint8_t type, void (*callback)())
    :Adafruit_NeoPixel(pixels, pin, type){
        OnComplete = callback;
    }

    void Init(Themes theme, int brightness = 255){
      ActivePattern = AvailablePatterns[0]; // Can change on first run
      ActiveBrightness = brightness;
      setBrightness(ActiveBrightness);
      colored_time = millis();
      InitTheme(theme);
      lastUpdate = millis() - Interval - 500;
      Update();
    }

    // Set the properties and colors based on the theme
    void InitTheme(Themes theme){
      ActiveTheme = theme;
      if (ActiveTheme == NORMAL){
        ColorChange = true;
        ColorCycle = false;
        ColorTimeSeconds = 10;
        ColorInterval = 30;
        PatternInterval = 30;

        Color1 = Color(0,255,0);
        Color1Wheel = 170;
        Color2 = Color(0,0,255);
        Color2Wheel = 255;
      }
      
      if (ActiveTheme == HALLOWEEN) {
        ColorChange = false;
        ColorCycle = true;
        ColorTimeSeconds = 10;
        PatternInterval = 30;
        
        ColorCollection[0] = Color(104,0,104);  // Purple
        ColorCollection[1] = Color(153,153,0);  // Orange
        ColorCollection[2] = Color(255,0,0);    // Red
        ColorCollection[3] = Color(255,255,255);// White
        ColorCollection[4] = Color(80,0,80);  // Purple
        ColorCollection[5] = Color(0,255,0);    // Green
        Color1 = ColorCollection[0];
        Color2 = ColorCollection[1];

        // deactivate undesired modes
        RainbowCycleEnabled = false;   
      }
      
      if (ActiveTheme == CHRISTMAS) {
        ColorChange = false;
        ColorCycle = true;
        ColorTimeSeconds = 10;
        PatternInterval = 30;
        
        ColorCollection[0] = Color(255,0,0);    // Red
        ColorCollection[1] = Color(0,255,0);    // Green
        ColorCollection[2] = Color(255,255,255);// White
        ColorCollection[3] = Color(153,153,0);  // Gold
        ColorCollection[4] = Color(255,0,0);    // Red
        ColorCollection[5] = Color(0,255,0);    // Green
        
        Color1 = ColorCollection[0];
        Color2 = ColorCollection[1];

        // deactivate undesired modes
        RainbowCycleEnabled = false;
      }
    }

    // Update the pattern
    void Update(){
      if (!LockPattern) { ChangePattern(); }
      if (ColorChange || ColorCycle) { ChangeColor(); }
      if((millis() - lastUpdate) > Interval){ // time to update
        ChangeSpeed();
        lastUpdate = millis();
        if (ActivePattern == "RainbowCycle") {
          RainbowCycleUpdate();
        } else if (ActivePattern == "ColorWipe") {
          ColorWipeUpdate();
        } else if (ActivePattern == "CircleFade") {
          CircleFadeUpdate();
        } else if (ActivePattern == "Clap") {
          ClapUpdate();
        } else if (ActivePattern == "TheaterChase") {
          TheaterChaseUpdate();
        } else {
          // do nothing
        }
        Increment();
      }
    }

        // Increment the Index and reset at the end
    void Increment(){
      if (Direction == FORWARD){
        Index++;
        if (Index > TotalSteps){
          Index = 0;
          if (OnComplete != NULL){
            OnComplete(); // call the comlpetion callback
          }
        }
      }
      else{ // Direction == REVERSE
        --Index;
        if (Index < 0){
          Index = TotalSteps-1;
          if (OnComplete != NULL){
            OnComplete(); // call the comlpetion callback
          }
        }
      }
    }

    void ChangePattern() {
      this_time = millis();
      if((this_time - changed_time) > (PatternInterval * 1000)) {
        changed_time = millis();
        show();
        ActivePattern = GetNextPattern();
        if (ActivePattern == "RainbowCycle") {
          RainbowCycle(RAINBOW_INTERVAL_MILIS);
        } else if (ActivePattern == "ColorWipe") {
          ColorWipe(Color1, Color2, WIPE_INTERVAL_MILIS);
        } else if (ActivePattern == "CircleFade") {
          CircleFade(Color1, Color2, FADE_INTERVAL_MILIS, 8, true);
        } else if (ActivePattern == "Clap") {
          Clap(Color1, Color2, CLAP_INTERVAL_MILIS, 3);
        } else if (ActivePattern == "TheaterChase") {
          TheaterChase(Color1, Color2, CHASE_INTERVAL_MILIS, 3);
        } else {
          // do nothing
        }
      }
    }

    String GetNextPattern() {
      int arrSize = sizeof(AvailablePatterns)/sizeof(AvailablePatterns[0]);
      iPattern++;
      if (iPattern == arrSize) {
        iPattern = 0;
      }
      if (IsEnabledPattern(AvailablePatterns[iPattern])) {
        return AvailablePatterns[iPattern];
      } else {
        return GetNextPattern();
      }
      int len = sizeof(AvailablePatterns);
    }

    bool IsEnabledPattern(String pattern) {
      if (pattern == "RainbowCycle") {
        return RainbowCycleEnabled;
      } else if (pattern == "ColorWipe") {
        return ColorWipeEnabled;
      } else if (pattern == "CircleFade") {
        return CircleFadeEnabled;
      } else if (pattern == "Clap") {
        return ClapEnabled;
      } else if (pattern == "TheaterChase") {
        return TheaterChaseEnabled;
      } else {
        return true;
      }
    }

    void ChangeColor() {
      this_time = millis();
      if((this_time - colored_time) > (ColorTimeSeconds * 1000)) {
        colored_time = millis();
        if (ColorChange && ActivePattern != "RainbowCycle") {
          SetNextColorFromWheel();
        }
        if (ColorCycle) {
          Color1 = Color2;
          Color2 = GetNextColorFromCollection();
        }
      }
    }

    uint32_t GetNextColorFromCollection() {
      int arrSize = sizeof(ColorCollection)/sizeof(ColorCollection[0]);
      iColor++;
      if (iColor == arrSize) {
        iColor = 0;
      }
      return ColorCollection[iColor];
    }

    void SetNextColorFromWheel() {
      Color1Wheel += ColorInterval;
      if (Color1Wheel > 255) { Color1Wheel -= 255; }
      Color2Wheel += ColorInterval;
      if (Color2Wheel > 255) { Color2Wheel -= 255; }

      Color1 = Wheel(Color1Wheel);
      Color2 = Wheel(Color2Wheel);
    }

    // Initialize for a ColorWipe
    void ColorWipe(uint32_t color1, uint32_t color2, uint8_t interval, Directions dir = FORWARD){
      Serial.println("Begin ColorWipe");
      ActivePattern = "ColorWipe";
      Interval = interval;
      TotalSteps = numPixels();
      Color1 = color1;
      Color2 = color2;
      Index = 0;
      Direction = dir;
      WipeColor = Color1;
    }

    // Update the Color Wipe Pattern
    void ColorWipeUpdate(){
      setPixelColor(Index, WipeColor);
      show();
      if (Index + 1 == TotalSteps){
        if (WipeColor == Color1){
          WipeColor = Color2;
        }
        else{
          WipeColor = Color1;
        }
      }
    }

    void RainbowCycle(uint8_t interval, Directions dir = FORWARD){
      Serial.println("Begin RainbowCycle");
      ActivePattern = "RainbowCycle";
      Interval = interval;
      TotalSteps = 255;
      Index = 0;
      Direction = dir;
    }

    // Update the Rainbow Cycle Pattern
    void RainbowCycleUpdate(){
      for(int i=0; i< numPixels(); i++){
        setPixelColor(i, Wheel(((i * 256 / numPixels()) + Index) & 255));
      }
      show();
    }

    // Initialize for a Theater Chase
    void TheaterChase(uint32_t color1, uint32_t color2, uint8_t interval, uint16_t count, Directions dir = FORWARD){
      Serial.println("Begin TheaterChase");
      ActivePattern = "TheaterChase";
      Interval = interval;
      ChaseSectionSize = count;   // ChaseSectionSize here will be the length of Color1
      TotalSteps = numPixels();
      Color1 = color1;
      Color2 = color2;
      Index = 0;
      Direction = dir;
    }

    // Update the Theater Chase Pattern
    void TheaterChaseUpdate(){
      for(int i=0; i< numPixels(); i++){
        if ((i + Index) % ChaseSectionSize == 0){
          setPixelColor(i, Color2);
        }
        else{
          setPixelColor(i, Color1);
        }
      }
      show();
    }

    // Initialize for a Circle Fade
    void CircleFade(uint32_t color1, uint32_t color2, uint16_t interval, uint16_t fadeLength, bool doubletone = false, Directions dir = FORWARD){
      Serial.println("Begin CircleFade");
      ActivePattern = "CircleFade";
      CircleFadeSize = fadeLength;
      Interval = interval;
      circleFadeDouble = doubletone;
      TotalSteps = numPixels() + 1;
      Color1 = color1;
      Color2 = color2;
      Index = 0;
      Direction = dir;
    }

    // Update the Theater Chase Pattern
    void CircleFadeUpdate(){
      CircleFadeSet(Index, Color1);
      if (circleFadeDouble){
        int start = (Index + (TotalSteps / 2)) % TotalSteps;
        CircleFadeSet(start, Color2);
      }
      show();
    }

    void CircleFadeSet(int start, uint32_t color){
      for (int i=0; i < CircleFadeSize; i++){
        int point = start - i;
        if (point < 0) { point = TotalSteps + point; }
        //Serial.println(point);
        int brightness = 255 * ((float)i/((float)CircleFadeSize * 2));
        //Serial.println(percent);
        int colorDimmed = DimColorPercent(color, brightness);
        setPixelColor(point, colorDimmed);
      }
      int point = start - CircleFadeSize;
      if (point < 0) { point = TotalSteps + point; }
      setPixelColor(point, 0); 
    }

        // Initialize for a Circle Fade
    void Clap(uint32_t color1, uint32_t color2, uint16_t interval, uint16_t len){
      Serial.println("Begin Clap");
      ActivePattern = "Clap";
      Interval = interval;
      ChaseSectionSize = len;
      TotalSteps = numPixels();
      Color1 = color1;
      Color2 = color2;
      Index = 0;
    }

    // Update the Theater Chase Pattern
    void ClapUpdate(){
      show();
      if (Index >= (TotalSteps / 2)){
        //Serial.println("REVERSE");
        Reverse();
      }

      // Set the ON lights
      for (int i=0; i < ChaseSectionSize; i++){
        int point = Index - i;
        if (Direction == REVERSE){
          point = Index + i;
        }
        ClapSet(point, Color1, Color2);
      }

      // Set the OFF lights
      int point = Index - ChaseSectionSize;
      if (Direction == REVERSE){
        point = Index + ChaseSectionSize;
      }
      ClapSet(point, 0, 0);
      show();
    }

    void ClapSet(int point, int32_t colorOne, int32_t colorTwo){
      if (point >= 0){
        if (point <= TotalSteps / 2){
          setPixelColor(point, colorOne);
          int oppositePoint = TotalSteps - point;
          setPixelColor(oppositePoint, colorTwo);
        }
        else{
          if (colorOne == 0 && colorTwo == 0){
            setPixelColor(point, colorOne);
            int oppositePoint = TotalSteps - point;
            setPixelColor(oppositePoint, colorTwo);
          }
        }
      }
    }
    
  private:
    bool circleFadeDouble = false;
    
    unsigned long lastUpdate;   // last update of position
    unsigned long this_time = millis();
    unsigned long changed_time = this_time - (PatternInterval * 1000);  // Set to init right away
    unsigned long colored_time = this_time;
    
    // Common Utility Functions
    // Returns the Red component of a 32-bit color
    uint8_t Red(int color){
      return (color >> 16) & 0xFF;
    }

    // Returns the Green component of a 32-bit color
    uint8_t Green(int color){
      return (color >> 8) & 0xFF;
    }

    // Returns the Blue component of a 32-bit color
    uint8_t Blue(int color){
      return color & 0xFF;
    }

    // Return color, dimmed by 75% (used by scanner)
    int DimColor(int color){
      int dimColor = Color(Red(color) >> 1, Green(color) >> 1, Blue(color) >> 1);
      return dimColor;
    }

    int DimColorPercent(int color, double brightness){
      if (brightness == 255) { return color; }
      int redPart = FlipColor(brightness*Red(color)/255);
      int greenPart = FlipColor(brightness*Green(color)/255);
      int bluePart = FlipColor(brightness*Blue(color)/255);
//      int redPart = FlipColor((int)(Red(color)*percent));
//      int greenPart = FlipColor((int)(Green(color)*percent));
//      int bluePart = FlipColor((int)(Blue(color)*percent));
      int dimColor = Color(redPart, greenPart, bluePart);
      return dimColor;
    }

    int FlipColor(int color){
      if (color != 0){
        return 255 - color;
      }
      return color;
    }

    // Input a value 0 to 255 to get a color value.
    // The colors are a transition r - g - b - back to r.
    int Wheel(byte WheelPos){
      WheelPos = 255 - WheelPos;
      if(WheelPos < 85){
        return Color(255 - WheelPos * 3, 0, WheelPos * 3);
      }
      else if(WheelPos < 170){
        WheelPos -= 85;
        return Color(0, WheelPos * 3, 255 - WheelPos * 3);
      }
      else{
        WheelPos -= 170;
        return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
      }
    }

    // Reverse direction of the pattern
    void Reverse(){
      if (Direction == FORWARD){
        Direction = REVERSE;
      }
      else{
        Direction = FORWARD;
      }
    }

    void ChangeSpeed() {
      if (SpeedChange) {
        if (Accelerating) {
          Interval -= CHASE_INTERVAL_MILIS;
          if (Interval < MAX_CHASE_SPEED) {
            Accelerating = false;
            Interval += CHASE_INTERVAL_MILIS;
          }
        }
        else {
          Interval += CHASE_INTERVAL_MILIS;
          if (Interval > MIN_CHASE_SPEED) {
            Accelerating = true;
            Interval -= CHASE_INTERVAL_MILIS;
          }
        }
      }
    }
};
