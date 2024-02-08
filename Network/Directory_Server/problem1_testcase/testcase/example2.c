#include <stdio.h>

int main() {
    int i, j, rows;
    rows = 8;

    for (i = 1; i <= rows; ++i) {
        // Print spaces before the asterisks
        for (j = 1; j <= rows - i; ++j) {
            printf(" ");
        }

        // Print asterisks
        for (j = 1; j <= 2 * i - 1; ++j) {
            printf("*");
        }

        // Move to the next line after each row
        printf("\n");
    }

    return 0;
}
                                                                                        