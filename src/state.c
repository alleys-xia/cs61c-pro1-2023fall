#include "state.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "snake_utils.h"

/* Helper function definitions */
static void set_board_at(game_state_t* state, unsigned int row, unsigned int col, char ch);
static bool is_tail(char c);
static bool is_head(char c);
static bool is_snake(char c);
static char body_to_tail(char c);
static char head_to_body(char c);
static unsigned int get_next_row(unsigned int cur_row, char c);
static unsigned int get_next_col(unsigned int cur_col, char c);
static void find_head(game_state_t* state, unsigned int snum);
static char next_square(game_state_t* state, unsigned int snum);
static void update_tail(game_state_t* state, unsigned int snum);
static void update_head(game_state_t* state, unsigned int snum);

/* Task 1 */
game_state_t* create_default_state() {
    //18行20列的游戏空间
    size_t columns= 20;
    size_t rows= 18;
  game_state_t* state= malloc(sizeof(game_state_t));
  //如果分配内存失败，则退出
  if(state == NULL) {
      exit(1);
  }
  state->board= malloc(sizeof(char*) * rows);
    if(state->board == NULL)
        exit(1);
  for(size_t i=0;i<rows;++i) {
      state->board[i]= malloc(sizeof(char) * (columns + 1));
      if(state->board[i] == NULL)
          exit(1);
      /**
       * strcpy函数：char *strcpy(char *destination, const char *source);
       * */
      if(i==0 || i==(rows - 1)) {
          strcpy(state->board[i], "####################");
      }else if(i==2) {
          strcpy(state->board[i], "# d>D    *         #");
      }else {
          strcpy(state->board[i], "#                  #");
      }
  }
    state->num_rows= 18;
    state->num_snakes= 1;
    state->snakes= malloc(sizeof(snake_t));
    state->snakes->head_row= 2;
    state->snakes->head_col= 4;
    state->snakes->tail_row= 2;
    state->snakes->tail_col= 2;
    state->snakes->live= true;
    return state;
}

/* Task 2 */
void free_state(game_state_t* state) {
  for(size_t i=0;i<18; ++i) {
      //释放字符串空间
    free(state->board[i]);
  }
  //释放char*动态数组空间
  free(state->board);
  //释放snakes*动态数组空间
  free(state->snakes);
  //释放state空间
  free(state);
}

/* Task 3 */
void print_board(game_state_t* state, FILE* fp) {
    for(size_t i=0;i<state->num_rows;++i) {
        fprintf(fp, "%s\n", state->board[i]);
    }
}

/*
  Saves the current state into filename. Does not modify the state object.
  (already implemented for you).
*/
void save_board(game_state_t* state, char* filename) {
  FILE* f = fopen(filename, "w");
  print_board(state, f);
  fclose(f);
}

/* Task 4.1 */
/*
  Helper function to get a character from the board
  (already implemented for you).
*/
char get_board_at(game_state_t* state, unsigned int row, unsigned int col) {
  return state->board[row][col];
}

/*
  Helper function to set a character on the board
  (already implemented for you).
*/
static void set_board_at(game_state_t* state, unsigned int row, unsigned int col, char ch) {
  state->board[row][col] = ch;
}

/*
  Returns true if c is part of the snake's tail.
  The snake consists of these characters: "wasd"
  Returns false otherwise.
*/
static bool is_tail(char c) {
    return (c == 'w' || c == 'a' || c == 's' || c == 'd');
}

/*
  Returns true if c is part of the snake's head.
  The snake consists of these characters: "WASDx"
  Returns false otherwise.
*/
static bool is_head(char c) {
    return (c == 'W' || c == 'A' || c == 'S' || c == 'D' || c == 'x');
}

/*
  Returns true if c is part of the snake.
  The snake consists of these characters: "wasd^<v>WASDx"
*/
static bool is_snake(char c) {
  if(is_head(c) || is_tail(c))
      return true;
  return (c == '^' || c == '<' || c == 'v' || c == '>');
}
/*
  Converts a character in the snake's body ("^<v>")
  to the matching character representing the snake's
  tail ("wasd").
*/
static char body_to_tail(char c) {
    char res=c;
    switch (c) {
        case '^':
            res= 'w';
            break;
        case 'v':
            res= 's';
            break;
        case '<':
            res= 'a';
            break;
        case '>':
            res= 'd';
            break;
        default:
            break;
    }
  return res;
}

