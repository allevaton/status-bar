#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* TODO's:
 * script for volume:
 *  amixer get Master | egrep -o "[0-9]+%"
 */

#define HOME "/home/nick" // home directory
#define BATTERY_THRESHOLD 10 // when the battery colors turn red
#define CPU_THRESHOLD 60 // degrees in celsius
#define WIRELESS_THRESHOLD 60 // percent

void icon(const char *path, char *ic)
{
    sprintf(ic, "^i(%s/.dzen2/xbm/%s.xbm)", HOME, path);
}

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
    // another character to try, from powerline: 
    sprintf(s, " %s| ^fg()", print_color("555555"));
}

void separator(char *s, char *sep)
{
    sprintf(s, "%s%s", s, sep);
}

char *readsplit(char **a, FILE *fp, char *sep)
{
    const int size = 256;
    char *s = malloc(size);
    char *buf = malloc(128);
    while((buf = fgets(buf, size, fp)) != NULL)
        sprintf(s, "%s%s", s, buf);

    if(strcmp(s, "") == 0)
    {
        a = NULL;
        return "";
    }

    char *lchar = &s[strlen(s)-1];
    if(*lchar == '\n' && *sep != '\n')
        *lchar = '\0';

    if(sep == NULL)
        return s;

    if(strcmp(sep, "\n") == 0)
        puts("kats");

    // special case for the volume, because it's annoying sometimes
    int i = 0;

    if(s[1] == 'm')
    {
        char *vol = malloc(4);
        sprintf(vol, "%c%c", s[6], s[7]);
        a[0] = vol;

        // separate it just to skip one token since we already got it
        strsep(&s, sep);
        i++;
    }

    char *token;
    while((token = strsep(&s, sep)) != NULL)
    {
        a[i] = token;
        i++;
    }

    free(s);
    return "";
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
    int temp = atoi(conky);
    if(temp >= CPU_THRESHOLD)
        sprintf(s, "%s%s", s, print_color("FF0000"));
    else
        sprintf(s, "%s%s", s, print_color(NULL));

    sprintf(s, "%s%d°C", s, temp);
}

void module_battery(char *s, char **acpi)
{
    if(*acpi == NULL)
    {
        sprintf(s, "%sNo battery", s);
        return;
    }

    char *ico = malloc(64);
    char *status = acpi[0];
    int percent = atoi(acpi[1]);

    if(strcmp(acpi[0], "Discharging") == 0)
    {
        icon("bat_full_02", ico);
        status = "BATT";
    }
    else if(strcmp(acpi[0], "Charging") == 0)
    {
        icon("ac", ico);
        status = "CHRG";
    }
    else if(strcmp(acpi[0], "Unknown") == 0)
    {
        icon("ac", ico);
        status = "FULL";
    }

    char *time = malloc(16);
    if(acpi[2] == NULL)
        time = "";
    else
        sprintf(time, " %s", acpi[2]);

    if(strcmp(status, "BATT") == 0 && percent <= BATTERY_THRESHOLD)
    {
        icon("bat_low_02", ico);
        sprintf(s, "%s%s", s, print_color("FF0000"));
    }
    else
        sprintf(s, "%s%s", s, print_color(NULL));

    sprintf(s, "%s%s %02d%% %s%s", s, status, percent, ico, time);
    free(ico);
    free(time);
}

void module_volume(char *s)
{
    char *e = "amixer get Master | egrep -o '([0-9]+%)|(on|off)]'";
    FILE *amixer = popen(e, "r");
    char **a = malloc(128);
    readsplit(a, amixer, "\n");

    int level = atoi(a[0]);
    a[1][strlen(a[1])-1] = 0; // trim last character

    if(strcmp(a[1], "on") == 0)
        sprintf(s, "%s♪%02d%%", s, level);
    else
        sprintf(s, "%s%s♪mm%%", s, print_color("FFFF00"));

    free(a);
    pclose(amixer);
}

void module_wireless(char *s)
{
    char *iw_cmd = "iwconfig wlp3s0 | awk '/Quality/{print $2}' | sed 's/.*=//' | awk -F\"/\" '{printf(\"%.0f%%\", $1/$2*100)}'";
    FILE *iw_out = popen(iw_cmd, "r");

    char *strength = malloc(8);
    strength = readsplit(&strength, iw_out, NULL);

    if(strcmp(strength, "") == 0)
    {
        sprintf(s, "%s%sW: down", s, print_color("FF0000"));
        return;
    }
    else
    {
        char *color;
        int strn = atoi(strength);

        if(strn <= WIRELESS_THRESHOLD)
            color = "FFFF00";
        else
            color = "00FF00";

        sprintf(s, "%s%sW: (%03d%%", s, print_color(color), strn);
    }

    FILE *ssid_out = popen("iwconfig wlp3s0 | egrep -o '\".*\"'", "r");
    char **ssid = malloc(64);
    readsplit(ssid, ssid_out, "\"");

    sprintf(s, "%s at %s)", s, ssid[1]);

    FILE *ip_out = popen("ifconfig wlp3s0 | egrep -o 'inet ([0-9]+\\.?)*'", "r");

    char **ip = malloc(64);
    readsplit(ip, ip_out, " ");
    if(*ip != NULL)
        sprintf(s, "%s %s", s, ip[1]);
    else
        sprintf(s, "%s aquiring...", s);

    pclose(iw_out);
    pclose(ssid_out);
    pclose(ip_out);
    free(strength);
    free(ssid);
    free(ip);
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

    module_wireless(s); separator(s, sep);
    //module_volume(s); separator(s, sep);
    module_battery(s, acpi_array); separator(s, sep);
    module_cpu_temp(s, conky_array[1]); separator(s, sep);
    module_cpu_load(s, conky_array[0]); separator(s, sep);
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
