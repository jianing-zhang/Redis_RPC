#include "SkipList.h"

int main(){
    SkipList<std::string,int> sl;
    sl.addItem("a",1);
    sl.addItem("b",2);
    sl.addItem("c",3);
    sl.addItem("d",4);
    sl.addItem("e",5);
    sl.printList();
    sl.deleteItem("a");
    sl.printList();
    std::cout<<sl.size()<<std::endl;
    std::cout<<sl.getCurrentLevel()<<std::endl;
    return 0;
}