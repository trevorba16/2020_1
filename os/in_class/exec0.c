#include <unistd.h>

/* exec example 0 */
int main(){

    execlp ("date", "date", (char *)NULL);

}
