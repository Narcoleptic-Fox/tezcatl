int narrow(int value);
int wide(int value);

int main(void) {
    return narrow(5) + narrow(20) + wide(50) + wide(-500) == 1 ? 0 : 1;
}
