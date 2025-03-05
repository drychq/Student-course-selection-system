#include "system.h"

//创建主菜单并进入事件循环
void System::userInterface() {
    MainMenu mainMenu;
    while (m_running) {
        mainMenu.display();
        mainMenu.execute(*this);
    }
}
