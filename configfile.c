/**
 * @file    configfile.c
 * @brief   handle configuration file
 *
 * Copyright (C) 2010 Gummi-Dev Team <alexvandermey@gmail.com>
 * All Rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */


#include "configfile.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include "environment.h"
#include "utils.h"

static const gchar* config_filename = 0;

const gchar config_str[] =
"[Editor]\n"
"line_numbers = True\n"
"highlighting = True\n"
"textwrapping = True\n"
"wordwrapping = True\n"
"spelling = False\n"
"toolbar = True\n"
"statusbar = True\n"
"rightpane = True\n"
"spell_language = None\n"
"font = Monospace 10\n"
"\n"
"[File]\n"
"autosaving = False\n"
"autosave_timer = 600\n"
"\n"
"[Compile]\n"
"typesetter = pdflatex\n"
"compile_status = True\n"
"compile_scheme = on_idle\n"
"compile_timer = 1\n"
"idle_threshold = 2000\n"
"\n"
"[Misc]\n"
"recent1 = __NULL__\n"
"recent2 = __NULL__\n"
"recent3 = __NULL__\n"
"welcome = \\documentclass{article}\n"
"	\\begin{document}\n"
"	\\begin{center}\n"
"	\\Huge{Welcome to Gummi} \\\\\\\n"
"	\\\\\n"
"	\\LARGE{You are using the "VERSION" version.\\\\\n"
"	I welcome your suggestions at\\\\\n"
"	http://gummi.midnightcoding.org}\\\\\n"
"	\\end{center}\n"
"	\\end{document}\n\n";

void config_init(const gchar* filename) {
    config_filename = filename;
}

const gchar* config_get_value(const gchar* term) {
    FILE* fh = 0;
    gchar buf[BUF_MAX];
    static gchar ret[BUF_MAX];
    gchar* pstr;

    if (!(fh = fopen(config_filename, "r"))) {
        slog(L_INFO, "can't find configuration file, reseting to default\n");
        config_set_default();
        return config_get_value(term);
    }

    while (!feof(fh)) {
        fgets(buf, BUF_MAX, fh);
        buf[strlen(buf) -1] = 0;
        if (NULL == (pstr = strtok(buf, "[=] ")))
            continue;

        if (0 != strcmp(pstr, term))
            continue;
        if (NULL == (pstr = strtok(NULL, "=")))
            continue;
        else {
            strncpy(ret, pstr + 1, BUF_MAX);
            while (!feof(fh)) {
                fgets(buf, BUF_MAX, fh);
                buf[strlen(buf) -1] = 0;
                if (buf[0] == '\t') {
                    strncat(ret, "\n", BUF_MAX - strlen(ret) -1);
                    strncat(ret, buf + 1, BUF_MAX - strlen(ret) -1);
                } else break;
            }
            break;
        }
    }
    fclose(fh);
    if (0 == strcmp(ret, "False"))
        return NULL;
    return ret;
}

void config_set_value(const gchar* term, const gchar* value) {
    int i = 0, count = 0;
    int max = strlen(value) > BUF_MAX -1? BUF_MAX -1: strlen(value);
    finfo fin = config_load();
    int index = 0;
    gchar buf[BUF_MAX];

    if (-1 == (index = config_find_index_of(fin.pbuf, term)))
        slog(L_FATAL, "invalid configuration\n");

    fin.pbuf[index][strlen(term) + 3] = 0;
    for (i = 0; i < max; ++i) {
        if (count == BUF_MAX -2) break;
        buf[count++] = value[i];
        if (value[i] == '\n')
            buf[count++] = '\t';
    }
    buf[count] = 0;

    strncat(fin.pbuf[index], buf, BUF_MAX - strlen(fin.pbuf[index]) -2);
    strncat(fin.pbuf[index], "\n", BUF_MAX - strlen(fin.pbuf[index]) -1);

    for (i = index + 1; i < fin.len; ++i) {
        if (fin.pbuf[i][0] == '\t')
            fin.pbuf[i][0] = 0;
        else break;
    }

    config_save(fin);

    for (i = 0; i < CONFIG_MAX; ++i)
        g_free(fin.pbuf[i]);
    g_free(fin.pbuf);
}

void config_set_default(void) {
    FILE* fh = 0;
    if (!(fh = fopen(config_filename, "w")))
        slog(L_FATAL, "can't open config for writing... abort\n");

    fwrite(config_str, strlen(config_str), 1, fh);
    fclose(fh);
}

finfo config_load(void) {
    int i = 0, count = 0;
    FILE* fh = 0;

    gchar** pbuf = (gchar**)g_malloc(CONFIG_MAX * sizeof(gchar*));
    for (i = 0; i < CONFIG_MAX; ++i)
        pbuf[i] = (gchar*)g_malloc(BUF_MAX * sizeof(gchar));

    if (!(fh = fopen(config_filename, "r"))) {
        slog(L_INFO, "can't find configuration file, reseting to default\n");
        config_set_default();
        return config_load();
    }

    while (!feof(fh)) {
        if (count == CONFIG_MAX -1)
            slog(L_FATAL, "maximum buffer size reached\n");
        fgets(pbuf[count++], BUF_MAX, fh);
    }
    --count;
    fclose(fh);
    return (finfo){ pbuf, count };
}

void config_save(finfo fin) {
    FILE* fh = 0;
    int i = 0;
    if (!(fh = fopen(config_filename, "w")))
        slog(L_FATAL, "can't open config for writing... abort\n");

    for (i = 0; i < fin.len; ++i) {
        if (strlen(fin.pbuf[i]))
            fputs(fin.pbuf[i], fh);
    }
    fclose(fh);
}

int config_find_index_of(gchar** pbuf, const gchar* term) {
    int i = 0;
    for (i = 0; i < CONFIG_MAX; ++i) {
        if (0 == strncmp(pbuf[i], term, strlen(term)))
            return i;
    }
    return -1;
}
