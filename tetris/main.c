// ============================ //
// Do not edit this part!!!!    //
// ============================ //
// 0x300001 - CONFIG1H
#pragma config OSC = HSPLL      // Oscillator Selection bits (HS oscillator,
                                // PLL enabled (Clock Frequency = 4 x FOSC1))
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable bit
                                // (Fail-Safe Clock Monitor disabled)
#pragma config IESO = OFF       // Internal/External Oscillator Switchover bit
                                // (Oscillator Switchover mode disabled)
// 0x300002 - CONFIG2L
#pragma config PWRT = OFF       // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = OFF      // Brown-out Reset Enable bits (Brown-out
                                // Reset disabled in hardware and software)
// 0x300003 - CONFIG1H
#pragma config WDT = OFF        // Watchdog Timer Enable bit
                                // (WDT disabled (control is placed on the SWDTEN bit))
// 0x300004 - CONFIG3L
// 0x300005 - CONFIG3H
#pragma config LPT1OSC = OFF    // Low-Power Timer1 Oscillator Enable bit
                                // (Timer1 configured for higher power operation)
#pragma config MCLRE = ON       // MCLR Pin Enable bit (MCLR pin enabled;
                                // RE3 input pin disabled)
// 0x300006 - CONFIG4L
#pragma config LVP = OFF        // Single-Supply ICSP Enable bit (Single-Supply
                                // ICSP disabled)
#pragma config XINST = OFF      // Extended Instruction Set Enable bit
                                // (Instruction set extension and Indexed
                                // Addressing mode disabled (Legacy mode))

#pragma config DEBUG = OFF      // Disable In-Circuit Debugger

#define KHZ 1000UL
#define MHZ (KHZ * KHZ)
#define _XTAL_FREQ (40UL * MHZ)

// ============================ //
//             End              //
// ============================ //

#include <xc.h>
#include <stdint.h>

#define T_PRELOAD_LOW 0x6A
#define T_PRELOAD_HIGH 0x67
#define NUM_PIECES 3

uint8_t prevB;
uint8_t currentPiece = 0; // Current Tetris piece index
uint8_t posX = 0; // X position of the current piece
uint8_t posY = 0; // Y position of the current piece
uint8_t pieceState = 0; // Rotation state of the current piece
uint8_t pieceTimer = 0; // Timer for piece movement
uint8_t gameArea[8][4] = {0}; // Represent the area(portc-f)
uint8_t gameArea2[8][4] = {0};
uint8_t numberOfBlocks = 0;
uint8_t var,var2;
uint8_t count = 0;
uint8_t flag = 0;
uint8_t m , n;

uint8_t segmentMap[10] = {
        0x3F, // 0 -> Segments ABCDEF
        0x06, // 1 -> Segments BC
        0x5B, // 2 -> Segments ABDEG
        0x4F, // 3 -> Segments ABCDG
        0x66, // 4 -> Segments BCFG
        0x6D, // 5 -> Segments ACDFG
        0x7D, // 6 -> Segments ACDEFG
        0x07, // 7 -> Segments ABC
        0x7F, // 8 -> Segments ABCDEFG
        0x6F  // 9 -> Segments ABCDFG
    };


void Init()
{
    LATG = 0x00; PORTG = 0x00; TRISG = 0xFF;
    // B
    LATB = 0x00; PORTB = 0x00; TRISB = 0xFF;
    // C
    LATC = 0x00; PORTC = 0x00; TRISC = 0x00;
    // D
    LATD = 0x00; PORTD = 0x00; TRISD = 0x00;
    LATE = 0x00; PORTE = 0x00; TRISE = 0x00;
    LATF = 0x00; PORTF = 0x00; TRISF = 0x00;
    LATH = 0x00; PORTH = 0x00; TRISH = 0x00;
    LATJ = 0x00; PORTJ = 0x00; TRISJ = 0x00;
    
}

void InitializeTimerAndInterrupts()
{
    // Enable pre-scalar
    // Full pre-scale
    // we also need to do in-code scaling
    T0CON = 0x00;
    T0CONbits.TMR0ON = 1;
    T0CONbits.T0PS2 = 1;
    T0CONbits.T0PS1 = 0;
    T0CONbits.T0PS0 = 1;
    // Pre-load the value
    TMR0H = T_PRELOAD_HIGH;
    TMR0L = T_PRELOAD_LOW;

    RCONbits.IPEN = 0;
    INTCON = 0x00;
    INTCONbits.TMR0IE = 1;
    INTCONbits.RBIE = 1;
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;
}

