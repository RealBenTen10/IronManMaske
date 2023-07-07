#include <Servo.h>
#include <LiquidCrystal.h>

// "Header" Datei
#include "pitches.h"

void setup() {

  // LCD-Initialisierung II
  lcd.begin(16, 2);
  // LCD-Screen gibt Hello World aus, bis geändert
  lcd.print("Hello, World!");
  // Pins zuordnen
  pinMode(led_pin_rand, OUTPUT);
  pinMode(led_pin_auge_rot, OUTPUT);
  pinMode(led_pin_auge_blau, OUTPUT);
  myservo.attach(pin_servo);
  // Mikrofon
  Serial.begin(9600);
  pinMode(sensor, INPUT);

  // Konfigurierung einbauen? z.B. vor loop() in setup() noch den threshhold des Mikros bestimmen
  // (2 Sekunden lang alle 50 ms nen wert aufnehmen und median nehmen)
  hello();
}

void loop() {

  // wir lesen durchgehend die aktuelle Lautstärke aus
  soundValue = analogRead(A0);
  servo();
  delay(1000);
  // falls der threshhold überschritten wird, startet unser Signal
  if (soundValue <= threshhold) {
    lichter_aus();
    delay(pause);
    signal_erfassen();
  }
  delay(10);
}

// Begrüßerung plus eventuell Konfigurierung
// Augen leuchten Rot, Maske fährt hoch, Rand leuchtet, Maske fährt runter, Rand geht aus, Augen gehen aus
// Augen leuchten blau für "gruss" ms
void hello() {
  auge_rot();
  delay(gruss);
  servo();
  rand_licht();
  delay(gruss);
  servo();
  rand_licht();
  delay(gruss);
  auge_rot();
  delay(gruss);
  auge_blau();
  delay(gruss);
  auge_blau();
}

// Mikro abfrage Signal

void signal_erfassen() {
  lcd.clear();
  auge_rot();
  // kurze Pause, damit ein Signal nicht zweimal erkannt wird (fälschlicherweise)
  delay(pause);
  // nach der Pause warten wir n-millisekunden auf den nächsten Wert
  current_noise_time = millis() + waiting_for_signal;
  auge_blau();
  while (current_noise_time > millis()) {
    soundValue = analogRead(A0);
    // falls Ton erkannt wird, boolean auf 1 setzen
    if (soundValue <= threshhold) {
      first = 1;
      // kurze Pause, damit ein Signal nicht zweimal erkannt wird (fälschlicherweise)
      delay(pause);
      // Zeit updaten, damit nicht unnötig lang gewartet wird
      current_noise_time = millis();
    }
  }
  lcd.print(int(first));
  // 2ter Durchlauf
  current_noise_time = millis() + waiting_for_signal;
  auge_rot();
  while (current_noise_time > millis()) {
    soundValue = analogRead(A0);
    // falls Ton erkannt wird, boolean auf 1 setzen
    if (soundValue <= threshhold) {
      second = 1;
      // kurze Pause, damit ein Signal nicht zweimal erkannt wird (fälschlicherweise)
      delay(pause);
      // Zeit updaten, damit nicht unnötig lang gewartet wird
      current_noise_time = millis();
    }
  }
  auge_rot();
  // 3ter Durchlauf
  current_noise_time = millis() + waiting_for_signal;
  rand_licht();
  while (current_noise_time > millis()) {
    soundValue = analogRead(A0);
    // falls Ton erkannt wird, boolean auf 1 setzen
    if (soundValue <= threshhold) {
      third = 1;
      // Zeit updaten, damit nicht unnötig lang gewartet wird
      current_noise_time = millis();
    }
  }
  rand_licht();
  delay(pause);
  //alten Zustand wiederherstellen
  alter_zustand();
  // ausführen von Befehlen
  // for the eyes
  if (first && second && third) ein_aus();
  if (first && second && !third) servo();
  if (first && !second && third) auge_blau();
  if (first && !second && !third) auge_rot();
  if (!first && second && third) rand_licht();
  // for the ears
  if (!first && second && !third) music("Zelda", zelda_melody, sizeof(zelda_melody), 88);
  if (!first && !second && third) music("Harry Potter", harry_potter_melody, sizeof(harry_potter_melody), 144);
  if (!first && !second && !third) music("Star Wars", star_wars_melody, sizeof(star_wars_melody), 108);
  // booleans auf 0 setzen für nächstes Signal
  first = 0;
  second = 0;
  third = 0;
}

// hoch oder runterfahren
void servo() {
  // falls Maske hochgefahren ist, runterfahren
  if (pos == 160) {
    for (pos = 160; pos >= 110; pos -= 1) {
      myservo.write(pos);  // tell servo to go to position in variable 'pos'
      delay(15);           // waits 15ms for the servo to reach the position, sonst extrem schnell
    }
  } else {
    // falls unten, hochfahren
    for (pos = 110; pos < 160; pos += 1) {
      myservo.write(pos);                   // tell servo to go to position in variable 'pos'
      delay(15);                            // waits 15ms for the servo to reach the position, sonst extrem schnell
    }
  }
}

