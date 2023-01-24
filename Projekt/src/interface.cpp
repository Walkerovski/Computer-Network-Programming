#include "interface.h"

pair <int, int> interface_read(){
    string input;
    cout<<"Witamy w speedteście! Prosimy o podanie typu operacji: \n1 - DOWNLOAD \n2 - UPLOAD \n";
    cin>>input;
    int type_input = atoi(input.c_str());
    while(type_input > 2 || type_input <= 0){
        cout<<"Podane dane na wejściu są złe! Proszę wybrać jedną z poniższych operacji: \n1 - DOWNLOAD \n2 - UPLOAD \n";
        cin>>input;
        type_input = atoi(input.c_str());
    }
    cout<<"\n";
    cout<<"Prosimy o podanie wielkości pakietu. Wielkość pakietu musi być z przedziału 10 - 1024 (wielkość podana w bajtach)\n";
    cin>>input;
    int size_input = atoi(input.c_str());
    while(size_input > 1024 || size_input < 10){
        cout<<"Podane dane na wejściu są złe! Proszę wpisać wartość z przedziału 10 - 1024 (wielkość podana w bajtach)\n";
        cin>>input;
        size_input = atoi(input.c_str());
    }
    return {type_input, size_input};
}
