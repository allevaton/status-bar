#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* TODO's:
 * script for volume:
 *  amixer get Master | egrep -o "[0-9]+%"
 */

char *print_color(char *c)
{
    if(c == NULL)
        return "^fg()";

    char *fmt = "^fg(#%s)";
    char *s = malloc(24);

    sprintf(s, fmt, c);
    return s;
}

void print_separator(char *s)
{
    // another character to try: 
    sprintf(s, " %s| ", print_color("00FFFF"));
}

void separator(char *s, char *sep)
{
    sprintf(s, "%s%s", s, sep);
}

void readsplit(char **a, FILE *fp, char *sep)
{
    const int size = 256;
    char *s = malloc(size);
    char *buf = malloc(128);
    while((buf = fgets(buf, size, fp)) != NULL)
        sprintf(s, "%s%s", s, buf);

    if(strcmp(s, "") == 0)
    {
        a = NULL;
        return;
    }

    char *lchar = &s[strlen(s)-1];
    if(*lchar == '\n')
        *lchar = '\0';

    if(sep == NULL)
        a = &s;

    char *token;
    int i = 0;
    while((token = strsep(&s, sep)) != NULL)
    {
        a[i] = token;
        i++;
    }

    free(s);
}

void load_conky(char **a)
{
    FILE *conky = popen("conky", "r");
    readsplit(a, conky, "|");
    pclose(conky);
}

void load_acpi(char **a)
{
    FILE *acpi = popen("acpi | awk '{ print $3$4$5 }'", "r");
    readsplit(a, acpi, ",");
    pclose(acpi);
}

// switch funcs {{{
char *switch_wday(int wday)
{
    switch(wday)
    {
        case 0: return "Sunday";
        case 1: return "Monday";
        case 2: return "Tuesday";
        case 3: return "Wednesday";
        case 4: return "Thursday";
        case 5: return "Friday";
        case 6: return "Saturday";
    }

    return "Caturday"; // joke
}

char *switch_month(int mon)
{
    switch(mon)
    {
        case 0: return "January";
        case 1: return "February";
        case 2: return "March";
        case 3: return "April";
        case 4: return "May";
        case 5: return "June";
        case 6: return "July";
        case 7: return "August";
        case 8: return "September";
        case 9: return "October";
        case 10: return "November";
        case 11: return "December";
    }

    return "Time"; // joke
}
// }}}

void module_time(char *s)
{
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    int year = timeinfo->tm_year+1900;
    char *wday = switch_wday(timeinfo->tm_wday);
    char *mon = switch_month(timeinfo->tm_mon);

    //sprintf(s, "%s%s", s, print_color("00FFFF"));
    sprintf(s, "%s:: %s, %s %02d, %d  %02d:%02d:%02d :: ", s,
            wday, mon, timeinfo->tm_mday, year,
            timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
}

void module_cpu_load(char *s, char *conky)
{
    // Add conditionals here
    sprintf(s, "%sCPU %02d%%", s, atoi(conky));
}

void module_cpu_temp(char *s, char *conky)
{
    // Add conditionals here
    sprintf(s, "%s%s°C", s, conky);
}

void module_battery(char *s, char **acpi)
{
    if(*acpi == NULL)
    {
        sprintf(s, "%sNo battery", s);
        return;
    }

    char *status = acpi[0];
    if(strcmp(acpi[0], "Discharging") == 0)
        status = "BAT";
    else if(strcmp(acpi[0], "Charging") == 0)
        status = "CHR";
    else
        status = "ERR";

    sprintf(s, "%s%s %s %s", s, status, acpi[1], acpi[2]);
}

void module_volume(char *s)
{
    char *e = "amixer get Master | egrep -o '([0-9]+%)|(on|off)]'";
    FILE *amixer = popen(e, "r");
    char **a = malloc(4);
    readsplit(a, amixer, "\n");

    int level = atoi(a[0]);
    char *status = a[1];
    a[1][strlen(a[1])-1] = 0; // trim last character

    if(strcmp(status, "on") == 0)
        sprintf(s, "%s♪%02d%%", s, level);
    else
        sprintf(s, "%s♪mm%%", s);

    free(a);
    pclose(amixer);
}

int main(void)
{
    char *s = malloc(16348);
    char *sep = malloc(24);
    print_separator(sep);

    char **conky_array = malloc(256);
    char **acpi_array = malloc(256);

    load_conky(conky_array);
    load_acpi(acpi_array);

    // MODULES-------------------------

    separator(s, sep);
    module_volume(s);
    separator(s, sep);

    module_battery(s, acpi_array);
    separator(s, sep);

    module_cpu_temp(s, conky_array[1]);
    separator(s, sep);

    module_cpu_load(s, conky_array[0]);
    separator(s, sep);

    module_time(s);
    // END MODULES---------------------

    // print result
    puts(s);

    free(s);
    free(sep);
    free(conky_array);
    free(acpi_array);
    return 0;
}
