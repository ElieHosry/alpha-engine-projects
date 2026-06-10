# Input Handling

Alpha Engine only supports mouse and keyboard input.
It does not support input from other sources (e.g. controllers).

## Mouse Position

Getting the current mouse cursor position is straightforward with `AEInputGetCursorPosition()`:

```c
s32 x, y;
AEInputGetCursorPosition(&x, &y);
// After the previous line, x should contain the x-position of the cursor
// while y will contain the y-position of the cursor.
```

!!!warning
    
    Note that the mouse cursor position obtained via `AEInputGetCursorPosition()` is in Screen Coordinates. Screen Coordinates are detailed here: <<screen_coordinates>>.

## Key Presses 

When it comes to key input (via mouse buttons or keyboard keys), pay close attention to how a key is being pressed. 
The OS can only tell us whether a key is pressed or released. 
Alpha Engine takes a step further by recording whether a key is pressed/released the previous frame.  
For common cases, the following functions might be good enough for your application:

* `AEInputCheckTriggered()`: This checks if a key is just pressed.
* `AEInputCheckReleased()`: This checks if a key is just released.

For example:

```c
// Checks if escape key is recently pressed.
if (AEInputCheckTriggered(AEVK_ESCAPE)) { ... }

// Checks if escape key is recently released.
if (AEInputCheckRelease(AEVK_ESCAPE)) { ... }
```

For more information related to input, check out the documentation surrounding the `AEInput.h` file.
