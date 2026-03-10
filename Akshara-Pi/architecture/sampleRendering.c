// Simplified MuPDF Render Flow
fz_context *ctx = fz_new_context(NULL, NULL, FZ_STORE_DEFAULT);
fz_document *doc = fz_open_document(ctx, path_to_book);
fz_page *page = fz_load_page(ctx, doc, page_number);

/* Define the scale to match the 5.76" E-ink dimensions */
fz_matrix ctm = fz_scale(zoom, zoom); 
fz_pixmap *pix = fz_new_pixmap_from_page(ctx, page, ctm, fz_device_rgb(ctx), 0);

/* Akshara-Pi logic: Convert RGB to 1-bit or 4-bit Grayscale for E-ink */
process_for_eink(pix->samples);
