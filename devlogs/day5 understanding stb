

1.  lets take a look at the example below 
we first get the ascent, descent and lineGap information.
we get that from using the 


                int i,j,ascent,baseline,ch=0;

                ...
                ...
    
    ------->    stbtt_GetFontVMetrics(&font, &ascent,0,0);




lets look at the api

                STBTT_DEF void stbtt_GetFontVMetrics(const stbtt_fontinfo *info, int *ascent, int *descent, int *lineGap);
                // ascent is the coordinate above the baseline the font extends; descent
                // is the coordinate below the baseline the font extends (i.e. it is typically negative)
                // lineGap is the spacing between one row's descent and the next row's ascent...
                // so you should advance the vertical position by "*ascent - *descent + *lineGap"
                //   these are expressed in unscaled coordinates, so you must multiply by
                //   the scale factor for a given size

although not shown here, I would assume you would use lineGap for '\n' characters
here it seems like we are only using ascent. the descent we dont care so we just pass in zero 


-   full code below:

                int main(int arg, char **argv)
                {
                   stbtt_fontinfo font;
                   int i,j,ascent,baseline,ch=0;
                   float scale, xpos=2; // leave a little padding in case the character extends left
                   char *text = "Heljo World!"; // intentionally misspelled to show 'lj' brokenness

                   fread(buffer, 1, 1000000, fopen("c:/windows/fonts/arialbd.ttf", "rb"));
                   stbtt_InitFont(&font, buffer, 0);

                   scale = stbtt_ScaleForPixelHeight(&font, 15);
    ---------->    stbtt_GetFontVMetrics(&font, &ascent,0,0);
                   baseline = (int) (ascent*scale);

                   while (text[ch]) {
                      int advance,lsb,x0,y0,x1,y1;
                      float x_shift = xpos - (float) floor(xpos);
                      stbtt_GetCodepointHMetrics(&font, text[ch], &advance, &lsb);
                      stbtt_GetCodepointBitmapBoxSubpixel(&font, text[ch], scale,scale,x_shift,0, &x0,&y0,&x1,&y1);
                      stbtt_MakeCodepointBitmapSubpixel(&font, &screen[baseline + y0][(int) xpos + x0], x1-x0,y1-y0, 79, scale,scale,x_shift,0, text[ch]);
                      // note that this stomps the old data, so where character boxes overlap (e.g. 'lj') it's wrong
                      // because this API is really for baking character bitmaps into textures. if you want to render
                      // a sequence of characters, you really need to render each bitmap to a temp buffer, then
                      // "alpha blend" that into the working buffer
                      xpos += (advance * scale);
                      if (text[ch+1])
                         xpos += scale*stbtt_GetCodepointKernAdvance(&font, text[ch],text[ch+1]);
                      ++ch;
                   }

                   for (j=0; j < 20; ++j) {
                      for (i=0; i < 78; ++i)
                         putchar(" .:ioVM@"[screen[j][i]>>5]);
                      putchar('\n');
                   }

                   return 0;
                }
                                





2.  now lets take a look at stbtt_GetCodepointBitmapBoxSubpixel();
    the stbtt_GetCodepointBitmapBoxSubpixel(); is exactly the same as stbtt_GetCodepointBitmapBox();
    but with two extra x_shift and y_shift arguments which we will ignore for now.


                STBTT_DEF void stbtt_GetCodepointBitmapBox(const stbtt_fontinfo *font, int codepoint, 
                                                                        float scale_x, float scale_y, 
                                                                        int *ix0, int *iy0, int *ix1, int *iy1);
                // get the bbox of the bitmap centered around the glyph origin; so the
                // bitmap width is ix1-ix0, height is iy1-iy0, and location to place
                // the bitmap top left is (leftSideBearing*scale,iy0).
                // (Note that the bitmap uses y-increases-down, but the shape uses
                // y-increases-up, so CodepointBitmapBox and CodepointBox are inverted.)




3.  then lets take a look at 
    once we get the bounding box, we make our bitmaps 

    notice that we are rendering bitmaps 

                    unsigned char screen[20][79];

                    while (text[ch]) {
                      int advance,lsb,x0,y0,x1,y1;
                      float x_shift = xpos - (float) floor(xpos);
                      stbtt_GetCodepointHMetrics(&font, text[ch], &advance, &lsb);
                      stbtt_GetCodepointBitmapBoxSubpixel(&font, text[ch], scale,scale,x_shift,0, &x0,&y0,&x1,&y1);
    ----------->      stbtt_MakeCodepointBitmapSubpixel(&font, &screen[baseline + y0][(int) xpos + x0], x1-x0,y1-y0, 79, scale,scale,x_shift,0, text[ch]);
                      // note that this stomps the old data, so where character boxes overlap (e.g. 'lj') it's wrong
                      // because this API is really for baking character bitmaps into textures. if you want to render
                      // a sequence of characters, you really need to render each bitmap to a temp buffer, then
                      // "alpha blend" that into the working buffer
                      xpos += (advance * scale);
                      if (text[ch+1])
                         xpos += scale*stbtt_GetCodepointKernAdvance(&font, text[ch],text[ch+1]);
                      ++ch;
                    }




