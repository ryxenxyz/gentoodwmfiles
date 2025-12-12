/* gpu_perc for slstatus (NVIDIA) */

#include <stdio.h>
#include <string.h>
#define MAXLEN 2048
/*
 * slstatus requires:
 *
 *   const char *func(const char *arg)
 *
 * and the returned pointer must stay valid until next call,
 * so we must use static storage.
 */
#include <stdlib.h>

static const char *
ram_used_pretty(const char *unused)
{
    (void)unused;

    static char out[64];

    long total = 0;
    long avail = 0;

    FILE *fp = fopen("/proc/meminfo", "r");
    if (!fp)
        return "RAM ?";

    char line[256];

    while (fgets(line, sizeof line, fp)) {
        if (sscanf(line, "MemTotal: %ld kB", &total) == 1)
            continue;
        if (sscanf(line, "MemAvailable: %ld kB", &avail) == 1)
            continue;
    }

    fclose(fp);

    if (total == 0)
        return "RAM ?";

    long used_kb = total - avail;

    long used_mb = used_kb / 1024;
    long total_gb = total / (1024 * 1024);

    snprintf(out, sizeof out,
             "%ldMiB / %ldGiB",
             used_mb,
             total_gb);

    return out;
}


static const char *
gpu_perc(const char *unused)
{
    (void)unused;

    static char out[32];
    char buf[128];

    FILE *fp = popen("nvidia-smi --query-gpu=utilization.gpu "
    "--format=csv,noheader,nounits", "r");

    if (!fp)
        return "GPU ?";

    if (!fgets(buf, sizeof buf, fp)) {
        pclose(fp);
        return "GPU ?";
    }
    pclose(fp);

    buf[strcspn(buf, "\n")] = 0;

    snprintf(out, sizeof out, " %s", buf);

    return out;
}

/* update interval (ms) */
const unsigned int interval = 1000;

/* fallback text */
static const char unknown_str[] = "n/a";

/* ----------------------------------------------------------
 * Status fields
 * ---------------------------------------------------------- */

static const struct arg args[] = {
    { cpu_perc, " %s%%     ", NULL },
    { gpu_perc, " %s%%     ", NULL },
    { ram_used_pretty, " %s", NULL },

};

