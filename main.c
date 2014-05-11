#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

char *print_color(char *c)
{
    if(c == NULL)
        return "^fg()";

    char *fmt = "^fg(#%s)";
    char *s = malloc(24);

    sprintf(s, fmt, c);
    return s;
}

char *print_separator()
{
    char *s = malloc(16);
    sprintf(s, " %s| ", print_color("CCCCCC"));
    return s;
}

int load_conky( char **a) //{{{
{
    FILE *conky_out = popen("conky", "r");
    if(conky_out == NULL)
    {
        puts("something went wrong with conky");
        return -1;
    }

    const int size = 128;
    char *conky = malloc(size);

    char c = ' ';
    int i = 0;
    while((c = fgetc(conky_out)) != '\n')
    {
        conky[i] = c;
        i++;
    }

    char *token;
    i = 0;
    while((token = strsep(&conky, "|")) != NULL)
    {
        a[i] = token;
        i++;
    }
    pclose(conky_out);
    free(conky);

    return 1;
} // }}}

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
    sprintf(s, "%sCPU %02d%%", s, atoi(conky));
}

void module_cpu_temp(char *s, char *conky)
{
    sprintf(s, "%s%s°C", s, conky);
}

int main(void)
{
    char *s = malloc(16348);
    const char *sep = print_separator();

    char *conky_array[128];
    load_conky(conky_array);

    module_cpu_temp(s, conky_array[1]);
    sprintf(s, "%s%s", s, sep);
    module_cpu_load(s, conky_array[0]);
    sprintf(s, "%s%s", s, sep);
    module_time(s);

    puts(s);

    free(s);
    return 0;
}
