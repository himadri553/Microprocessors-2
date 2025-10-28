"""
    Lab 2: Game Control with Joystick, Gyro, and Accelerometer
    EECE.5520 - Microprocessor II and Embedded System Design
    Himadri Saha, Daniel Burns, Chris Worthley

    snake.py:
    - Runs the snake game controlled by Arduino joystick or gyro
    - Gyro shake turns apple yellow for double points
    - Joystick/gyro inputs control movement via serial
"""

"""
Snake Game controlled by Arduino Joystick or Gyro
- Gyro shake makes apple yellow for double points
- Thread-safe GUI updates (no 'not in main loop' errors)
"""

import turtle
import time
import random
import serial
import threading

# --- Ask for controller mode ---
print("====================================")
print("   Snake Controller Configuration   ")
print("====================================")
print("[1] Joystick")
print("[2] Gyro")
mode = input("Enter controller mode (1 or 2): ").strip()
if mode not in ["1", "2"]:
    print("Invalid input. Defaulting to joystick mode (1).")
    mode = "1"

# --- Serial setup ---
joystick_enabled = False
port = "COM8"   # change if your Arduino uses another port
try:
    ser = serial.Serial(port, 9600, timeout=0.01)
    time.sleep(2)
    ser.reset_input_buffer()
    joystick_enabled = True
    print(f"✅ Arduino connected on {port}")
except Exception as e:
    print(f"⚠️  Could not open {port}: {e}")
    joystick_enabled = False

if joystick_enabled:
    try:
        ser.write(mode.encode("utf-8"))
        ser.flush()
        print(f"Sent controller mode {mode} to Arduino.")
    except Exception as e:
        print("⚠️  Failed to send mode:", e)

# --- Game variables ---
delay = 0.1
score = 0
high_score = 0
ppa = 10
double_points = False
shake_detected = False   # <--- thread-safe flag

# --- Screen setup ---
wn = turtle.Screen()
wn.title("Snake Game (Arduino Controlled)")
wn.bgcolor("green")
wn.setup(width=600, height=600)
wn.tracer(0)

# --- Snake head ---
head = turtle.Turtle()
head.speed(0)
head.shape("square")
head.color("black")
head.penup()
head.goto(0, 0)
head.direction = "stop"

# --- Food ---
food = turtle.Turtle()
food.speed(0)
food.shape("circle")
food.color("red")
food.penup()
food.goto(0, 100)

segments = []

# --- Score display ---
pen = turtle.Turtle()
pen.speed(0)
pen.shape("square")
pen.color("white")
pen.penup()
pen.hideturtle()
pen.goto(0, 260)
pen.write("Score: 0  High Score: 0  P/A: 10",
          align="center", font=("Courier", 24, "normal"))

# --- Movement functions ---
def go_up():
    if head.direction != "down":
        head.direction = "up"

def go_down():
    if head.direction != "up":
        head.direction = "down"

def go_left():
    if head.direction != "right":
        head.direction = "left"

def go_right():
    if head.direction != "left":
        head.direction = "right"

def move():
    if head.direction == "up":
        head.sety(head.ycor() + 20)
    elif head.direction == "down":
        head.sety(head.ycor() - 20)
    elif head.direction == "left":
        head.setx(head.xcor() - 20)
    elif head.direction == "right":
        head.setx(head.xcor() + 20)

def convert_controller_to_key(ch):
    mapping = {'w': go_up, 's': go_down, 'a': go_left, 'd': go_right}
    if ch.lower() in mapping:
        mapping[ch.lower()]()

# --- Keyboard fallback ---
wn.listen()
wn.onkey(go_up, "w")
wn.onkey(go_down, "s")
wn.onkey(go_left, "a")
wn.onkey(go_right, "d")

# --- Background thread to read serial ---
def read_controller():
    global shake_detected
    while True:
        if joystick_enabled and ser.in_waiting:
            try:
                data = ser.read(ser.in_waiting).decode('utf-8', errors='ignore')
                for ch in data:
                    if ch == 'X':
                        shake_detected = True        # set flag; main loop handles GUI
                    elif ch in ['w', 'a', 's', 'd']:
                        convert_controller_to_key(ch)
            except Exception:
                pass
        time.sleep(0.05)

if joystick_enabled:
    t = threading.Thread(target=read_controller, daemon=True)
    t.start()

# --- Buzzer feedback ---
def safe_beep():
    if not joystick_enabled:
        return
    try:
        ser.write(b'E')
        ser.flush()
    except Exception:
        pass

# --- Main game loop ---
while True:
    wn.update()

    # Handle shake in the main thread safely
    if shake_detected:
        food.color("yellow")
        double_points = True
        shake_detected = False
        print("🍋 Shake detected! Next apple worth double points!")

    # Border collision
    if head.xcor() > 290 or head.xcor() < -290 or head.ycor() > 290 or head.ycor() < -290:
        time.sleep(1)
        head.goto(0, 0)
        head.direction = "stop"
        for seg in segments:
            seg.goto(1000, 1000)
        segments.clear()
        score = 0
        delay = 0.1
        pen.clear()
        pen.write(f"Score: {score}  High Score: {high_score}  P/A: {ppa}",
                  align="center", font=("Courier", 24, "normal"))

    # Food collision
    if head.distance(food) < 20:
        threading.Thread(target=safe_beep, daemon=True).start()
        food.goto(random.randint(-290, 290), random.randint(-290, 290))

        new_segment = turtle.Turtle()
        new_segment.speed(0)
        new_segment.shape("square")
        new_segment.color("grey")
        new_segment.penup()
        segments.append(new_segment)

        delay = max(0.05, delay - 0.001)

        gained = 10 * (2 if double_points else 1)
        score += gained
        if double_points:
            print(f"💰 Double points! You earned {gained} points!")
            double_points = False
            food.color("red")

        if score > high_score:
            high_score = score

        pen.clear()
        pen.write(f"Score: {score}  High Score: {high_score}  P/A: {ppa}",
                  align="center", font=("Courier", 24, "normal"))

    # Move body
    for i in range(len(segments) - 1, 0, -1):
        x = segments[i - 1].xcor()
        y = segments[i - 1].ycor()
        segments[i].goto(x, y)

    if segments:
        segments[0].goto(head.xcor(), head.ycor())

    move()

    # Self collision
    for seg in segments:
        if seg.distance(head) < 20:
            time.sleep(1)
            head.goto(0, 0)
            head.direction = "stop"
            for s in segments:
                s.goto(1000, 1000)
            segments.clear()
            score = 0
            delay = 0.1
            pen.clear()
            pen.write(f"Score: {score}  High Score: {high_score}  P/A: {ppa}",
                      align="center", font=("Courier", 24, "normal"))
            break

    time.sleep(delay)

wn.mainloop()

