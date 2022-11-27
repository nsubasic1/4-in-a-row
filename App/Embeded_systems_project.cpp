#include "mbed.h"
#include "stm32f413h_discovery_ts.h"
#include "stm32f413h_discovery_lcd.h"
#include "vector"
#define PI 4 * atan(1)

enum StateOfSlot { NOTTAKEN, YELLOW, RED }; 

TS_StateTypeDef TS_State = { 0 };
int viewNow = 0;
int pastView = 0;
void makeHomeView();
bool housePressed(int x1, int y1);

StateOfSlot player = StateOfSlot::YELLOW;

void homeButton(){
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_DrawCircle(220, 20, 12);
    BSP_LCD_FillRect(215, 18, 12, 8);
    Point p[3]; // trougao sa 3 vrha
	    p[0].X = 212; 
	    p[1].X = 221; 
	    p[2].X = 229;
	    p[0].Y = 18;
	    p[1].Y = 12;
	    p[2].Y = 18;
	    BSP_LCD_FillPolygon(p, 3);
}

class Slot{
    int xKoord, yKoord;
    StateOfSlot state;
public:
    Slot(int x, int y, StateOfSlot s){
        xKoord = x;
        yKoord = y;
        state = s;
    }
    StateOfSlot getState(){
        return state;
    }
    void changeState(StateOfSlot s){
        state = s;
    }
    int getXKoord(){return xKoord;}
    int getYKoord(){return yKoord;}
};

typedef  std::vector<std::vector<Slot> > VectorOfSlots;

