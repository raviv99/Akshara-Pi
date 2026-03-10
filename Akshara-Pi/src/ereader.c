
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <mupdf/fitz.h>
#include "DEV_Config.h"
#include "EPD_5in83_V2.h"

int main(int argc, char **argv) {
    fz_context *ctx = NULL;
    fz_document *doc = NULL;
    fz_pixmap *pix = NULL;
    fz_matrix ctm;
    fz_page *page = NULL;
    fz_rect rect;
    
    int start_page = (argc > 1) ? atoi(argv[1]) - 1 : 0;
    if (start_page < 0) start_page = 0;

    if (DEV_Module_Init() != 0) return -1;
    EPD_5in83_V2_Init();

    ctx = fz_new_context(NULL, NULL, FZ_STORE_DEFAULT);
    fz_register_document_handlers(ctx);
    fz_set_aa_level(ctx, 0); 

    const char *path = "books/stchild.pdf"; 
    fz_try(ctx) {
        doc = fz_open_document(ctx, path);
    }
    fz_catch(ctx) {
        DEV_Module_Exit();
        return 1;
    }
    
    int page_count = fz_count_pages(ctx, doc);
    size_t imagesize = ((EPD_5in83_V2_WIDTH % 8 == 0) ? (EPD_5in83_V2_WIDTH / 8) : (EPD_5in83_V2_WIDTH / 8 + 1)) * EPD_5in83_V2_HEIGHT;
    UBYTE *img_buf = (UBYTE *)malloc(imagesize);

    for (int p = start_page; p < page_count; p++) {
        // Simple, clean status line
        printf("\rReading: Page %d of %d  ", p + 1, page_count);
        fflush(stdout);

        page = fz_load_page(ctx, doc, p);
        rect = fz_bound_page(ctx, page);

        ctm = fz_identity;
        ctm = fz_pre_rotate(ctm, 90);
        float zoom = ((float)EPD_5in83_V2_HEIGHT / (rect.x1 - rect.x0)) * 1.15; 
        ctm = fz_pre_scale(ctm, zoom, zoom);
        ctm = fz_pre_translate(ctm, -35, -10);

        pix = fz_new_pixmap_from_page(ctx, page, ctm, fz_device_gray(ctx), 0);
        memset(img_buf, 0xff, imagesize); 

        for (int y = 0; y < pix->h && y < EPD_5in83_V2_HEIGHT; y++) {
            for (int x = 0; x < pix->w && x < EPD_5in83_V2_WIDTH; x++) {
                if (pix->samples[y * pix->w + x] < 155) { 
                    img_buf[(x + y * EPD_5in83_V2_WIDTH) / 8] &= ~(0x80 >> (x % 8));
                }
            }
        }

        EPD_5in83_V2_Display(img_buf);
        
        fz_drop_pixmap(ctx, pix);
        fz_drop_page(ctx, page);

        if (p < page_count - 1) {
            sleep(8); 
        }
    }

    printf("\nEnd of book.\n");
    EPD_5in83_V2_Sleep();
    
    free(img_buf);
    fz_drop_document(ctx, doc);
    fz_drop_context(ctx);
    DEV_Module_Exit();

    return 0;
}
