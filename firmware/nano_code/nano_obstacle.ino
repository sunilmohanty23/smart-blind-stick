/*******************************************************
 * Project: Smart Blind Stick - Obstacle & Pit Detection
 * Controller: Arduino Nano
 *
 * Description:
 * This system uses two ultrasonic sensors:
 * 1. Front sensor  -> detects obstacles
 * 2. Down sensor   -> detects ground changes (stairs/pit)
 *
 * A buzzer provides 5 different sound patterns based on:
 * - Obstacle distance
 * - Ground proximity
 * - Pit detection
 *
 * Priority Order:
 * 1. Pit Detection (highest)
 * 2. Both sensors close
 * 3. Lower sensor warning
 * 4. Front strong warning
 * 5. Front normal warning
 *******************************************************/

#define FRONT_TRIG 2
#define FRONT_ECHO 3

#define DOWN_TRIG 4
#define DOWN_ECHO 5

#define BUZZER 6

long duration;
float frontDist, downDist;
float prevDownDist = 0;

/*******************************************************
 * Function: getDistance()
 * Purpose : Measure distance using ultrasonic sensor
 * Input   : Trigger pin, Echo pin
 * Output  : Distance in cm
 * Notes   : Returns -1 if no signal detected
 *******************************************************/
float getDistance(int trigPin, int echoPin) {

  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH, 30000); // 30ms timeout

  if (duration == 0) return -1;  // invalid reading

  return duration * 0.0343 / 2;  // convert to cm
}

/*******************************************************
 * SOUND PATTERNS
 * Each function defines a unique buzzer pattern
 *******************************************************/

// Sound 1: Front obstacle (20–40 cm) → slow beep
void sound1() {
  digitalWrite(BUZZER, HIGH);
  delay(300);
  digitalWrite(BUZZER, LOW);
  delay(300);
}

// Sound 2: Front strong danger (<20 cm) → fast beep
void sound2() {
  digitalWrite(BUZZER, HIGH);
  delay(100);
  digitalWrite(BUZZER, LOW);
  delay(100);
}

// Sound 3: Lower sensor (<20 cm) → double pulse
void sound3() {
  digitalWrite(BUZZER, HIGH);
  delay(80);
  digitalWrite(BUZZER, LOW);
  delay(80);

  digitalWrite(BUZZER, HIGH);
  delay(80);
  digitalWrite(BUZZER, LOW);
  delay(200);
}

// Sound 4: Pit detection → continuous alert (highest priority)
void sound4() {
  digitalWrite(BUZZER, HIGH);
}

// Sound 5: Both sensors <20 cm → custom pattern
void sound5() {
  digitalWrite(BUZZER, LOW);
  delay(150);

  digitalWrite(BUZZER, HIGH);
  delay(200);

  digitalWrite(BUZZER, LOW);
  delay(150);

  digitalWrite(BUZZER, HIGH);
  delay(200);

  digitalWrite(BUZZER, LOW);
}

/*******************************************************
 * Setup Function
 *******************************************************/
void setup() {

  Serial.begin(9600);

  pinMode(FRONT_TRIG, OUTPUT);
  pinMode(FRONT_ECHO, INPUT);

  pinMode(DOWN_TRIG, OUTPUT);
  pinMode(DOWN_ECHO, INPUT);

  pinMode(BUZZER, OUTPUT);
}

/*******************************************************
 * Main Loop
 * Reads sensor data and applies priority-based logic
 *******************************************************/
void loop() {

  // Read distances from both sensors
  frontDist = getDistance(FRONT_TRIG, FRONT_ECHO);
  downDist  = getDistance(DOWN_TRIG, DOWN_ECHO);

  // Debug output for calibration and monitoring
  Serial.print("Front: ");
  Serial.print(frontDist);
  Serial.print(" cm | Down: ");
  Serial.println(downDist);

  /***************************************************
   * PRIORITY-BASED DECISION MAKING
   ***************************************************/

  // 1. PIT DETECTION (Highest Priority)
  // Detect sudden increase in ground distance
  if (downDist > 0 && prevDownDist > 0 &&
      downDist > 60 && (downDist - prevDownDist > 40)) {

    Serial.println("PIT DETECTED");
    sound4();
  }

  // 2. BOTH SENSORS CLOSE (<20 cm)
  else if (frontDist > 0 && downDist > 0 &&
           frontDist < 20 && downDist < 20) {

    Serial.println("BOTH CLOSE");
    sound5();
  }

  // 3. LOWER SENSOR WARNING (<20 cm)
  else if (downDist > 0 && downDist < 20) {

    Serial.println("LOWER WARNING");
    sound3();
  }

  // 4. FRONT STRONG WARNING (<50 cm)
  else if (frontDist > 0 && frontDist < 50) {

    Serial.println("FRONT STRONG");
    sound2();
  }

  // 5. FRONT NORMAL WARNING (51–100 cm)
  else if (frontDist >= 51 && frontDist <= 100) {

    Serial.println("FRONT WARNING");
    sound1();
  }

  // SAFE CONDITION
  else {
    digitalWrite(BUZZER, LOW);
  }

  // Update previous ground distance (only valid values)
  if (downDist > 0) {
    prevDownDist = downDist;
  }

  delay(50); // small delay for stability
}