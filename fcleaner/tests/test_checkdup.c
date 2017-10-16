#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "../fileutil.h"

int main(int argc, char **argv)
{
    int check;

    check = util_checkdup("res/file_first_01", "res/file_second_01");
    assert(check == 0);

    check = util_checkdup("res/file_first_02", "res/file_second_02");
    assert(check == 1);

    check = util_checkdup("res/file_first_03", "res/file_second_03");
    assert(check == 1);
    return 0;
}
