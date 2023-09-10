void getCommand(char *cmd, int len){
    __asm__(
        "li $v0, 8\n\t"
        "syscall\n"
    );
}

int isExit(char *cmd){
    if(cmd[0] == 'e' && cmd[1] == 'x' 
    && cmd[2] == 'i' && cmd[3] == 't' 
    && cmd[4] == '\n' && cmd[5] == '\0'){
        return 1;
    }else{
        return 0;
    }
}

void exitProgram(){
    __asm__(
        "li $v0, 10\n\t"
        "syscall\n"
    );
}

void run(char *cmd){
    __asm__(
        "li $v0, 18\n\t"
        "syscall\n"
    );
}

void print(char *cmd){
    __asm__(
        "li $v0, 4\n\t"
        "syscall\n"
    );
}

int main(){
    int exit = 0;
    while(exit == 0){
        print("-->");
        char cmd[256];
        getCommand(cmd, 256 * 4);
        if(isExit(cmd) == 1){
            exit = 1;
        }else{
            run(cmd);
        }
    }  
    exitProgram();
    return 0;
}