class PlayingBoard{
    VectorOfSlots slots;
public:
    PlayingBoard(){}
    PlayingBoard(VectorOfSlots s){
        slots = s;
    }
    void makeTheBoard(){
    player = StateOfSlot::YELLOW;
    VectorOfSlots v;
    for(int i = 0; i < 6; i++){
        std::vector<Slot> forInsert;
        for(int j = 0; j < 7; j++){
            Slot toInsert(36 + 26.8 * j, 61 + 24*i, StateOfSlot::NOTTAKEN);
            forInsert.push_back(toInsert);
        }
        v.push_back(forInsert);
    }
    slots = v;
    }
    int findFirstAvailableSlot(int column){
        // vraca index reda u koji cemo ubaciti zeton
        // ovom informacijom se koristimo i za for petlju
        // kada budemo crtali "animaciju"
        for(int i = slots.size() - 1; i >= 0; i--){
            if(slots.at(i).at(column).getState() == StateOfSlot::NOTTAKEN){
                return i;
            }    
        }
        return -1; // sve puno
    }
    void insertCoin(int column, int index){
        for(int i = 0; i <= index; i++){
            // moram ovaj prije sto sam obojio vratiti na staru boju
            if(i > 0){
                BSP_LCD_SetTextColor(LCD_COLOR_DARKBLUE); // ZA SAD
                BSP_LCD_FillCircle (slots.at(i-1).at(column).getXKoord()
                    , slots.at(i-1).at(column).getYKoord(), 8);
                BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
                BSP_LCD_DrawCircle (slots.at(i-1).at(column).getXKoord()
                    , slots.at(i-1).at(column).getYKoord(), 8);     
            }
            if(player == StateOfSlot::YELLOW)
                BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
            else BSP_LCD_SetTextColor(LCD_COLOR_RED);
            BSP_LCD_FillCircle (slots.at(i).at(column).getXKoord()
                , slots.at(i).at(column).getYKoord(), 8);
            BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
            BSP_LCD_DrawCircle (slots.at(i).at(column).getXKoord()
                , slots.at(i).at(column).getYKoord(), 8);
            wait_ms(100);
        }
        slots.at(index).at(column).changeState(player);
        if(endGame()){
            viewNow = 4;
                if(player == StateOfSlot::YELLOW) winnerView(1);
                else winnerView(2);
        }
        else if(drawGame()){
            viewNow = 4;
            winnerView(3);
        }
        if(player == StateOfSlot::YELLOW)
            player = StateOfSlot::RED;
        else player = StateOfSlot::YELLOW;
    }
    void updateView(){
        for(int i = 0; i < slots.size(); i++){
            for(int j = 0; j < slots.at(i).size(); j++){
                if(slots.at(i).at(j).getState() == StateOfSlot::YELLOW){
                    BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
                    BSP_LCD_FillCircle (slots.at(i).at(j).getXKoord()
                        , slots.at(i).at(j).getYKoord(), 8);
                    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
                    BSP_LCD_DrawCircle (slots.at(i).at(j).getXKoord()
                        , slots.at(i).at(j).getYKoord(), 8);
                }
                else if(slots.at(i).at(j).getState() == StateOfSlot::RED){
                    BSP_LCD_SetTextColor(LCD_COLOR_RED);
                    BSP_LCD_FillCircle (slots.at(i).at(j).getXKoord()
                        , slots.at(i).at(j).getYKoord(), 8);
                    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
                    BSP_LCD_DrawCircle (slots.at(i).at(j).getXKoord()
                        , slots.at(i).at(j).getYKoord(), 8);
                }
            }
        }
    }
    bool endGame(){
        //horizontalno provjeravanje 4 u nizu
        for(int i = 0; i< slots.size(); i++){
            int horiz = 0;
            for(int j = 0; j < slots.at(0).size(); j++)
            if(slots.at(i).at(j).getState() != player) horiz = 0;
            else{
                horiz++;
                if(horiz == 4) return true;
            }
        }
        
        //vertikalno provjeravanje 4 u nizu
        for(int i = 0; i< slots.at(0).size(); i++){
            int vert = 0;
            for(int j = 0; j < slots.size(); j++)
            if(slots.at(j).at(i).getState() != player) vert = 0;
            else{
                vert++;
                if(vert == 4) return true;
            }
        }
        //provjera po dijagonalama
        for(int i = 0; i< slots.size(); i++){
            for(int j = 0; j < slots.at(0).size(); j++){
                if(slots.at(i).at(j).getState() != player) continue;
                int ima = 1;
                int i1 = i, j1 = j;
                //krece se prema dole desno
                while(i1 != 5 && j1 != 6){
                    i1++;
                    j1++;
                    if(i1 == 6 || j1 == 7) {
                        ima = 1;
                        break;
                    }
                    if(slots.at(i1).at(j1).getState() != player){
                        ima = 1;
                        break;
                    } 
                    else{
                        ima++;
                        if(ima == 4) return true;
                    }
                }
                //krece se prema dole lijevo
                ima = 1;
                i1 = i; j1 = j;
                while(i1 != 5 && j1 != 0){
                    i1++;
                    j1--;
                    if(i1 == 6 || j1 == -1) {
                        ima = 1;
                        break;
                    }
                    if(slots.at(i1).at(j1).getState() != player){
                        ima = 1;
                        break;
                    } 
                    else{
                        ima++;
                        if(ima == 4) return true;
                    }
                }
                //gore desno
                ima = 1;
                i1 = i; j1 = j;
                while(i1 != 0 && j1 != 6){
                    i1--;
                    j1++;
                    if(i1 == -1 || j1 == 7) {
                        ima = 1;
                        break;
                    }
                    if(slots.at(i1).at(j1).getState() != player){
                        ima = 1;
                        break;
                    } 
                    else{
                        ima++;
                        if(ima == 4) return true;
                    }
                }
                //gore lijevo
                ima = 1;
                i1 = i; j1 = j;
                while(i1 != 0 && j1 != 0){
                    i1--;
                    j1--;
                    if(i1 == -1 || j1 == -1) {
                        ima = 1;
                        break;
                    }
                    if(slots.at(i1).at(j1).getState() != player){
                        ima = 1;
                        break;
                    } 
                    else{
                        ima++;
                        if(ima == 4) return true;
                    }
                }
            }
            
        }
        
        return false;
    }
    bool drawGame(){
        for(int i = 0; i < slots.size(); i++)
            for(int j = 0; j < slots.at(i).size(); j++)
                if(slots.at(i).at(j).getState() == StateOfSlot::NOTTAKEN)
                    return false;
        return true;
    }
    void winnerView(int player){
        viewNow = 4;
    // pozadina, elipsa i text u njoj
    BSP_LCD_SetTextColor(LCD_COLOR_DARKBLUE);
    BSP_LCD_FillRect(0, 0, 	BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
    //natpis winner
    BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
    BSP_LCD_FillRect(0, 0, BSP_LCD_GetXSize(), 40);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_SetBackColor(LCD_COLOR_BLUE);
    BSP_LCD_SetFont(&Font20);
    BSP_LCD_DisplayStringAt(0, 15, (uint8_t *)"WINNER", CENTER_MODE);
    //player natpis
    BSP_LCD_SetBackColor(LCD_COLOR_DARKBLUE);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_SetFont(&Font20);
    if(player == 1)
    BSP_LCD_DisplayStringAt(BSP_LCD_GetXSize()/2-50, BSP_LCD_GetYSize()/2,
        (uint8_t *)"PLAYER 1", LEFT_MODE);
    else if(player == 2) BSP_LCD_DisplayStringAt(BSP_LCD_GetXSize()/2-50, BSP_LCD_GetYSize()/2,
        (uint8_t *)"PLAYER 2", LEFT_MODE);
    else BSP_LCD_DisplayStringAt(BSP_LCD_GetXSize()/2-50, BSP_LCD_GetYSize()/2,
        (uint8_t *)"DRAW :(", LEFT_MODE);     
     homeButton();
     //vatromet
    int boja = 0;
    auto color = LCD_COLOR_RED;
    for(int i = BSP_LCD_GetXSize()-50; i >0; i -= 20){
        if(boja == 0){
            color = LCD_COLOR_RED;
            boja++;
        }
        else if(boja == 1){
            color = LCD_COLOR_GREEN;
            boja++;
        }
        else if(boja == 2){
            color = LCD_COLOR_YELLOW;
            boja++;
        }
        else if(boja == 3){
            color = LCD_COLOR_CYAN;
            boja=0;
        }
        for(double j = 0; j < PI; j += PI/100){
            
            int x1 = BSP_LCD_GetXSize() / 2 + j*50;
            int x2 = BSP_LCD_GetXSize() / 2 - j*50;
            int y = BSP_LCD_GetYSize() - (int)(sin(j) * i);
            BSP_LCD_DrawPixel(x1,y, color);
            BSP_LCD_DrawPixel(x1,y+1, color);
            BSP_LCD_DrawPixel(x2,y, color);
            BSP_LCD_DrawPixel(x2,y+1, color);
            //provjera za kliknut home
            BSP_TS_GetState(&TS_State);
        if(TS_State.touchDetected) {
            //ako je dodirnut ekran
            //dohvacamo koordinate
            uint16_t x1 = TS_State.touchX[0];
            uint16_t y1 = TS_State.touchY[0];
            if(housePressed(x1,y1)){
                    BSP_LCD_Clear(LCD_COLOR_DARKBLUE);
                    makeTheBoard();
                    makeHomeView();
                    pastView = 4;
                    viewNow = 0;
                    return;
                }
        }
            
            wait_ms(10);
        }
    }
}
    void playBot(){
        int column;
        do{
            column = 1 + (rand() % 6);
        }while(column == -1);
        
        int row = findFirstAvailableSlot(column);
        insertCoin(column, row);
    }
};

void pointToPlayer(int player){
    int pomX = BSP_LCD_GetXSize()/2;
    int pomY = BSP_LCD_GetYSize();
    BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
    if(player == 1){
        // pokazujemo na player1 koji je lijevo
        Point p[3]; // trougao sa 3 vrha
	    p[0].X = pomX - 25; 
	    p[1].X = pomX - 10; 
	    p[2].X = pomX - 10;
	    p[0].Y = pomY - 25;
	    p[1].Y = pomY - 35;
	    p[2].Y = pomY - 15;
	    BSP_LCD_FillPolygon(p, 3);
	    BSP_LCD_FillRect(pomX - 10, pomY - 29, 30, 7);
    }
    else{
        // pokazujemo na player 2 koji je desno
        Point p[3]; // trougao sa 3 vrha
	    p[0].X = pomX + 25; 
	    p[1].X = pomX + 10; 
	    p[2].X = pomX + 10;
	    p[0].Y = pomY - 25;
	    p[1].Y = pomY - 35;
	    p[2].Y = pomY - 15;
	    BSP_LCD_FillPolygon(p, 3);
	    BSP_LCD_FillRect(pomX - 20, pomY - 29, 30, 7);
    }
}

void make1PlayerView(){
    // pozadina i text TOUCH HERE gdje ce korisnik pritisnuti da ubaci zeton
    BSP_LCD_SetTextColor(LCD_COLOR_DARKBLUE);
    BSP_LCD_FillRect(0, 0, 	BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
    BSP_LCD_SetBackColor(LCD_COLOR_DARKBLUE);
    BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
    BSP_LCD_SetFont(&Font12);
    BSP_LCD_DisplayStringAt(BSP_LCD_GetXSize()/2 - 62, BSP_LCD_GetYSize()/10,
        (uint8_t *)"TOUCH ", LEFT_MODE);
    BSP_LCD_SetTextColor(LCD_COLOR_RED);
    BSP_LCD_DisplayStringAt(BSP_LCD_GetXSize()/2 - 22, BSP_LCD_GetYSize()/10,
        (uint8_t *)"HERE ", LEFT_MODE);
    BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
    BSP_LCD_DisplayStringAt(BSP_LCD_GetXSize()/2 + 10, BSP_LCD_GetYSize()/10,
        (uint8_t *)"TO ", LEFT_MODE);
    BSP_LCD_SetTextColor(LCD_COLOR_RED);
    BSP_LCD_DisplayStringAt(BSP_LCD_GetXSize()/2 + 30, BSP_LCD_GetYSize()/10,
        (uint8_t *)"DROP", LEFT_MODE);
    // kraj pisanja za drop
    
    // pocetak dugmeta pause
    BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
    BSP_LCD_FillCircle(220, 20, 10);
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_DrawCircle(220, 20, 10);
    BSP_LCD_FillRect(215, 15, 5, 10);
    BSP_LCD_FillRect(222, 15, 5, 10);
    // kraj dugmeta
    
    // početak ploče za igranje
    BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
    BSP_LCD_FillRect(20, 45, 195, 150);
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_DrawRect(20, 45, 195, 150);
    // početak crtanja slotova za zetone
    for(int i = 0; i < 6; i++){
        for(int j = 0; j < 7; j++){
            BSP_LCD_SetTextColor(LCD_COLOR_DARKBLUE);
            BSP_LCD_FillCircle (36 + 26.8 * j, 61 + 24*i, 8);
            BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
            BSP_LCD_DrawCircle (36 + 26.8 * j, 61 + 24*i, 8);
        }
    }
    // kraj crtanja ploče
    
    // player1
    BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
    BSP_LCD_FillCircle(15, BSP_LCD_GetYSize() - 30, 5);
    BSP_LCD_FillRect(15 - 3, BSP_LCD_GetYSize()  - 30, 7, 20);
    BSP_LCD_FillRect(15 + 3, BSP_LCD_GetYSize()  - 26, 4, 10);
    BSP_LCD_FillRect(15 - 6, BSP_LCD_GetYSize()  - 26, 4, 10);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_SetBackColor(LCD_COLOR_DARKBLUE);
    BSP_LCD_SetFont(&Font12);
    BSP_LCD_DisplayStringAt(25, BSP_LCD_GetYSize() - 25,
        (uint8_t *)"PLAYER 1", LEFT_MODE);
    // kraj player1
    
    // player2 - racunar
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_FillRect(214, BSP_LCD_GetYSize()  - 15, 15, 3);
    BSP_LCD_SetTextColor(LCD_COLOR_RED);
    BSP_LCD_FillRect(210, BSP_LCD_GetYSize()  - 33, 23, 18);
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_FillRect(212, BSP_LCD_GetYSize()  - 31, 19, 14);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_SetBackColor(LCD_COLOR_DARKBLUE);
    BSP_LCD_SetFont(&Font12);
    BSP_LCD_DisplayStringAt(185, BSP_LCD_GetYSize() - 25,
        (uint8_t *)"BOT", LEFT_MODE);
    // kraj player2
    
    // pocetak strelice koja obiljezava ko igra
    if(pastView == 4)
        player = StateOfSlot::YELLOW;
    if(player == StateOfSlot::YELLOW)
        pointToPlayer(1); // prima int kao parametar, 1 - PLAYER1 ili 2 - PLAYER2
    else pointToPlayer(2);
}

void make2PlayerView(){
    // pozadina i text TOUCH HERE gdje ce korisnik pritisnuti da ubaci zeton
    BSP_LCD_SetTextColor(LCD_COLOR_DARKBLUE);
    BSP_LCD_FillRect(0, 0, 	BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
    BSP_LCD_SetBackColor(LCD_COLOR_DARKBLUE);
    BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
    BSP_LCD_SetFont(&Font12);
    BSP_LCD_DisplayStringAt(BSP_LCD_GetXSize()/2 - 62, BSP_LCD_GetYSize()/10,
        (uint8_t *)"TOUCH ", LEFT_MODE);
    BSP_LCD_SetTextColor(LCD_COLOR_RED);
    BSP_LCD_DisplayStringAt(BSP_LCD_GetXSize()/2 - 22, BSP_LCD_GetYSize()/10,
        (uint8_t *)"HERE ", LEFT_MODE);
    BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
    BSP_LCD_DisplayStringAt(BSP_LCD_GetXSize()/2 + 10, BSP_LCD_GetYSize()/10,
        (uint8_t *)"TO ", LEFT_MODE);
    BSP_LCD_SetTextColor(LCD_COLOR_RED);
    BSP_LCD_DisplayStringAt(BSP_LCD_GetXSize()/2 + 30, BSP_LCD_GetYSize()/10,
        (uint8_t *)"DROP", LEFT_MODE);
    // kraj pisanja za drop
    
    // početak ploče za igranje
    BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
    BSP_LCD_FillRect(20, 45, 195, 150);
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_DrawRect(20, 45, 195, 150);
    // početak crtanja slotova za zetone
    for(int i = 0; i < 6; i++){
        for(int j = 0; j < 7; j++){
            BSP_LCD_SetTextColor(LCD_COLOR_DARKBLUE);
            BSP_LCD_FillCircle (36 + 26.8 * j, 61 + 24*i, 8);
            BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
            BSP_LCD_DrawCircle (36 + 26.8 * j, 61 + 24*i, 8);
        }
    }
    // kraj crtanja ploče

    // pocetak dugmeta pause
    BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
    BSP_LCD_FillCircle(220, 20, 10);
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_DrawCircle(220, 20, 10);
    BSP_LCD_FillRect(215, 15, 5, 10);
    BSP_LCD_FillRect(222, 15, 5, 10);
    // kraj dugmeta
    
    // player1
    BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
    BSP_LCD_FillCircle(15, BSP_LCD_GetYSize() - 30, 5);
    BSP_LCD_FillRect(15 - 3, BSP_LCD_GetYSize()  - 30, 7, 20);
    BSP_LCD_FillRect(15 + 3, BSP_LCD_GetYSize()  - 26, 4, 10);
    BSP_LCD_FillRect(15 - 6, BSP_LCD_GetYSize()  - 26, 4, 10);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_SetBackColor(LCD_COLOR_DARKBLUE);
    BSP_LCD_SetFont(&Font12);
    BSP_LCD_DisplayStringAt(25, BSP_LCD_GetYSize() - 25,
        (uint8_t *)"PLAYER 1", LEFT_MODE);
    // kraj player1
    
    // player2
    BSP_LCD_SetTextColor(LCD_COLOR_RED);
    BSP_LCD_FillCircle(230, BSP_LCD_GetYSize() - 30, 5);
    BSP_LCD_FillRect(230 - 3, BSP_LCD_GetYSize()  - 30, 7, 20);
    BSP_LCD_FillRect(230 + 3, BSP_LCD_GetYSize()  - 26, 4, 10);
    BSP_LCD_FillRect(230 - 6, BSP_LCD_GetYSize()  - 26, 4, 10);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_SetBackColor(LCD_COLOR_DARKBLUE);
    BSP_LCD_SetFont(&Font12);
    BSP_LCD_DisplayStringAt(165, BSP_LCD_GetYSize() - 25,
        (uint8_t *)"PLAYER 2", LEFT_MODE);
    // kraj player2
    if(pastView == 4)
        player = StateOfSlot::YELLOW;
   if(player == StateOfSlot::YELLOW)
        pointToPlayer(1); // prima int kao parametar, 1 - PLAYER1 ili 2 - PLAYER2
    else pointToPlayer(2);
}

void makeHomeView(){
    // pozadina, elipsa i text u njoj
    BSP_LCD_SetTextColor(LCD_COLOR_DARKBLUE);
    BSP_LCD_FillRect(0, 0, 	BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_FillEllipse(BSP_LCD_GetXSize()/2, BSP_LCD_GetYSize()/8 + 5, 53, 23);
    BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
    BSP_LCD_FillEllipse(BSP_LCD_GetXSize()/2, BSP_LCD_GetYSize()/8 + 5, 50, 20);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_SetBackColor(LCD_COLOR_BLUE);
    BSP_LCD_SetFont(&Font12);
    BSP_LCD_DisplayStringAt(BSP_LCD_GetXSize()/2 - 33, BSP_LCD_GetYSize()/8,
        (uint8_t *)"4 IN A ROW", LEFT_MODE);
    // kraj pozadine, elipse i texta u njoj
    
    // prvi covjeculjak
    BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
    BSP_LCD_FillCircle(BSP_LCD_GetXSize()/5, BSP_LCD_GetYSize()/3 + 4, 5);
    BSP_LCD_FillRect(BSP_LCD_GetXSize()/5 - 3, BSP_LCD_GetYSize()/3 + 4, 7, 20);
    BSP_LCD_FillRect(BSP_LCD_GetXSize()/5 + 3, BSP_LCD_GetYSize()/3 + 8, 4, 10);
    BSP_LCD_FillRect(BSP_LCD_GetXSize()/5 - 6, BSP_LCD_GetYSize()/3 + 8, 4, 10);
    // odabir single player
    BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
    BSP_LCD_FillRect(BSP_LCD_GetXSize()/5 - 3 + 40, BSP_LCD_GetYSize()/3, 90, 25);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_DrawRect(BSP_LCD_GetXSize()/5 - 3 + 40, BSP_LCD_GetYSize()/3, 90, 25);
    BSP_LCD_SetBackColor(LCD_COLOR_BLUE);
    BSP_LCD_DisplayStringAt(BSP_LCD_GetXSize()/5 + 50, BSP_LCD_GetYSize()/3 + 7,
        (uint8_t *)"1 PLAYER", LEFT_MODE);
    // druga dva covjeculjka
    BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
    BSP_LCD_FillCircle(BSP_LCD_GetXSize()/5, BSP_LCD_GetYSize()/3 + 64, 5);
    BSP_LCD_FillRect(BSP_LCD_GetXSize()/5 - 3, BSP_LCD_GetYSize()/3 + 64, 7, 20);
    BSP_LCD_FillRect(BSP_LCD_GetXSize()/5 + 3, BSP_LCD_GetYSize()/3 + 68, 4, 10);
    BSP_LCD_FillRect(BSP_LCD_GetXSize()/5 - 6, BSP_LCD_GetYSize()/3 + 68, 4, 10);
    BSP_LCD_SetTextColor(LCD_COLOR_RED);
    BSP_LCD_FillCircle(BSP_LCD_GetXSize()/5 + 7, BSP_LCD_GetYSize()/3 + 64, 5);
    BSP_LCD_FillRect(BSP_LCD_GetXSize()/5 - 3 + 7, BSP_LCD_GetYSize()/3 + 64, 7, 20);
    BSP_LCD_FillRect(BSP_LCD_GetXSize()/5 + 3 + 7, BSP_LCD_GetYSize()/3 + 68, 4, 10);
    BSP_LCD_FillRect(BSP_LCD_GetXSize()/5 - 6 + 7, BSP_LCD_GetYSize()/3 + 68, 4, 10);
    // odabir 2 players
    BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
    BSP_LCD_FillRect(BSP_LCD_GetXSize()/5 - 3 + 40, BSP_LCD_GetYSize()/3 + 60, 90, 25);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_DrawRect(BSP_LCD_GetXSize()/5 - 3 + 40, BSP_LCD_GetYSize()/3 + 60, 90, 25);
    BSP_LCD_SetBackColor(LCD_COLOR_BLUE);
    BSP_LCD_DisplayStringAt(BSP_LCD_GetXSize()/5 + 50, BSP_LCD_GetYSize()/3 + 67,
        (uint8_t *)"2 PLAYERS", LEFT_MODE);
    // kraj odabira 
    
    // potpis
    BSP_LCD_SetBackColor(LCD_COLOR_DARKBLUE);
    BSP_LCD_SetFont(&Font8);
    BSP_LCD_DisplayStringAt(BSP_LCD_GetXSize()/8 , BSP_LCD_GetYSize() - 40,
        (uint8_t *)"University of Electrical Engineering", LEFT_MODE);
    BSP_LCD_DisplayStringAt(BSP_LCD_GetXSize()/8 + 10 , BSP_LCD_GetYSize() - 20,
        (uint8_t *)"Embedded Systems 2020/2021 class", LEFT_MODE);
}

void onPausePressed(){
    // pozadina i izbori za pritisak na dugme pause
    BSP_LCD_SetTextColor(LCD_COLOR_DARKBLUE);
    BSP_LCD_FillRect(0, 0, BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
     BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
    BSP_LCD_FillRect(0, 0, BSP_LCD_GetXSize(), 50);
    BSP_LCD_SetBackColor(LCD_COLOR_BLUE);
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_DisplayStringAt(BSP_LCD_GetXSize()/2 - 55, 20,
        (uint8_t *)"Game paused", LEFT_MODE);
    BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
    BSP_LCD_FillRect(BSP_LCD_GetXSize()/2 - 55, 80, 115, 30);
    BSP_LCD_FillRect(BSP_LCD_GetXSize()/2 - 55, 130, 115, 30);
    BSP_LCD_FillRect(BSP_LCD_GetXSize()/2 - 55, 180, 115, 30);
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_DrawRect(BSP_LCD_GetXSize()/2 - 55, 80, 115, 30);
    BSP_LCD_DrawRect(BSP_LCD_GetXSize()/2 - 55, 130, 115, 30);
    BSP_LCD_DrawRect(BSP_LCD_GetXSize()/2 - 55, 180, 115, 30);
    BSP_LCD_SetBackColor(LCD_COLOR_BLUE);
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_DisplayStringAt(BSP_LCD_GetXSize()/2 - 40 , 87,
        (uint8_t *)"Continue", LEFT_MODE);
    BSP_LCD_DisplayStringAt(BSP_LCD_GetXSize()/2 - 35 , 137,
        (uint8_t *)"Restart", LEFT_MODE);
    BSP_LCD_DisplayStringAt(BSP_LCD_GetXSize()/2 - 20 , 187,
        (uint8_t *)"Home", LEFT_MODE);
    
    
}

bool single_pressed(int x1, int y1){
    if(x1 >BSP_LCD_GetXSize()/5 - 3 + 40 && x1 < BSP_LCD_GetXSize()/5 - 3 + 130 
        && y1 > BSP_LCD_GetYSize()/3 && y1 < BSP_LCD_GetYSize()/3 + 25){
        return true;
    }
    return false;
}

bool multi_pressed(int x1, int y1){
    if(x1 >BSP_LCD_GetXSize()/5 - 3 + 40 && x1 < BSP_LCD_GetXSize()/5 - 3 + 130 && y1 > BSP_LCD_GetYSize()/3 + 60 && y1 < BSP_LCD_GetYSize()/3 + 85)
        return true;
    return false;
}

bool pausePressed(int x1, int y1){
    if(x1 >= 210 && x1 < 230 && y1 >= 10 && y1 <= 30)
        return true;
    return false;
}

bool housePressed(int x1, int y1){
    if(x1 >= 208 && x1 < 232 && y1 >= 8 && y1 <= 32)
        return true;
    return false;
}

bool homePressed(int x1, int y1){
    if(x1 >= BSP_LCD_GetXSize()/2 - 55 && x1 <= BSP_LCD_GetXSize()/2 - 55 + 115 &&
        y1 >= 180 && y1 <= 180 + 30)
        return true;
    return false;
}

bool ContinuePressed(int x1, int y1){
    if(x1 >= BSP_LCD_GetXSize()/2 - 55 && x1 <= BSP_LCD_GetXSize()/2 - 55 + 115 &&
        y1 >= 80 && y1 <= 80 + 30)
        return true;
    return false;
}

bool RestartPressed(int x1, int y1){
    if(x1 >= BSP_LCD_GetXSize()/2 - 55 && x1 <= BSP_LCD_GetXSize()/2 - 55 + 115 &&
        y1 >= 130 && y1 <= 130 + 30)
        return true;
    return false;
}

// funkcija vraca index 0 - 6 kolone u koju korisnik
// hoce da ubaci zeton
int columnInserted(int x1, int y1){
    if(y1 <= 45){ // jer mora pritisnuti a prostor iznad ploce
        if(x1 >= 20 && x1 < 49) return 0;
        else if(x1 >= 49 && x1 < 75) return 1;
        else if(x1 >= 75 && x1 < 101.8) return 2;
        else if(x1 >= 101.8 && x1 < 128.6) return 3;
        else if(x1 >= 128.6 && x1 < 157.2) return 4;
        else if(x1 >= 157.2 && x1 < 184) return 5;
        else if(x1 >= 184 && x1 < 215) return 6;
    }
    return -1;
}

int main() {
    
    // pravim sve slotove i na pocetku su slobodni
    PlayingBoard playingBoard;
    playingBoard.makeTheBoard();
    
    makeHomeView();
   
    if (BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize()) == TS_ERROR) {
        printf("BSP_TS_Init error\n");
    }
    
    // scroll kroz ekrane - ciklicno
     while (1) {
        BSP_TS_GetState(&TS_State);
        if(TS_State.touchDetected) {
            //ako je dodirnut ekran
            //dohvacamo koordinate
            uint16_t x1 = TS_State.touchX[0];
            uint16_t y1 = TS_State.touchY[0];
            if(viewNow == 0){
                if(single_pressed(x1,y1)){
                    BSP_LCD_Clear(LCD_COLOR_DARKBLUE);
                    viewNow = 1;
                    make1PlayerView();
                    pastView = 0;
                }
                else if(multi_pressed(x1,y1)){
                    BSP_LCD_Clear(LCD_COLOR_DARKBLUE);
                    viewNow = 2;
                    make2PlayerView();
                    pastView = 0;
                }
                
            }
            else if(viewNow == 1 || viewNow == 2){
              // znaci da smo na igracem view i moze pauza
              if(pausePressed(x1, y1)){
                BSP_LCD_Clear(LCD_COLOR_DARKBLUE);
                pastView = viewNow;
                viewNow = 3;
                onPausePressed();
              }
              int column = columnInserted(x1, y1);
              if(column >= 0 && column <= 6){
                  // znaci dobro je pritisnuo sada moramo ubaciti zeton
                  int indexInColumn = playingBoard.findFirstAvailableSlot(column);
                  // moramo provjeriti u koji index ubacujemo i onda ga crtamo
                  if(indexInColumn >= 0 && indexInColumn <= 5){
                      playingBoard.insertCoin(column, indexInColumn);
                      if(viewNow == 1){
                          make1PlayerView();
                          playingBoard.updateView();
                          wait(0.2);
                          playingBoard.playBot(); // bot igra
                          if(viewNow == 1){ // mozda pobijedi
                            make1PlayerView();
                            playingBoard.updateView();
                          }
                      }
                      else if(viewNow == 2){
                          make2PlayerView();
                          playingBoard.updateView();
                      }
                  }
              }
            }
            else if(viewNow == 3){
                // sada je na pauzi i moze se vratiti kuci ili na igru
                if(homePressed(x1, y1)){
                  BSP_LCD_Clear(LCD_COLOR_DARKBLUE);
                    pastView = viewNow;
                    viewNow = 0;
                    playingBoard.makeTheBoard();
                    makeHomeView();
              }
               else if(ContinuePressed(x1, y1)){
                  BSP_LCD_Clear(LCD_COLOR_DARKBLUE);
                    viewNow = pastView;
                   if(viewNow == 1)
                        make1PlayerView();
                    else make2PlayerView();
                      playingBoard.updateView();
              }
              else if(RestartPressed(x1, y1)){
                  BSP_LCD_Clear(LCD_COLOR_DARKBLUE);
                    viewNow = pastView;
                    playingBoard.makeTheBoard();
                   if(viewNow == 1)
                        make1PlayerView();
                    else make2PlayerView();
              }
            }
            else if(viewNow == 4){
                if(housePressed(x1,y1)){
                    BSP_LCD_Clear(LCD_COLOR_DARKBLUE);
                    pastView = 4;
                    viewNow = 0;
                    playingBoard.makeTheBoard();
                    makeHomeView();
                }
                
            }
            wait_ms(10);
        }
    }
}