/*
  Converts a character in the snake's head ("WASD")
  to the matching character representing the snake's
  body ("^<v>").
*/
static char head_to_body(char c) {
    char res=c;
    switch (c) {
        case 'W':
            res= '^';
            break;
        case 'S':
            res= 'v';
            break;
        case 'A':
            res= '<';
            break;
        case 'D':
            res= '>';
            break;
        default:
            break;
    }
    return res;
}

/*
  Returns cur_row + 1 if c is 'v' or 's' or 'S'.
  Returns cur_row - 1 if c is '^' or 'w' or 'W'.
  Returns cur_row otherwise.
*/
static unsigned int get_next_row(unsigned int cur_row, char c) {
    switch (c) {
        case 'v':
        case 's':
        case 'S':
            cur_row += 1;
            break;
        case '^':
        case 'w':
        case 'W':
            cur_row -= 1;
        default:
            break;
    }
  return cur_row;
}

/*
  Returns cur_col + 1 if c is '>' or 'd' or 'D'.
  Returns cur_col - 1 if c is '<' or 'a' or 'A'.
  Returns cur_col otherwise.
*/
static unsigned int get_next_col(unsigned int cur_col, char c) {
    switch (c) {
        case '>':
        case 'd':
        case 'D':
            cur_col += 1;
            break;
        case '<':
        case 'a':
        case 'A':
            cur_col -= 1;
        default:
            break;
    }
    return cur_col;
}

/*
  Task 4.2

  Helper function for update_state. Return the character in the cell the snake is moving into.

  This function should not modify anything.
*/
//根据蛇头的方向返回下一步即将到达的字符
static char next_square(game_state_t* state, unsigned int snum) {
  unsigned int row= state->snakes[snum].head_row;
  unsigned int col= state->snakes[snum].head_col;
  //找到当前的蛇头的方向
  char head_point= get_board_at(state,row,col);
  if(head_point=='W') {
      return state->board[row-1][col];
  }else if(head_point=='A') {
      return state->board[row][col-1];
  }else if(head_point=='S') {
      return state->board[row+1][col];
  }else{
      return state->board[row][col+1];
  }
}

/*
  Task 4.3

  Helper function for update_state. Update the head...

  ...on the board: add a character where the snake is moving

  ...in the snake struct: update the row and col of the head

  Note that this function ignores food, walls, and snake bodies when moving the head.
*/
static void update_head(game_state_t* state, unsigned int snum) {
    unsigned int row= state->snakes[snum].head_row;
    unsigned int col= state->snakes[snum].head_col;
    //找到当前的蛇头的方向
    char head_point= get_board_at(state,row,col);
    //死蛇直接返回
    if(head_point=='x') {
        return;
    }
    //update state:head->body
    set_board_at(state, row, col, head_to_body(head_point));
    if(head_point=='D') {
        //update state:
        set_board_at(state, row, col+1, head_point);
        //update snake
        state->snakes[snum].head_col= col+1;
    }else if(head_point=='A') {
        //update state
        set_board_at(state, row, col-1, head_point);
        //update snake
        state->snakes[snum].head_col= col-1;
    }else if(head_point=='W') {
        //update state
        set_board_at(state, row-1, col, head_point);
        //update snake
        state->snakes[snum].head_row= row-1;
    }else {
        //update state
        set_board_at(state, row+1, col, head_point);
        //update snake
        state->snakes[snum].head_row= row+1;
    }
}

/*
  Task 4.4

  Helper function for update_state. Update the tail...

  ...on the board: blank out the current tail, and change the new
  tail from a body character (^<v>) into a tail character (wasd)

  ...in the snake struct: update the row and col of the tail
*/
static void update_tail(game_state_t* state, unsigned int snum) {
    unsigned int row= state->snakes[snum].tail_row;
    unsigned int col= state->snakes[snum].tail_col;
    //找到当前的蛇尾的方向
    char tail_point= get_board_at(state, row, col);
    //blank out the current tail
    set_board_at(state, row, col, ' ');
    if(tail_point == 'd') {
        //update snake tail_col
        state->snakes[snum].tail_col= col+1;
    }else if(tail_point == 'a') {
        state->snakes[snum].tail_col= col-1;
    }else if(tail_point == 'w') {
        state->snakes[snum].tail_row= row-1;
    }else {
        state->snakes[snum].tail_row= row+1;
    }
    //获取当前的即将变为尾部的字符
    char c= get_board_at(state, state->snakes[snum].tail_row, state->snakes[snum].tail_col);
    //change the new tail from a body character (^<v>) into a tail character (wasd)
    set_board_at(state, state->snakes[snum].tail_row, state->snakes[snum].tail_col,body_to_tail(c));
}