int CheckCollision(uint8_t currentPiece,uint8_t posX,uint8_t posY,uint8_t pieceState,uint8_t gameArea[8][4]){
    if(currentPiece == 0){ //Dot
        if(gameArea[posY][posX]==0){
            return 1;
        }
    }
    else if(currentPiece == 1){//Square 
        if(gameArea[posY][posX]==0&&gameArea[posY+1][posX]==0&&
            gameArea[posY][posX+1]==0&&gameArea[posY+1][posX+1]==0){
            
            return 1;
        }
    }
    else{//L
        if(pieceState == 0){
            if(gameArea[posY][posX]==0&&gameArea[posY][posX+1]==0&&gameArea[posY+1][posX+1]==0){
            return 1;
            }
        }
        else if(pieceState ==1){
            if(gameArea[posY+1][posX]==0&&gameArea[posY][posX+1]==0&&gameArea[posY+1][posX+1]==0){
            return 1;
            }
        }
        else if(pieceState ==2){
            if(gameArea[posY+1][posX]==0&&gameArea[posY][posX]==0&&gameArea[posY+1][posX+1]==0){
            return 1;
            }
        }
        else if(pieceState ==3){
            if(gameArea[posY+1][posX]==0&&gameArea[posY][posX+1]==0&&gameArea[posY][posX]==0){
            return 1;
            }
        }
    }
    return 0;
}
int CheckBorder(uint8_t currentPiece,uint8_t posX,uint8_t posY,uint8_t pieceState,uint8_t gameArea[8][4]){
    if(currentPiece == 0){ //Dot
        if(posX>=4 || posY >=8 || posX<  0 ||posY<  0  ){
            return 1;
        }
    }
    else{//Square and L
        if(posX>=3 || posY >=7 || posX<  0 ||posY<  0 ){
            
            return 1;
            }
        
        }
    
    return 0;
}
    

void PlacePiece(uint8_t currentPiece,uint8_t posX,uint8_t posY,uint8_t pieceState,uint8_t gameArea[8][4]){//number of blocks burda m? a?a??da m? updatelensin

    
    if(currentPiece == 0){ //Dot
        gameArea[posY][posX] = 1;
        numberOfBlocks ++;
    }
    else if(currentPiece == 1){//Square 
        gameArea[posY][posX]    = 1;
        gameArea[posY+1][posX]  = 1;
        gameArea[posY][posX+1]  = 1;
        gameArea[posY+1][posX+1]= 1;
        numberOfBlocks = numberOfBlocks + 4;
    }
    else{//L
        if(pieceState == 0){
            gameArea[posY][posX]    = 1;
            gameArea[posY][posX+1]  = 1;
            gameArea[posY+1][posX+1]= 1;
        }
        else if(pieceState ==1){
            gameArea[posY+1][posX]   = 1;
            gameArea[posY][posX+1]   = 1;
            gameArea[posY+1][posX+1] = 1;

        }
        else if(pieceState ==2){
            gameArea[posY+1][posX]   = 1;
            gameArea[posY][posX]     = 1;
            gameArea[posY+1][posX+1] = 1;
        }
        else if(pieceState ==3){
            gameArea[posY+1][posX] = 1;
            gameArea[posY][posX+1] = 1;
            gameArea[posY][posX]   = 1;
        }
        numberOfBlocks = numberOfBlocks + 3;
    }
    
}