// ein-/ausschalten je nachdem was das letze mal war
// als erstens einschalten
void ein_aus() {
  if (start) start_zustand();
  else alles_aus();
  start = !start;
}

// Start (einschalten)

void start_zustand() {
  // Maske runterfahren falls oben
  if (pos == 180) servo();
  // Rand-LEDs einschalten
  if (led_state_rand == LOW) rand_licht();
  // Augen rot leuchten lassen (falls blau, werden sie auf rot gewechselt)
  if (led_state_auge_rot == LOW) auge_rot();
}

// runterfahren

void lichter_aus() {
  red = 0;
  blue = 0;
  rim = 0;
  // LEDs ausschalten
  if (led_state_rand == HIGH) {
    rand_licht();
    rim = 1;
  }
  // falls Augen in einer Farbe leuchten, werden sie durch aufruf der Funktion deaktiviert
  if (led_state_auge_rot == HIGH) {
    auge_rot();
    red = 1;
  }
  if (led_state_auge_blau == HIGH) {
    auge_blau();
    blue = 1;
  }
}

void alter_zustand() {
  if (rim) rand_licht();
  if (red) auge_rot();
  if (blue) auge_blau();
}

void alles_aus() {
  // Maske runterfahren falls oben
  if (pos == 180) servo;
  lichter_aus();
}

// Rand LEDs

void rand_licht() {
  // Rand-LEDs einschalten, falls aus
  if (led_state_rand == LOW) {
    led_state_rand = HIGH;
  } else {
    led_state_rand = LOW;
  }
  // Zustand updaten (hier wird erst der Pin angesteuert)
  digitalWrite(led_pin_rand, led_state_rand);
}

// Augen LEDS

//Rot

void auge_rot() {
  // falls rot aus ist, rot einschalten und blau ausschalten
  if (led_state_auge_rot == LOW) {
    led_state_auge_rot = HIGH;
    led_state_auge_blau = LOW;
  } else {
    led_state_auge_rot = LOW;
  }
  // neue Werte an Pins übergeben, erst ausschalten, dann an
  digitalWrite(led_pin_auge_blau, led_state_auge_blau);
  digitalWrite(led_pin_auge_rot, led_state_auge_rot);
}

//Blau

void auge_blau() {
  // falls rot aus ist, rot einschalten und blau ausschalten
  if (led_state_auge_blau == LOW) {
    led_state_auge_blau = HIGH;
    led_state_auge_rot = LOW;
  } else {
    led_state_auge_blau = LOW;
  }
  // neue Werte an Pins übergeben
  digitalWrite(led_pin_auge_rot, led_state_auge_rot);
  digitalWrite(led_pin_auge_blau, led_state_auge_blau);
}

// Melodien

/*
Werte für Melodien
music("Zelda", zelda_melody, sizeof(zelda_melody), 88);
music("Star Wars", star_wars_melody, sizeof(star_wars_melody), 108);
music("Harry Potter", harry_potter_melody, sizeof(harry_potter_melody), 144);
*/

void music(char name[], int melody[], int sizeofmelody, int tempo) {
  long time_since_start = millis();
  int wholenote = (60000 * 4) / tempo;
  int divider = 0, noteDuration = 0;
  lcd.clear();
  lcd.print(name);
  // sizeof gives the number of bytes, each int value is composed of two bytes (16 bits)
  // there are two values per note (pitch and duration), so for each note there are four bytes
  int notes = sizeofmelody / sizeof(melody[0]) / 2;


  for (int thisNote = 0; thisNote < notes * 2; thisNote = thisNote + 2) {

    // calculates the duration of each note
    divider = melody[thisNote + 1];
    if (divider > 0) {
      // regular note, just proceed
      noteDuration = (wholenote) / divider;
    } else if (divider < 0) {
      // dotted notes are represented with negative durations!!
      noteDuration = (wholenote) / abs(divider);
      noteDuration *= 1.5;  // increases the duration in half for dotted notes
    }
    // print the number of seconds since reset:
    // set the cursor to column 0, line 1
    // (note: line 1 is the second row, since counting begins with 0):
    lcd.setCursor(0, 1);
    lcd.print((millis() - time_since_start) / 1000);

    // we only play the note for 90% of the duration, leaving 10% as a pause
    tone(buzzer, melody[thisNote], noteDuration * 0.9);

    // Wait for the specief duration before playing the next note.
    delay(noteDuration);

    // stop the waveform generation before the next note.
    noTone(buzzer);
    // print the number of seconds since reset:
    // set the cursor to column 0, line 1
    // (note: line 1 is the second row, since counting begins with 0):
    lcd.setCursor(0, 1);
    lcd.print((millis() - time_since_start) / 1000);
  }
  lcd.clear();
}
