#include <iostream>
#include <chrono>
#include <pthread.h>

#define LEN 3
#define WAIT_UPDATE 2000    // on/off wait time
#define WAIT_PRINT  5000    // time to wait before next auto print



// Global variables
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

std::chrono::steady_clock::time_point lastUpdate = std::chrono::steady_clock::now();

int states[LEN] = {0};
const char names[3] = {'A', 'B', 'C'};



void updateStates(int *input, int len){
    // Looks for the first zero in the array
    // When the zero is found it checks the value of the last element in the array
    //      - if that element is 1 it sets the next element to 0 (sets next state to off)
    //      - if that element is 0 it sets the next element to 1 (sets next state to on)
    // So if the last element is 1 it means it is in the middle of switching all following states
    // off, otherwise if the last element is 0 it means all elements between current and last
    // are off so it starts switching them on
    pthread_mutex_lock(&mutex);
    for(int i = 0; i < len; i++){
        if(input[i] == 0){
            if(input[len - 1] == 0 || i == len - 1){
                input[i] = 1;
            }
            else{
                for(int j = i+1; j < len; j++){
                    if(input[j] == 1){
                        input[j] = 0;
                        break;
                    }
                }
            }
            break;
        }
    }
    pthread_mutex_unlock(&mutex);
}



void print(int *input, int len){
    using namespace std;

    // Remembers the time when the print function was last called 
    pthread_mutex_lock(&mutex);
    lastUpdate = chrono::steady_clock::now();
    pthread_mutex_unlock(&mutex);


    for(int i = 0; i < len; i++){
        cout << names[i] << " ";
        pthread_mutex_lock(&mutex);
        if(input[i]){
            cout << "upaljeno\n";
        }
        else{
            cout << "ugaseno\n";
        }
        pthread_mutex_unlock(&mutex);
    }
    cout << ".......\n\n";
}



void delay(int ms){
    // Runs a while loop for MS amount of milliseconds
    using namespace std::chrono;

    steady_clock::time_point start = steady_clock::now();
    while(duration_cast<milliseconds>( steady_clock::now() - start).count() < ms);
}



void* threadFunction(void* arg){
    // Calls the print function every 5 seconds after it was last called
    using namespace std::chrono;

    while(1){
        bool timeToPrint = false;

        pthread_mutex_lock(&mutex);
        if(duration_cast<milliseconds>(steady_clock::now() - lastUpdate).count() >= WAIT_PRINT){
            timeToPrint = true;
            lastUpdate = steady_clock::now();
        }
        pthread_mutex_unlock(&mutex);

        if(timeToPrint){
            print(states, LEN);
        }
    }
}



int main(){
    using namespace std;

    // Switches all states to on and starts the thread
    print(states, LEN);
    
    for(int i = 0; i < LEN; i++){
        updateStates(states, LEN);
        delay(WAIT_UPDATE);
        print(states, LEN);
    }

    lastUpdate = chrono::steady_clock::now();

    pthread_t constPrint;
    pthread_create(&constPrint, NULL, &threadFunction ,NULL);


    while(1){
        
        int valid = 0;
        char pos;
        cin >> pos;
        
        // Sets the required state to off and saves its location in variable 'valid'
        // Variable 'valid' is also used to check if user input is a valid state
        switch(pos){
            case 'A':{
                valid = 1;
                break;
            }
            case 'B':{
                valid = 2;
                break;
            }
            case 'C':{
                valid = 3;
                break;
            }
            default:{
                cout << "Not a valid position!" << endl;
                break;
            }
        }

        if(valid){
            cout << "Gasenje " << names[valid - 1] << "\n.......\n\n";

            delay(WAIT_UPDATE);

            // Sets the appropriate state to off
            pthread_mutex_lock(&mutex);
            states[valid - 1] = 0;
            pthread_mutex_unlock(&mutex);

            print(states, LEN);

            // Length of the for loop depends on which state is switched off by the user
            //      A - 5 passes
            //      B - 3 passes
            //      C - 1 pass 
            // Loop length is detremined by the formula:
            //      1 + 2 * (3 - state), where state is 1 for A, 2 for B and 3 for C
            for(int i = 0; i < 1 + 2 * (3 - valid); i++){
                delay(WAIT_UPDATE);
                updateStates(states, LEN);
                print(states, LEN);
            }
        }
    }
    return 0;
}