/* Task 4.5 */
void update_state(game_state_t* state, int (*add_food)(game_state_t* state)) {
    unsigned int head_row, head_col;
    //  每条蛇依次更新
    for(unsigned int i= 0;i<state->num_snakes;++i) {
        head_row= state->snakes[i].head_row;
        head_col= state->snakes[i].head_col;
        //找到蛇头即将接触的字符，若为蛇身或者边界则蛇死亡
        char temp= next_square(state, i);
        if(is_snake(temp) || temp=='#'){
            //设置蛇头为x标识死亡,同时更新蛇数组中的状态
            set_board_at(state, head_row, head_col, 'x');
            state->snakes[i].live= false;
        }
        //吃到苹果时不更新tail并且生成给一个新的苹果
        else if(temp=='*'){
            update_head(state, i);
            add_food(state);
        }else {
            //默认时，更新头部以及尾部
            update_head(state, i);
            update_tail(state, i);
        }
    }
}

/* Task 5 */
game_state_t* load_board(FILE* fp) {
    if(fp==NULL)
        return NULL;
    // 内存中的堆区分配内存
    game_state_t* state= malloc(sizeof(game_state_t));
    //初始化
    state->num_snakes= 0;
    state->num_rows= 0;
    state->snakes= NULL;
    state->board= malloc(sizeof(char*));
    int capacity= 5;
    //设置初始缓冲区buffer长度为5
    char* buffer= malloc(sizeof(char) * (unsigned long) capacity);
    //判断是否读取到尾端,若至尾端则可以确定长度为len
    unsigned long len= 0;
    //若没有到达尾端，则扩大为2倍
    unsigned int rows= 0;
    while (fgets(buffer, capacity, fp)){
        len= strlen(buffer);
        //当前行未读取到末端时
        while(buffer[len-1]!='\n'&& !feof(fp)) {
            //容量+len
            capacity += len;
            char* new_buffer= realloc(buffer, sizeof(char) * (unsigned long) capacity);
            if(new_buffer==NULL) {
                exit(1);
            }
            fgets(new_buffer+len,sizeof(char)*((unsigned long) capacity - len), fp);
            buffer= new_buffer;
            len= strlen(buffer);
        }
        // 去掉换行符
        buffer[len - 1] = '\0';
        rows++;
        //copy to board
        state->board= realloc(state->board, sizeof(char*)*rows);
        state->board[rows-1]= malloc(sizeof(char)*(strlen(buffer)+1));
        strcpy(state->board[rows-1], buffer);
        //reset buffer以供下次读取文件
        free(buffer);
        capacity= 5;
        buffer= malloc(sizeof(char) * (unsigned long) capacity);
    }
    state->num_rows= rows;
    return state;
}

/*
  Task 6.1

  Helper function for initialize_snakes.
  Given a snake struct with the tail row and col filled in,
  trace through the board to find the head row and col, and
  fill in the head row and col in the struct.
*/
static void find_head(game_state_t* state, unsigned int snum) {
    unsigned int tail_row= state->snakes[snum].tail_row;
    unsigned int tail_col= state->snakes[snum].tail_col;
    char c= state->board[tail_row][tail_col];
    while(!is_head(c)) {
        switch (c) {
            case 'w':
            case '^':
                tail_row -=1;
                break;
            case 's':
            case 'v':
                tail_row +=1;
                break;
            case 'a':
            case '<':
                tail_col -=1;
                break;
            case 'd':
            case '>':
                tail_col +=1;
                break;
            default:
                printf("something wrong!");
                exit(1);
        }
        c= state->board[tail_row][tail_col];
    }
    //找到后更新snake中的状态
    state->snakes[snum].head_col= tail_col;
    state->snakes[snum].head_row= tail_row;
    state->snakes[snum].live= true;
}




/* Task 6.2 */
game_state_t* initialize_snakes(game_state_t* state) {
    unsigned int snum= 0;
    for(size_t i=0;i<state->num_rows;++i) {
        size_t j=0;
        //find tail
        while(state->board[i][j]!='\0') {
            ++snum;
            if(is_tail(state->board[i][j])){
                state->snakes[++snum].tail_row= (unsigned int)i;
                state->snakes[++snum].tail_col= (unsigned int)j;
                //find head
                find_head(state, snum);
                break;
            }
            j++;
        }
    }
    //update总的数量
    state->num_snakes= snum;
    return state;
}
