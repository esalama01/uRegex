#include <iostream>
#include <map>
#include <vector>
#include <stack>
#include <queue>
#include <unordered_map>
#include <tuple>
#include <set>

using namespace std;

void shunting_yard(stack<char> &ostack, queue<char> &oqueue, char character){
    map<char,int> ops;
    ops['|'] = 1;
    ops['?'] = 2;
    ops['*'] = 3;
    if (ostack.empty()){
        ostack.push(character);
    }
    else{
        if (ostack.top() == '(')
        {
            ostack.push(character);
        }
        else if (ops[character] > ops[ostack.top()])
        {
            ostack.push(character);
        }
        else{
            while(!ostack.empty() && ostack.top() != '(' && ops[character] <= ops[ostack.top()]){
                oqueue.push(ostack.top());
                ostack.pop();
            }
            ostack.push(character);
        }
    }
}

void shunting_yard2(stack<char> &ostack, queue<char> &oqueue, char character){
    if (character == '('){
        ostack.push(character);
    }
    else if (character == ')'){
        while((!ostack.empty()) && (ostack.top() != '(')){
            oqueue.push(ostack.top());
            ostack.pop();
        }
        if (ostack.top() == '('){
            ostack.pop();
        }
    }
}

queue<char> shunting(string str){
    int i = 0;
    stack<char> ostack;
    queue<char> oqueue;
    while(str[i] != '\0' && str[i] >= 33 && str[i] <= 126){
        if((str[i] == '|') || (str[i] == '?') || str[i] == '*'){
            shunting_yard(ostack,oqueue,str[i]);
        }
        else if ((str[i] == '(') || (str[i] == ')')){
            shunting_yard2(ostack,oqueue,str[i]);
        }
        else{
            oqueue.push(str[i]);
        }
        i++;
    }
    while(!ostack.empty()){
        oqueue.push(ostack.top());
        ostack.pop();
    }

    return oqueue;
}

class NFA{
    private:
        string istate;
        string fstate;
        static int global_id;
        unordered_map<string,vector<tuple<string,char>>> adjlists;
        string get_id(){
            return to_string(global_id++);
        }
    public:
        NFA(){
            istate = get_id();
            fstate = get_id();
            adjlists[istate] = {};
            adjlists[fstate] = {}; 
        }

        unordered_map<string,vector<tuple<string,char>>>& adjacency_list(){
            return adjlists;
        }

        string input_state(){
            return istate;
        }
        
        string final_state(){
            return fstate;
        }

        void addtransition(string input,string output, char value){
            adjlists[input].push_back(make_tuple(output,value));
        }
        
        void epsilontransition(string input,string output){
            addtransition(input,output,'0');
        }
        
        void symboltransition(string input,string output, char symbol){
            addtransition(input,output,symbol);
        }

        void uniontransition(NFA& nfa1, NFA& nfa2){
            this ->istate = get_id();
            this -> fstate = get_id();
            epsilontransition(this ->istate, nfa1.istate);
            for (const auto& pair : nfa1.adjlists){
                vector<tuple<string,char>> new_vect;
                new_vect = pair.second;
                adjlists[pair.first] = new_vect;
            }
            epsilontransition(nfa1.fstate, this -> fstate);

            epsilontransition(this ->istate, nfa2.istate);
            for (const auto& pair : nfa2.adjlists){
                vector<tuple<string,char>> new_vect;
                new_vect = pair.second;
                adjlists[pair.first] = new_vect;
            }
            epsilontransition(nfa2.fstate, this -> fstate);   
        }
        
        void concatenationtransition(NFA& nfa1, NFA& nfa2){
            this -> istate = nfa1.istate;
            for (const auto& pair: nfa1.adjlists){
                vector<tuple<string,char>> new_vect;
                new_vect = pair.second;
                adjlists[pair.first] = new_vect;
            }
            epsilontransition(nfa1.fstate,nfa2.istate);
            for (const auto& pair: nfa2.adjlists){
                vector<tuple<string,char>> new_vect;
                new_vect = pair.second;
                adjlists[pair.first] = new_vect;
            }
            this -> fstate = nfa2.fstate;
        }
        
        void kleenestartransition(NFA& nfa1){
            this -> istate = get_id();
            this -> fstate = get_id();
            epsilontransition(this -> istate, this -> fstate);
            epsilontransition(this -> istate,nfa1.istate);
            for (const auto& pair: nfa1.adjlists){
                vector<tuple<string,char>> new_vect;
                new_vect = pair.second;
                adjlists[pair.first] = new_vect;
            }
            epsilontransition(nfa1.fstate ,nfa1.istate);
            epsilontransition(nfa1.fstate, this -> fstate);
        }

};

int NFA::global_id = 0;

NFA nfa_conversion(queue<char> iqueue){
    stack<NFA> mystack;
    while(!iqueue.empty()){
        NFA nfa;
        if ((iqueue.front() != '|') && (iqueue.front() != '?') && (iqueue.front() != '*')){
            nfa.symboltransition(nfa.input_state() ,nfa.final_state() ,iqueue.front());
            mystack.push(nfa);
            iqueue.pop();
        }
        else{
            if (iqueue.front() == '|'){
                NFA nfa1 = mystack.top();
                mystack.pop();
                nfa.uniontransition(nfa1, mystack.top());
                mystack.pop();
                mystack.push(nfa);
                iqueue.pop();
            }
            else if (iqueue.front() == '*'){
                nfa.kleenestartransition(mystack.top());
                mystack.pop();
                iqueue.pop();
                mystack.push(nfa);
            }
            else if (iqueue.front() == '?'){
                NFA nfa1 = mystack.top();
                mystack.pop();
                nfa.concatenationtransition(mystack.top(), nfa1);
                mystack.pop();
                mystack.push(nfa);
                iqueue.pop();
            }
        }
    }
    return mystack.top();
}

class matcher{
    private:
        string text;
        NFA nfa;
    public:
        matcher(string text, NFA& nfa){
            this->text = text;
            this ->nfa = nfa;
        }
        bool match(NFA& nfa1, string text, string state, int i, set<string> visited){
            if (visited.count(state)>0){
                return false;
            }
            visited.insert(state);
            if (state == nfa1.final_state() && i == text.length()){
                return true;
            }
            for (const auto& pair : nfa1.adjacency_list()[state]){
                if (get<1>(pair) == '0'){
                    if (match(nfa1, text, get<0>(pair), i, visited)){
                        return true;
                    }
                }
                else if((i < text.length()) && text[i] == get<1>(pair)){
                    set <string> new_visited;
                    if (match(nfa1, text, get<0>(pair), i + 1, new_visited)){
                        return true;
                    }
                }
            }
            return false;
        }

};

int main(){
    string regex;
    string text;

    cout<<"Enter your regex pat: ";
    cin>>regex;
    cout<<"Enter your text: ";
    cin>>text;

    queue<char> postfix = shunting(regex);
    NFA f_nfa = nfa_conversion(postfix);
    matcher engine(text,f_nfa);
    set<string> visited;
    bool result = engine.match(f_nfa, text, f_nfa.input_state(), 0, visited);
    if (result){
        cout<<"Match"<<endl;
    }
    else{
        cout<<"Foff"<<endl;
    }
    return 0;
}
