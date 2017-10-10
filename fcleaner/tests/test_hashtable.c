#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../hashtable.h"

int main(int argc, char **argvs) {
    int status;
    hashtable_t *tbl = hsh_init(203, 5);
    assert(tbl != NULL);

    status = hsh_put(tbl, "Node1", strdup("Data1"));
    assert(status == 0);

    status = hsh_put(tbl, "Node1", strdup("Data1"));
    assert(status == -1);           /* Dupplicate node */

    status = hsh_put(tbl, "Node2", strdup("Data2"));
    assert(status == 0);

    char *data = (char *)hsh_search(tbl, "Node1");
    assert(strcmp(data, "Data1") == 0);

    data = (char *)hsh_search(tbl, "Node2");
    assert(strcmp(data, "Data2") == 0);

    data = (char *)hsh_search(tbl, "Node3");
    assert(data == NULL);

    status = hsh_remove(tbl, "Node3");
    assert(status == -1);

    status = hsh_remove(tbl, "Node2");
    assert(status == 0);

    data = (char *)hsh_search(tbl, "Node2");
    assert(data == NULL);

    status = hsh_replace(tbl, "Node2", strdup("Data4"));
    assert(status == -1);

    status = hsh_replace(tbl, "Node1", strdup("Data1_replaced"));
    assert(status == 0);

    data = (char *)hsh_search(tbl, "Node1");
    assert(strcmp(data, "Data1_replaced") == 0);

    hsh_destroy(tbl);
    return 0;
}