void UpdateLEDs(uint8_t currentPiece,uint8_t posX,uint8_t posY,uint8_t pieceState,uint8_t gameArea[8][4]){//Light the alive piece leds
    uint8_t i;
    //gameArea2 = {0};
    for(var = 0; var<8 ; var++){
        for(var2 = 0 ;var2<4 ; var2++){
            gameArea2[var][var2] = gameArea[var][var2];
        }        
        
    }
    if(currentPiece == 0){ //Dot
        gameArea2[posY][posX] = 1;
   
    }
    else if(currentPiece == 1){//Square 
        gameArea2[posY][posX]    = 1;
        gameArea2[posY+1][posX]  = 1;
        gameArea2[posY][posX+1]  = 1;
        gameArea2[posY+1][posX+1]= 1;

    }
    else{//L
        if(pieceState == 0){
            gameArea2[posY][posX]    = 1;
            gameArea2[posY][posX+1]  = 1;
            gameArea2[posY+1][posX+1]= 1;
        }
        else if(pieceState ==1){
            gameArea2[posY+1][posX]   = 1;
            gameArea2[posY][posX+1]   = 1;
            gameArea2[posY+1][posX+1] = 1;

        }
        else if(pieceState ==2){
            gameArea2[posY+1][posX]   = 1;
            gameArea2[posY][posX]     = 1;
            gameArea2[posY+1][posX+1] = 1;
        }
        else if(pieceState ==3){
            gameArea2[posY+1][posX] = 1;
            gameArea2[posY][posX+1] = 1;
            gameArea2[posY][posX]   = 1;
        }

    }

    LATCbits.LC0 = gameArea2[0][0];
    LATCbits.LC1 = gameArea2[1][0];
    LATCbits.LC2 = gameArea2[2][0];
    LATCbits.LC3 = gameArea2[3][0];
    LATCbits.LC4 = gameArea2[4][0];
    LATCbits.LC5 = gameArea2[5][0];
    LATCbits.LC6 = gameArea2[6][0];
    LATCbits.LC7 = gameArea2[7][0];
    
    LATDbits.LD0 = gameArea2[0][1];
    LATDbits.LD1 = gameArea2[1][1];
    LATDbits.LD2 = gameArea2[2][1];
    LATDbits.LD3 = gameArea2[3][1];
    LATDbits.LD4 = gameArea2[4][1];
    LATDbits.LD5 = gameArea2[5][1];
    LATDbits.LD6 = gameArea2[6][1];
    LATDbits.LD7 = gameArea2[7][1];
    
    LATEbits.LE0 = gameArea2[0][2];
    LATEbits.LE1 = gameArea2[1][2];
    LATEbits.LE2 = gameArea2[2][2];
    LATEbits.LE3 = gameArea2[3][2];
    LATEbits.LE4 = gameArea2[4][2];
    LATEbits.LE5 = gameArea2[5][2];
    LATEbits.LE6 = gameArea2[6][2];
    LATEbits.LE7 = gameArea2[7][2];
    
    LATFbits.LF0 = gameArea2[0][3];
    LATFbits.LF1 = gameArea2[1][3];
    LATFbits.LF2 = gameArea2[2][3];
    LATFbits.LF3 = gameArea2[3][3];
    LATFbits.LF4 = gameArea2[4][3];
    LATFbits.LF5 = gameArea2[5][3];
    LATFbits.LF6 = gameArea2[6][3];
    LATFbits.LF7 = gameArea2[7][3];
    __delay_ms(30);

}

