/*
* To change this license header, choose License Headers in Project Properties.
* To change this template file, choose Tools | Templates
* and open the template in the editor.
*/
/*
* File: main.cpp
* Author: Theo Choma
*
*
*/

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;




//BASE MESSAGE ============================================================

class message{
    protected:
        string msg;
    public:
            message();
            message(string);
            virtual ~message();
            virtual void print();
};


message::message(){
    cout<<"Please input a message to translate: "<<endl;
    cin>>msg;
}


message::message(string input){
    msg = input;
}


message::~message(){
    //cout << "\nBase Destructor" << endl;
}


void message::print(){
    cout <<"in message :: " << msg <<endl;
}



//MORSECODE MESSAGE ========================================================

class morseCodeMessage:public message{
    public:
        string tran_msg;
        int index;
        morseCodeMessage();
        morseCodeMessage(string);
        ~morseCodeMessage();
        void translate(string message1);
        void print();
        void MorseCodeToLights();
};


morseCodeMessage::morseCodeMessage() : message(){
    translate(msg);
}


morseCodeMessage::morseCodeMessage(string input) : message(input){
    msg = input;
    translate(msg);
}


morseCodeMessage::~morseCodeMessage(){
    //cout << "\nMorse Destructor" << endl;
}


void morseCodeMessage::print(){
    cout  <<"in morseCodeMessage :: " << msg << endl;
    cout  <<"in morseCodeMessage :: " << tran_msg << endl;
}


void morseCodeMessage::translate(string message1) {
    //translate english to morse using a morse code table
    string morseTable[] = {".-", "-...", "-.-.", "-..", ".", "..-.",
"--.", "....", "..", ".---", "-.-", ".-..", "--", "-.", "---",
".--.", "--.-", ".-.", "...", "-", "..-", "...-", ".--", "-..-",
"-.--", "--.."};

    char ch;
    string output = "";

    for(unsigned int i=0; i < message1.length(); i++)
    {
        ch = message1[i];
        if(ch == ' '){
            output += "   ";
        }
        else{
            ch = toupper(ch);
            output += morseTable[ch - 'A']; //A is the first value of the array; by subtracting, it's finding the appropriate morse code value for each letter
            output += " ";
        }
    }
    tran_msg = output;
}


void morseCodeMessage::MorseCodeToLights(){

    cout<<tran_msg<<endl; //Start by printing morse code string to terminal

    int fd; // for the file descriptor of the special file we need to open.
    unsigned long *BasePtr; // base pointer, for the beginning of the memory page (mmap)
    unsigned long *PBDR, *PBDDR; // pointers for port B DR/DDR

    fd = open("/dev/mem", O_RDWR|O_SYNC); // open the special file /dem/mem
    if(fd == -1){
        printf("\n error\n");
    }

    // We need to map Address 0x80840000 (beginning of the page)
    BasePtr = (unsigned long*)mmap(NULL,getpagesize(),PROT_READ|PROT_WRITE,MAP_SHARED,fd,0x80840000);
    if(BasePtr == MAP_FAILED){
        printf("\n Unable to map memory space \n");
    } // failed mmap

    // To access other registers in the page, we need to offset the base pointer to reach the
    // corresponding addresses. Those can be found in the board's manual.
    PBDR = BasePtr + 1; // Address of port B DR is 0x80840004
    PBDDR = BasePtr + 5; // Address of port B DDR is 0x80840014

    *PBDDR |= 0xE0; // configures port B5-7 as outputs (Green-Yellow-Red LEDs)
    *PBDDR &= 0xFFFFFFFE; // configures port B0 as input (first push button). You could use: &= ~0x01

    *PBDR &= ~0x20; // OFF: write a 0 to port B5. Mask all other bits. (Red)
    *PBDR &= ~0x80; // OFF: write a 0 to port B7. Mask all other bits. (Green)
    *PBDR &= ~0x40; // OFF: write a 0 to port B6. Mask all other bits. (Yellow)

    int i = 0, j = 0;
    for(i=0; i < msg.length(); i++)
    {
        while(tran_msg[j] != ' ')
        {
            if(tran_msg[j] == '.') // DOT flashes Red LED for 1s then off for 1s
            {
                *PBDR |= 0x20; // ON: write a 1 to port B5. Mask all other bits.
                sleep(1);
                *PBDR &= ~0x20; // OFF: write a 0 to port B5. Mask all other bits.
                sleep(1);
            }
            else if(tran_msg[j] == '-') // DASH flashes Yellow LED for 1s then off for 1s
            {
                *PBDR |= 0x40; // ON: write a 1 to port B6. Mask all other bits.
                sleep(1);
                *PBDR &= ~0x40; // OFF: write a 0 to port B6. Mask all other bits.
                sleep(1);
            }
            j++;
        }
        if(tran_msg[j] == ' ' && tran_msg[j+1] == ' ' && tran_msg[j+2] == ' '){ //If space between words
            *PBDR |= 0x80; // ON: write a 1 to port B7. Mask all other bits.
            sleep(1);
            *PBDR &= ~0x80; // OFF: write a 0 to port B7. Mask all other bits.
            sleep(1);
            j = j + 3;
        }
        else{ //If space between letters
            j++;
            sleep(2); // SPACE keeps all LEDs off for 2s
        }

    }
    //End of Message flashes Green LED for 2s then off for 1s
    *PBDR |= 0x80; // ON: write a 1 to port B7. Mask all other bits.
    sleep(2);
    *PBDR &= ~0x80; // OFF: write a 0 to port B7. Mask all other bits.
    sleep(1);


}



//MESSAGE STACK ============================================================

class messageStack{
    public:
        message *ptrstack[10];
        messageStack(message);
        ~messageStack();
        void push(message *current_obj);
        void pop();
        void printStack();
        int stack_top;
        int numobj;
};


messageStack::messageStack(message init){
    //cout<<"stack constructor"<<endl;
    //ptrstack[0] = new message(init);
    ptrstack[0] = &init;
    numobj= 1;
    stack_top = 1;
}


messageStack::~messageStack(){
    //cout << "\nStack Destructor" << endl;
    for(int i = 0; i < numobj; i++)
    {
        delete ptrstack[i];
    }
}


void messageStack::push(message* current_obj){
   // current_obj->print();
    ptrstack[stack_top] = current_obj;
   // ptrstack[stack_top]->print();
    numobj++;
    stack_top++;
}


void messageStack::pop(){
    //cout<<"begin popping"<<endl;
    delete ptrstack[stack_top-1];
    stack_top--;
    numobj--;
}


void messageStack::printStack(){
    //print all messages from top to bottom of stack
    //cout<<"print stack fn"<< endl;
    for(int i = 0; i < numobj; i++)
    {
        cout<<"Message no. " << i <<endl;
        ptrstack[i]->print();
    }
}



//MAIN FUNCTION ============================================================

int main(int argc, char** argv) {
    message msg1("test message");
       // msg1.print();
    morseCodeMessage msg2("Test morseCodeMessage");
       // msg2.print();
        //msg2.MorseCodeToLights();
    message msg3("Test2 message");
      //  msg3.print();
    cout<<endl;


    messageStack stack(msg1);
    stack.push(&msg2);
    stack.push(&msg3);
    stack.printStack();

    stack.pop();
    cout<<endl;

    stack.printStack();
    cout<<endl<<"exit"<<endl;

    return 0;
}
