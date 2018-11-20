#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

 Adafruit_PCD8544 display = Adafruit_PCD8544(14, 15, 16);
#define BOARD_WIDTH 51
#define BOARD_HEIGHT 48
#define max_snake_length 500
#define default_snake_pos 1000
#define UP_BUTTON 5
#define DOWN_BUTTON 7
#define RIGHT_BUTTON 6
#define LEFT_BUTTON 4
#define default_game_speed 500
typedef enum {UP,DOWN,LEFT,RIGHT} direction;
enum snake_state{HIT_BODY, HIT_BORDER,HIT_FOOD,NO_HIT};
enum {NO_INTERRUPT, INTERRUPT_UP, INTERRUPT_DOWN, INTERRUPT_LEFT, INTERRUPT_RIGHT} interrupt_event;
void update_snake_pos(int* snake, int snake_length, direction snake_direction)
{
  for(int i=snake_length-1;i>=1;i--)
  {
    snake[i]=snake[i-1]; // update from tail to head
  }
  // update head
  switch( snake_direction)
  {
    case UP:
    {
      snake[0]=snake[0]-BOARD_WIDTH;
      break;
    }
    case DOWN:
    {
      snake[0]=snake[0]+BOARD_WIDTH;
      break;
    }
    case LEFT:
    {
      snake[0]=snake[0]-1;
      break;
    }
    case RIGHT:
    {
      snake[0]=snake[0]+1;
      break;
    }
  }
}
snake_state get_snake_state(int* snake, int snake_length,int food_pos)
{
  for(int i=1;i<snake_length;i++)
  {
    if(snake[0]==snake[i]) 
    {
      Serial.print(snake[0]);
      Serial.println("HIT BODY");
      return HIT_BODY; //Its head hit the body
    }
  }
  if((snake[0])%BOARD_WIDTH==0 || (snake[0] % BOARD_WIDTH)==(BOARD_WIDTH-1)) 
  {
    Serial.println("HIT BORDER");
    return HIT_BORDER; // SNAKE HITS SIDE BORDER
  }
  if(snake[0]<=BOARD_WIDTH)   {
    Serial.println("HIT BORDER");
    return HIT_BORDER;
  }  //snake hits upper border
  if(snake[0]>=(BOARD_WIDTH*(BOARD_HEIGHT-1)) && snake[0]<=((BOARD_WIDTH*BOARD_HEIGHT)-1)) {
    Serial.println("HIT BORDER");
    return HIT_BORDER;
  } // SNAKE HIT LOWER BORDER
  if(snake[0]==food_pos) return HIT_FOOD;
  else return NO_HIT;
}
void hit_food(int * snake,int * snake_length, int food_pos)
{
  for(int i=(*snake_length-1);i>=0;i--)
  {
    snake[i+1]=snake[i]; //make a room for new snake head
  }
  snake[0]=food_pos; //new snake_head
  *snake_length = *snake_length + 1;
}
void spawn_food(int *food_pos, int*snake, int snake_length)
{
  int spawn_border=0;
  int spawn_body=0;
  while(spawn_border==0||spawn_body==0)
  {
  *food_pos=random(BOARD_WIDTH+1,(BOARD_WIDTH-1)*(BOARD_HEIGHT-1));
  for(int i=0;i<snake_length;i++)
  {
    if(*food_pos==snake[i])
    {
      spawn_body=0;   // spawn on body=>respawn
      break;
    }
    else
    {
      spawn_body=1;  // good
    }
  }
  if( (*food_pos) % BOARD_WIDTH==0 || 
      (*food_pos) % BOARD_WIDTH==(BOARD_WIDTH-1) ||
      (*food_pos) <=BOARD_WIDTH ||
      ((*food_pos)>=(BOARD_WIDTH*(BOARD_HEIGHT-1)) && (*food_pos)<=((BOARD_WIDTH*BOARD_HEIGHT)-1))) spawn_border=0;
  else spawn_border=1; // good to go
  }
}
void game_over(int score)
{
  display.clearDisplay();
  display.setTextColor(BLACK);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("END GAME");
  display.setCursor(0,20);
  display.setTextSize(0);
  display.print("max score:");
  display.setCursor(0,30);
  display.print(score);
  display.display();
  while(1);
}
void draw_board(int * snake, int snake_length,int food_pos, int score)
{
  display.clearDisplay();
  display.drawRect(0,0,BOARD_WIDTH,BOARD_HEIGHT,BLACK);
  for(int i=0;i<snake_length;i++)
  {
    display.drawPixel((int) snake[i]%BOARD_WIDTH,(int)snake[i]/BOARD_WIDTH,BLACK);
  }
  display.drawPixel((int) food_pos%BOARD_WIDTH,(int)food_pos/BOARD_WIDTH,BLACK);
  display.setTextColor(BLACK);
  display.setTextSize(0);
  display.setCursor(53,0);
  display.print("Score");
  display.setCursor(55,15);
  display.print(score);
  display.display();
}
void interrupt_handle()
{
  static uint64_t last_millis=0;
  if(millis()-last_millis>=5)
  {
      last_millis=millis();
      if(digitalRead(UP_BUTTON)==0)
      {
        interrupt_event=INTERRUPT_UP;
        return;
      }
      if(digitalRead(DOWN_BUTTON)==0)
      {
        interrupt_event=INTERRUPT_DOWN;
        return;
      }
      if(digitalRead(RIGHT_BUTTON)==0)
      {
        interrupt_event=INTERRUPT_RIGHT;
        return;
      }
      if(digitalRead(LEFT_BUTTON)==0)
      {
        interrupt_event=INTERRUPT_LEFT;
        return;
      }
  }
}
void setup()
{
 pinMode(UP_BUTTON,INPUT_PULLUP);
 pinMode(DOWN_BUTTON,INPUT_PULLUP);
 pinMode(RIGHT_BUTTON,INPUT_PULLUP);
 pinMode(LEFT_BUTTON,INPUT_PULLUP);
 pinMode(2,INPUT_PULLUP);
 display.begin();  
 display.setContrast(15);
 display.clearDisplay();
 display.drawRect(0,0,BOARD_WIDTH,BOARD_HEIGHT,WHITE);
 display.display();
 attachInterrupt(0,interrupt_handle,FALLING);
 randomSeed(analogRead(5));
 Serial.begin(9600);
}
void loop()
{
 int food_pos=0;
 int SNAKE[max_snake_length];
 int current_snake_length=3;
 SNAKE[0]=default_snake_pos;
 SNAKE[1]=default_snake_pos-1;
 SNAKE[2]=default_snake_pos-2;
 direction snake_direction=RIGHT;
 spawn_food(&food_pos,SNAKE,current_snake_length);
 int game_speed=default_game_speed;
 int score=0;
 uint64_t last_update=millis();
 while(1)
 {
  if(millis()-last_update>=game_speed)
  {
  update_snake_pos(SNAKE,current_snake_length,snake_direction); // update_snake_position
  if(get_snake_state(SNAKE,current_snake_length,food_pos) == HIT_BORDER || get_snake_state(SNAKE,current_snake_length,food_pos)== HIT_BODY) game_over(score); //stop game here
  else 
   {
    if( get_snake_state(SNAKE,current_snake_length,food_pos)==HIT_FOOD) 
      {
        hit_food(SNAKE,&current_snake_length,food_pos);// HIT FOOD=NEW HEAD
        spawn_food(&food_pos,SNAKE,current_snake_length); // spawn new food
        score++;
        current_snake_length++;
        if(score>=max_snake_length-3) game_over(score);
      }
   }
   last_update=millis();
  }
   // now let check if there are any input
   if(interrupt_event!=NO_INTERRUPT)
   {
    switch(interrupt_event)
      {
        case INTERRUPT_UP:
          {
            if(snake_direction!=DOWN) snake_direction=UP;
            break;
          }
        case INTERRUPT_DOWN:
          {
            if(snake_direction!=UP) snake_direction=DOWN;
            break;
          }
        case INTERRUPT_LEFT:
          {
            if(snake_direction!=RIGHT) snake_direction=LEFT;
            break;
          }
        case INTERRUPT_RIGHT:
          {
            if(snake_direction!=LEFT) snake_direction=RIGHT;
            break;
          }
      }
     interrupt_event=NO_INTERRUPT;
   }
   // check if holding_button
   if(digitalRead(2)==0)
   {
    if(game_speed>40) game_speed-=5;
   }
   else game_speed=default_game_speed;
   // let display_board
   draw_board(SNAKE,current_snake_length,food_pos,score);
}
}