void Blink(uint8_t currentPiece,uint8_t posX,uint8_t posY,uint8_t pieceState,uint8_t gameArea2[8][4]){      
        
    if(currentPiece == 0){ //Dot
        gameArea2[posY][posX] =  ~gameArea2[posY][posX];
   
    }
    else if(currentPiece == 1){//Square 
        gameArea2[posY][posX]    = ~gameArea2[posY][posX];
        gameArea2[posY+1][posX]  = ~gameArea2[posY+1][posX];
        gameArea2[posY][posX+1]  = ~gameArea2[posY][posX+1];
        gameArea2[posY+1][posX+1]= ~gameArea2[posY+1][posX+1];

    }
    else{//L
        if(pieceState == 0){
            gameArea2[posY][posX]    = ~gameArea2[posY][posX];
            gameArea2[posY][posX+1]  =  ~gameArea2[posY][posX+1];
            gameArea2[posY+1][posX+1]=  ~gameArea2[posY+1][posX+1];
        }
        else if(pieceState ==1){
            gameArea2[posY+1][posX]   = ~gameArea2[posY+1][posX];
            gameArea2[posY][posX+1]   = ~gameArea2[posY][posX+1] ;
            gameArea2[posY+1][posX+1] = ~gameArea2[posY+1][posX+1];

        }
        else if(pieceState ==2){
            gameArea2[posY+1][posX]   = ~gameArea2[posY+1][posX];
            gameArea2[posY][posX]     = ~gameArea2[posY][posX] ;
            gameArea2[posY+1][posX+1] = ~gameArea2[posY+1][posX+1];
        }
        else if(pieceState ==3){
            gameArea2[posY+1][posX] = ~gameArea2[posY+1][posX];
            gameArea2[posY][posX+1] = ~gameArea2[posY][posX+1];
            gameArea2[posY][posX]   = ~gameArea2[posY][posX] ;
        }

    }
    LATCbits.LC0 = gameArea2[0][0];
    LATCbits.LC1 = gameArea2[1][0];
    LATCbits.LC2 = gameArea2[2][0];
    LATCbits.LC3 = gameArea2[3][0];
    LATCbits.LC4 = gameArea2[4][0];
    LATCbits.LC5 = gameArea2[5][0];
    LATCbits.LC6 = gameArea2[6][0];
    LATCbits.LC7 = gameArea2[7][0];
    
    LATDbits.LD0 = gameArea2[0][1];
    LATDbits.LD1 = gameArea2[1][1];
    LATDbits.LD2 = gameArea2[2][1];
    LATDbits.LD3 = gameArea2[3][1];
    LATDbits.LD4 = gameArea2[4][1];
    LATDbits.LD5 = gameArea2[5][1];
    LATDbits.LD6 = gameArea2[6][1];
    LATDbits.LD7 = gameArea2[7][1];
    
    LATEbits.LE0 = gameArea2[0][2];
    LATEbits.LE1 = gameArea2[1][2];
    LATEbits.LE2 = gameArea2[2][2];
    LATEbits.LE3 = gameArea2[3][2];
    LATEbits.LE4 = gameArea2[4][2];
    LATEbits.LE5 = gameArea2[5][2];
    LATEbits.LE6 = gameArea2[6][2];
    LATEbits.LE7 = gameArea2[7][2];
    
    LATFbits.LF0 = gameArea2[0][3];
    LATFbits.LF1 = gameArea2[1][3];
    LATFbits.LF2 = gameArea2[2][3];
    LATFbits.LF3 = gameArea2[3][3];
    LATFbits.LF4 = gameArea2[4][3];
    LATFbits.LF5 = gameArea2[5][3];
    LATFbits.LF6 = gameArea2[6][3];
    LATFbits.LF7 = gameArea2[7][3];
}
// ============================ //CheckCollision
__interrupt(high_priority)
void HandleInterrupt()
{
    // Timer overflowed (333 ms)
    if(INTCONbits.TMR0IF)
    {
        INTCONbits.TMR0IF = 0;
        // Pre-load the value
        TMR0H = T_PRELOAD_HIGH;
        TMR0L = T_PRELOAD_LOW;
        count++;
        Blink(currentPiece, posX, posY, pieceState,gameArea2);
        if(count == 8){
                    
            posY++;
            // Check for collision
            if (CheckBorder(currentPiece, posX, posY, pieceState,gameArea))
            {
                // If collision, revert the movement and place the piece
                posY--;
            }
            count = 0;
        }

        // Fully lit LATC
        //LATD = ~PORTD;
    }
    if(INTCONbits.RBIF)
    {
        // Read the value to satisfy the interrupt
        if (PORTBbits.RB6 && (prevB< 64 || (prevB>=128 && prevB<192) ))
        {
            // Rotate the piece clockwise
            pieceState = (pieceState + 1) % 4;
            PORTBbits.RB6 = 0;
        }

        // Handle submission via PORTB6
        if (PORTBbits.RB7 && prevB < 128)
        {
            // Check if the current position of the piece is valid for submission
            if (CheckCollision(currentPiece, posX, posY, pieceState,gameArea))
            {
                // Place the piece on the game board
                PlacePiece(currentPiece, posX, posY, pieceState,gameArea);
                
                // Generate a new piece
                currentPiece = (currentPiece + 1) % NUM_PIECES;
                posX = 0;
                posY = 0;
                pieceState = 0;
            }
        }

        // Then clear the bit
        INTCONbits.RBIF = 0;
        prevB = PORTB;

    }
    
}

