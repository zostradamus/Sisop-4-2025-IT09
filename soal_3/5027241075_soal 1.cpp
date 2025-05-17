#include <iostream>
#include <string>
using namespace std;

class Stack
{
private:
    struct StackNode
    {
        char data;
        StackNode* next;
    };
    StackNode* topNode;

public:
    Stack()
    {
        topNode = 0;
    }

    ~Stack()
    {
        while (topNode){
            StackNode* temp = topNode;
            topNode = topNode->next;
            delete temp;
        }
    }

    bool isEmpty() const
    {
        return topNode == 0;
    }

    // unsigned size() const
    // {
    //     return count;
    // }

    void push(char v)
    {
        StackNode* n = new StackNode;
        n ->data = v;
        n ->next = topNode;
        topNode = n;
    }

//     void pop()
//     {
//         if(!isEmpty()){
//             StackNode* temp = topNode->data;
//             return 0;
//         }
//     }

//     char top() const
//     {
//         if(!isEmpty()) return topNode->data;
//         return 0;
//     }
// };

bool popMatch(char v_close){
    char want = 0;
    if(v_close  == ')')want = '(';
    if(v_close == '}')want= '{';
    if(v_close == ']')want = '[';
    if (!want)return false;
    if (!topNode) return false;
    if (topNode -> data == want){
        StackNode* t = topNode;
        topNode = topNode->next;
        delete t;
        return true;
    }
    StackNode* prev = topNode;
    StackNode* cur = topNode->next;
        while (cur){
            if(cur->data == want){
                prev->next = cur ->next;
                delete cur;
                return true;
            }
            prev = cur;
            cur = cur->next;
        }
        return false;
    }
};

int main()
{
   string s;
    getline(cin,s);
    Stack st;
    int closed=0;
    for(size_t i = 0; i < s.length();i++){
        char c = s[i];
        if(c == '(' || c == '{'||c=='['){
            st.push(c);
        }else if(c == ')' || c == '}'|| c == ']'){
            if(st.popMatch(c)) closed++;
            }
        }
    if(st.isEmpty()){
        cout<< closed;
    }else{
        cout<< "LUPA NUTUP" ;
    }
    return 0;
}