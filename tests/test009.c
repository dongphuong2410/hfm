#include <stdio.h>
#include <stdlib.h>

#include "trapmngr.h"
#include "config.h"
#include "log.h"

int interrupted = 0;
config_t *config;

int main(int argc, char **argv)
{
    trapmngr_t *tm = tm_init();

    remapped_t *re = (remapped_t *)calloc(1, sizeof(remapped_t));
    re->o = 12345;
    re->r = 45678;

    tm_add_remapped(tm, re);
    remapped_t *foundre = tm_find_remapped(tm, 12345);
    remapped_t *notfoundre = tm_find_remapped(tm, 4343);
    printf("Found remapped %lu %lu\n", foundre->o, foundre->r);
    printf("%s\n", notfoundre ? "NOT NULL" : "NULL");

    uint64_t frame_one = 12;
    uint64_t frame_two = 17;
    uint64_t frame_three = 20;
    uint64_t frame_one_pa_one = (frame_one << 12) + 123;
    uint64_t frame_one_pa_two = (frame_one << 12) + 456;
    uint64_t frame_two_pa_one = (frame_two << 12) + 563;
    uint64_t frame_three_pa_one = (frame_three << 12) + 947;

    trap_t *trap1 = (trap_t *)calloc(1, sizeof(trap_t));
    strcpy(trap1->name, "Trap 1 frame one pa one");
    tm_add_int3trap(tm, frame_one_pa_one, trap1);

    trap_t *trap2 = (trap_t *)calloc(1, sizeof(trap_t));
    strcpy(trap2->name, "Trap 2 frame one pa one");
    tm_add_int3trap(tm, frame_one_pa_one, trap2);

    trap_t *trap3 = (trap_t *)calloc(1, sizeof(trap_t));
    strcpy(trap3->name, "Trap 3 frame one pa two");
    tm_add_int3trap(tm, frame_one_pa_two, trap3);

    trap_t *trap4 = (trap_t *)calloc(1, sizeof(trap_t));
    strcpy(trap4->name, "Trap 4 frame two pa one");
    tm_add_int3trap(tm, frame_two_pa_one, trap4);

    GSList *list;
    list = tm_int3traps_at_pa(tm, frame_one_pa_one);
    printf("We have %d trap(s) at frame_one_pa_one\n", g_slist_length(list));

    list = tm_int3traps_at_pa(tm, frame_one_pa_two);
    printf("we have %d trap(s) at frame_one_pa_two\n", g_slist_length(list));

    list = tm_int3traps_at_pa(tm, frame_two_pa_one);
    printf("we have %d trap(s) at frame_two_pa_one\n", g_slist_length(list));

    list = tm_int3traps_at_pa(tm, frame_three_pa_one);
    printf("we have %d trap(s) at frame_three_pa_one\n", g_slist_length(list));

    list = tm_int3traps_at_gfn(tm, frame_one);
    printf("we have %d trap(s) at frame_one\n", g_slist_length(list));

    list = tm_int3traps_at_gfn(tm, frame_two);
    printf("we have %d trap(s) at frame_two\n", g_slist_length(list));

    list = tm_int3traps_at_gfn(tm, frame_three);
    printf("we have %d trap(s) at frame_three\n", g_slist_length(list));

    tm_destroy(tm);
    return 0;
}