//if okay return 1 else return 0 
int CheckIfContinue(uint8_t currentPiece,uint8_t pieceState,uint8_t gameArea[8][4]){

if(currentPiece == 0){
for(var = 0 ; var <8 ; var++){
	for( var2=0 ; var2 <4 ; var2++){if(CheckCollision(currentPiece,var2,var,pieceState,currentPiece)) {return 1;}}
}
}
else if (currentPiece == 1) {
for(var = 0 ; var <7 ; var++){
	for( var2=0 ; var2 <3 ; var2++){if(CheckCollision(currentPiece,var2,var,pieceState,currentPiece)) {return 1;}}
}
}

else if (currentPiece == 2) {
for(var = 0 ; var <7 ; var++){
	for( var2=0 ; var2 <3 ; var2++){if(CheckCollision(currentPiece,var2,var,0,currentPiece)) {return 1;}
    if(CheckCollision(currentPiece,var2,var,1,currentPiece)) {return 1;}
    if(CheckCollision(currentPiece,var2,var,2,currentPiece)) {return 1;}
    if(CheckCollision(currentPiece,var2,var,3,currentPiece)) {return 1;}
    }
}
}


return 0;

}

void UpdateSeven(){
    //LATH = 0x08;
    //LATJ = 0x4F;
    if(numberOfBlocks > 9){
        if(flag){
            LATH = 0x08;
            LATJ = segmentMap[numberOfBlocks%10];
            flag = 0;
        }
        else{
            LATH = 0x04;
            LATJ = segmentMap[numberOfBlocks/10];
            flag = 1;
        }
    }
    else{
        if(flag){
            LATH = 0x08;
            LATJ = segmentMap[numberOfBlocks];
            flag = 0;
        }
        else{
            LATH = 0x04;
            LATJ = segmentMap[0];
            flag = 1;
        }
    }

}


__interrupt(low_priority)
void HandleInterrupt2()
{
// ISR ...
}
// ============================ //
//            MAIN              //
// ============================ //
void main()
{
    Init();
    InitializeTimerAndInterrupts();

    prevB = PORTB;
    __delay_ms(100);

    // Game variables
    


    // Main Loop
    while(1)
    {
        // Handle Tetris piece movement every 250ms
        

        // Handle user input
        if (PORTGbits.RG3)
        {
            // Move the piece left
            posX--;
            // Check for collision
            if (CheckBorder(currentPiece, posX, posY, pieceState,gameArea))
            {
                posX++;
            }
            __delay_ms(80);
        }
        else if (PORTGbits.RG0 )
        {
            // Move the piece right
            posX++;
            // Check for collision
            if (CheckBorder(currentPiece, posX, posY, pieceState,gameArea))
            {
                posX--;
            }
            __delay_ms(80);
        }
        else if (PORTGbits.RG4)
        {
            // Move the piece up
            posY--;
            // Check for collision
            if (CheckBorder(currentPiece, posX, posY, pieceState,gameArea))
            {
                posY++;
            }
            __delay_ms(80);
        }
        else if (PORTGbits.RG2)
        {
            // Move the piece down
            posY++;
            // Check for collision
            if (CheckBorder(currentPiece, posX, posY, pieceState,gameArea))
            {
                posY--;
            }
            __delay_ms(80);
        }

        // Handle rotation via PORTB5
        

        // Update LED display for the game area
        UpdateLEDs(currentPiece, posX, posY, pieceState,gameArea);
                    

        UpdateSeven();
        if(!CheckIfContinue(currentPiece,pieceState,gameArea )){
            currentPiece = 0; // Current Tetris piece index
            posX = 0; // X position of the current piece
            posY = 0; // Y position of the current piece
            pieceState = 0; // Rotation state of the current piece
            for(m=0 ; m<8 ; m++){
                for(n=0; n<4; n++){
                    gameArea[m][n] = 0;
                    gameArea2[m][n] = 0;
                    
                }
            }
             // Represent the area(portc-f)
            numberOfBlocks = 0;
            count = 0;
        }
        // Update 7-segment display for occupied block count

        // Delay for 1ms
        // Increment piece timer
    }
}