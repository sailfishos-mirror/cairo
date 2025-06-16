/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

#ifndef DWRITE_EXTRA_H
#define DWRITE_EXTRA_H

#if defined(__MINGW64_VERSION_MAJOR) && __MINGW64_VERSION_MAJOR < 10
// The DWRITE_COLOR_GLYPH_RUN struct isn't valid.
// <https://sourceforge.net/p/mingw-w64/bugs/913/>
struct DWRITE_COLOR_GLYPH_RUN1_WORKAROUND : DWRITE_COLOR_GLYPH_RUN
{
    DWRITE_GLYPH_IMAGE_FORMATS glyphImageFormat;
    DWRITE_MEASURING_MODE measuringMode;
};
#else
using DWRITE_COLOR_GLYPH_RUN1_WORKAROUND = DWRITE_COLOR_GLYPH_RUN1;
#endif

#if defined(__MINGW64_VERSION_MAJOR) && __MINGW64_VERSION_MAJOR < 11
DEFINE_ENUM_FLAG_OPERATORS(DWRITE_GLYPH_IMAGE_FORMATS);
#endif

#endif /* DWRITE_EXTRA_H */