if we render our output one by one,
this is what we see 



                char is H, c_1 (1, -10)         (9, 0)
                char is e, c_1 (0, -8)          (7, 1)
                char is j, c_1 (-1, -10)        (3, 3)


baseline is 12

                'H' x0 y0 (1, -10)  x1 y1 (9, 0)        pos 3,2
                'e' x0 y0 (1, -8)   x1 y1 (8, 1)        pos 12,4
                'j' x0 y0 (-1, -10) x1 y1 (3, 3)        pos 18,2



so bitmaps are rendered from the top left

                MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
                MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
                MMMo.    iiMMMMMMM  o.MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
                MMM@:    VoMMMMMMM  V.MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
                MMM@:    VoM          MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
                MMM@:    VoM :V@V.  @.MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
                MMM@MVVVV@oM.@. :@  @.MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
                MMM@ViiiiMoMiV...M  @.MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
                MMM@:    VoMV@MMMM  @.MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
                MMM@:    VoMoV   .  @.MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
                MMM@:    VoM.@. .@  @.MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
                MMM@:    VoM :M@Mi  @.MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
                MMMMMMMMMMMM        @.MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
                MMMMMMMMMMMMMMMMMM :@ MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
                MMMMMMMMMMMMMMMMMM:V: MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
                MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
                MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
                MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
                MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
                MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM





2.  then for each character we first get more numbers: 


                for (i = 0; i < strlen(word); ++i)
                {
                    /* how wide is this character */
                    int ax;
                    int lsb;
    ----------->    stbtt_GetCodepointHMetrics(&info, word[i], &ax, &lsb);

                    /* get bounding box for character (may be offset to account for chars that dip above or below the line */
                    int c_x1, c_y1, c_x2, c_y2;
                    stbtt_GetCodepointBitmapBox(&info, word[i], scale, scale, &c_x1, &c_y1, &c_x2, &c_y2);
                    
                    /* compute y (different characters have different heights */
                    int y = ascent + c_y1;
                    
                    /* render character (stride and offset is important here) */
                    int byteOffset = x + roundf(lsb * scale) + (y * b_w);
                    stbtt_MakeCodepointBitmap(&info, bitmap + byteOffset, c_x2 - c_x1, c_y2 - c_y1, b_w, scale, scale, word[i]);

                    /* advance x */
                    x += roundf(ax * scale);
                    
                    /* add kerning */
                    int kern;
                    kern = stbtt_GetCodepointKernAdvance(&info, word[i], word[i + 1]);
                    x += roundf(kern * scale);
                }


if you look at the API 
we get the advanceWidth and leftSideBearing


                STBTT_DEF void stbtt_GetCodepointHMetrics(const stbtt_fontinfo *info, int codepoint, int *advanceWidth, int *leftSideBearing);
                // leftSideBearing is the offset from the current horizontal position to the left edge of the character
                // advanceWidth is the offset from the current horizontal position to the next horizontal position
                //   these are expressed in unscaled coordinates


in the example mentioned in stb_truetype.h, it doesnt seem like the demo uses lsb, it only uses advanceWidth to advance the character position




2.  





                STBTT_DEF void stbtt_GetCodepointBitmapBox(const stbtt_fontinfo *font, int codepoint, float scale_x, float scale_y, 
                                                            int *ix0, int *iy0, int *ix1, int *iy1);
                // get the bbox of the bitmap centered around the glyph origin; so the
                // bitmap width is ix1-ix0, height is iy1-iy0, and location to place
                // the bitmap top left is (leftSideBearing*scale,iy0).
                // (Note that the bitmap uses y-increases-down, but the shape uses
                // y-increases-up, so CodepointBitmapBox and CodepointBox are inverted.)


with a 
so when you get the bitmap for a 


                float scale = stbtt_ScaleForPixelHeight(&loadedFont->fontInfo, 25);

                int width, height, xOffset, yOffset;
                unsigned char*  monoBitmap = stbtt_GetCodepointBitmap(&loadedFont->fontInfo, 0, 
                                                                        scale, 'a',
                                                                        &width, &height, &xOffset, &yOffset);

                int c_x1, c_y1, c_x2, c_y2;
                stbtt_GetCodepointBitmapBox(&loadedFont->fontInfo, c, scale, scale, &c_x1, &c_y1, &c_x2, &c_y2);


I get 

c1_xy = (0, -12)
c2_xy = (12, 1)

width is = 12 - 0 = 12, which is consistent with the bitmap below 
height is = 1 - (-12) = 13, which is also consistent with below

                   @@@@@@@
                 @@@@@@@@@@
                 @@@@  @@@@
                 @@@    @@@
                       @@@@
                  @@@@@@@@@
                 @@@@@@@@@@
                 @@@@@  @@@
                @@@@    @@@
                @@@@   @@@@
                 @@@@@@@@@@@
                 @@@@@@@@@@@
                    @@@














2.  
