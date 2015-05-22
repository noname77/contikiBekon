#define CURSOR_LEFT 0
#define CURSOR_RIGHT 1
#define LCD_BACKLIGHT_PIN 7

//initialize the display
void lcd_init();

//print string str to display
void lcd_print(char* str);

//clear the display
void lcd_clear();


//put cursor home (pos 1,1)
void lcd_home();

//move cursor to the left (dir = CURSOR_LEFT) or right (dir = CURSOR_RIGHT)
void lcd_move_cursor(char dir);

//move cursor to row r(0/1), column c(0-16)
void lcd_set_cursor(int r, int c);

//turn cursor on/off (1/0)
void lcd_cursor(char state);

//turn blink on/off (1/0)
void lcd_blink(char state);

//turn backlight on/off (1/0)
void lcd_backlight(char state);

//delete current character at cursor
void lcd_delete();
