/*
 *  genbitmap.c
 *  modified from https://zhuanlan.zhihu.com/p/356995082
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <getopt.h>
#include <ft2build.h>
#include <freetype/ftglyph.h>
#include "charset.h"

enum arg_types {
    arg_font, arg_out, arg_help
};

typedef struct _ft_fontinfo {
    FT_Face face;
    FT_Library library;
    int32_t mono;
} ft_fontinfo;

typedef enum _glyph_format_t {
    GLYPH_FMT_ALPHA, 
    GLYPH_FMT_MONO, 
} glyph_format_t;

typedef struct _glyph_t {
    int16_t x;
    int16_t y;
    uint16_t w;
    uint16_t h;
    uint16_t advance;
    uint8_t format;
    uint8_t pitch;
    uint8_t * data;
    void * handle;
} glyph_t;

uint8_t bitmap_mono_get_pixel(const uint8_t * buff, uint32_t w, uint32_t h, uint32_t x, uint32_t y) {
    uint32_t line_length = ((w + 15) >> 4) << 1;
    uint32_t offset = y * line_length + (x >> 3);
    uint32_t offset_bit = 7 - (x % 8);
    const uint8_t * data = buff + offset;
    if (buff == NULL || (x > w && y > h))
        return 0;
    return (*data >> offset_bit) & 0x1;
}

static int font_ft_get_glyph(ft_fontinfo * font_info, wchar_t c, float font_size, glyph_t * g) {
    FT_Glyph glyph;
    FT_GlyphSlot glyf;
    FT_Int32 flags = FT_LOAD_DEFAULT | FT_LOAD_RENDER ;
    if (font_info->mono)
        flags |= FT_LOAD_TARGET_MONO;
    FT_Set_Char_Size(font_info->face, 0, font_size * 64, 0, 96);
    if (!FT_Load_Char(font_info->face, c, flags)) {
        glyf = font_info->face->glyph;
        FT_Get_Glyph(glyf, &glyph);
        g->format = GLYPH_FMT_ALPHA;
        g->h = glyf->bitmap.rows;
        g->w = glyf->bitmap.width;
        g->pitch = glyf->bitmap.pitch;
        g->x = glyf->bitmap_left;
        g->y = -glyf->bitmap_top;
        g->data = glyf->bitmap.buffer;
        g->advance = glyf->metrics.horiAdvance / 64;
        if (g->data != NULL) {
            if (glyf->bitmap.pixel_mode == FT_PIXEL_MODE_MONO)
                g->format = GLYPH_FMT_MONO;
            g->handle = glyph;
        }
        else
            FT_Done_Glyph(glyph);
    }
    return g->data != NULL || c == ' ' ? 1 : 0;
}

void print_usage() {
    printf("OVERVIEW: a simple font bitmap generator\n");
    printf("USAGE: genbitmap [options]\n");
    printf("OPTIONS: \n");
    printf("  --help/-h\t\tPrint help message\n");
    printf("  --font/-f <file_name>\tUse file <file_name> as font file\n");
    printf("  --out/-o <file_name>\tUse file <file_name> as output file\n");
}

int main(int argc, char ** argv) {
    char fontfilenm[100] = "./font/ArialCE.ttf";
    char outfilenm[100] = "./font/bitmap.txt";
    static struct option long_options[] = {
        {"font", required_argument, NULL, (int)arg_font}, 
        {"out", required_argument, NULL, (int)arg_out}, 
        {"help", no_argument, NULL, (int)arg_help}
    };
    int opt;
    while ((opt = getopt_long(argc, argv, "f:o:h", long_options, NULL)) != EOF) {
        switch (opt) {
        case 'f':
        case (int)arg_font:
            if (!optarg) {
                fprintf(stderr, "Error: Font file is not correctly specified!\n");
                print_usage();
                exit(1);
            }
            if (strlen(optarg) >= 100) {
                fprintf(stderr, "Error: Font file name is too long!\n");
                exit(1);
            }
            strncpy(fontfilenm, optarg, 99);
            break;
        case 'o':
        case (int)arg_out:
            if (!optarg) {
                fprintf(stderr, "Error: Output file is not correctly specified!\n");
                print_usage();
                exit(1);
            }
            if (strlen(optarg) >= 100) {
                fprintf(stderr, "Error: Output file name is too long!\n");
                exit(1);
            }
            strncpy(outfilenm, optarg, 99);
            break;
        case 'h':
        case (int)arg_help:
            print_usage();
            exit(1);
            break;
        default:
            fprintf(stderr, "Error: Wrong argument format!\n");
            print_usage();
            exit(1);
            break;
        }
    }
    ft_fontinfo font_info;
    long int size = 0;
    unsigned char * font_buf = NULL;
    FILE * font_file = fopen(fontfilenm, "rb");
    if (font_file == NULL) {
        fprintf(stderr, "Error: Failed to open font file!\n");
        exit(1);
    }
    fseek(font_file, 0, SEEK_END);
    size = ftell(font_file);
    fseek(font_file, 0, SEEK_SET);
    font_buf = (unsigned char *)calloc(size, sizeof(unsigned char));
    size_t nbytes = fread(font_buf, size, 1, font_file);
    fclose(font_file);
    font_info.mono = 1;
    FT_Init_FreeType(&(font_info.library));
    FT_New_Memory_Face(font_info.library, font_buf, size, 0, &(font_info.face));
    FT_Select_Charmap(font_info.face, FT_ENCODING_UNICODE);
    glyph_t g;
    FILE * outfile = fopen(outfilenm, "wb");
    for (int k = 0; k < N_CHAR; k++) {
        font_ft_get_glyph(&font_info, w_characters[k], FONT_SIZE, &g);
        int i = 0, j = 0;
        if (g.format == GLYPH_FMT_MONO) {
            fprintf(outfile, "%c %d %d\n", characters[k], g.w, g.h);
            int cnt = 0;
            for (j = 0; j < g.h; ++j) {
                for (i = 0; i < g.w; ++i) {
                    uint8_t pixel = bitmap_mono_get_pixel(g.data, g.w, g.h, i, j);
                    if (pixel) {
                        fprintf(outfile, "%d %d ", i, j);
                        cnt++;
                    }
                    if (cnt == 10) {
                        fprintf(outfile, "\n");
                        cnt = 0;
                    }
                }
            }
            fprintf(outfile, "-1 -1\n\n");
        }
    }
    fclose(outfile);
    FT_Done_Glyph(g.handle);
    FT_Done_FreeType(font_info.library);
    free(font_buf);
    printf("Bitmap of font file %s saved to %s.\n", fontfilenm, outfilenm);
    return 0;
}