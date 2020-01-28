int main() {
    int cpid, status;
    char *cmd;
    while (1) {
        cmd = readline('$');
        cpid = fork();
        if (cpid == 0) {
            execlp(cmd, cmd);
        }
        wait(&status);
    }
}