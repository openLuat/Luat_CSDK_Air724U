#include "../../lvgl.h"

#include "ucs2_to_gb2312_table.h"
#include "ucs2_to_gb2312_offset.h"

const uint8_t number_of_bit_1[256] = 
{
    0x00, 0x01, 0x01, 0x02, 0x01, 0x02, 0x02, 0x03,
    0x01, 0x02, 0x02, 0x03, 0x02, 0x03, 0x03, 0x04,
    0x01, 0x02, 0x02, 0x03, 0x02, 0x03, 0x03, 0x04,
    0x02, 0x03, 0x03, 0x04, 0x03, 0x04, 0x04, 0x05,
    0x01, 0x02, 0x02, 0x03, 0x02, 0x03, 0x03, 0x04,
    0x02, 0x03, 0x03, 0x04, 0x03, 0x04, 0x04, 0x05,
    0x02, 0x03, 0x03, 0x04, 0x03, 0x04, 0x04, 0x05,
    0x03, 0x04, 0x04, 0x05, 0x04, 0x05, 0x05, 0x06,
    0x01, 0x02, 0x02, 0x03, 0x02, 0x03, 0x03, 0x04,
    0x02, 0x03, 0x03, 0x04, 0x03, 0x04, 0x04, 0x05,
    0x02, 0x03, 0x03, 0x04, 0x03, 0x04, 0x04, 0x05,
    0x03, 0x04, 0x04, 0x05, 0x04, 0x05, 0x05, 0x06,
    0x02, 0x03, 0x03, 0x04, 0x03, 0x04, 0x04, 0x05,
    0x03, 0x04, 0x04, 0x05, 0x04, 0x05, 0x05, 0x06,
    0x03, 0x04, 0x04, 0x05, 0x04, 0x05, 0x05, 0x06,
    0x04, 0x05, 0x05, 0x06, 0x05, 0x06, 0x06, 0x07,
    0x01, 0x02, 0x02, 0x03, 0x02, 0x03, 0x03, 0x04,
    0x02, 0x03, 0x03, 0x04, 0x03, 0x04, 0x04, 0x05,
    0x02, 0x03, 0x03, 0x04, 0x03, 0x04, 0x04, 0x05,
    0x03, 0x04, 0x04, 0x05, 0x04, 0x05, 0x05, 0x06,
    0x02, 0x03, 0x03, 0x04, 0x03, 0x04, 0x04, 0x05,
    0x03, 0x04, 0x04, 0x05, 0x04, 0x05, 0x05, 0x06,
    0x03, 0x04, 0x04, 0x05, 0x04, 0x05, 0x05, 0x06,
    0x04, 0x05, 0x05, 0x06, 0x05, 0x06, 0x06, 0x07,
    0x02, 0x03, 0x03, 0x04, 0x03, 0x04, 0x04, 0x05,
    0x03, 0x04, 0x04, 0x05, 0x04, 0x05, 0x05, 0x06,
    0x03, 0x04, 0x04, 0x05, 0x04, 0x05, 0x05, 0x06,
    0x04, 0x05, 0x05, 0x06, 0x05, 0x06, 0x06, 0x07,
    0x03, 0x04, 0x04, 0x05, 0x04, 0x05, 0x05, 0x06, 
    0x04, 0x05, 0x05, 0x06, 0x05, 0x06, 0x06, 0x07,
    0x04, 0x05, 0x05, 0x06, 0x05, 0x06, 0x06, 0x07,
    0x05, 0x06, 0x06, 0x07, 0x06, 0x07, 0x07, 0x08,
};

#define HZ_FONT_WIDTH  (16)
#define HZ_FONT_HEIGHT (16)
#define ASC_FONT_WIDTH  (8)
#define ASC_FONT_HEIGHT (16)

#define FONT_SIZE(WIDTH, HEIGHT)   (((WIDTH) + 7) / 8 * (HEIGHT))

#define HZ_FONT_SIZE FONT_SIZE(HZ_FONT_WIDTH, HZ_FONT_HEIGHT)
#define ASC_FONT_SIZE FONT_SIZE(ASC_FONT_WIDTH, ASC_FONT_HEIGHT)

#define ASC_FONT_FILE_NAME "font.dat"
#define HZ_FONT_FILE_NAME "fonthzTwoLevel.dat"
#define HZ_EXT_FONT_FILE_NAME "fonthzext.dat"


typedef struct FontInfoTag
{
    uint8_t        width;
    uint8_t        height;
    uint8_t        size;
    uint16_t       start;
    uint16_t       end;
    const uint8_t *data;
} FontInfo;

static const uint8_t blankChar[HZ_FONT_SIZE] = {0};

// font 
// 宋体16 ascii 0x20~0x7e
static const uint8_t sansFont16Data[]=
{
    #include ASC_FONT_FILE_NAME
};

static const FontInfo sansFont16 = 
{
    ASC_FONT_WIDTH,
    ASC_FONT_HEIGHT,
    ASC_FONT_SIZE,
    0x20,
    0x7E,
    sansFont16Data,
};

static const uint8_t sansHzFont16Data[] =
{
    #include HZ_FONT_FILE_NAME
};

/*+\NEW\liweiqiang\2013.12.18\增加中文标点符号的显示支持 */
static const uint8_t sansHzFont16ExtData[] = 
{
    #include HZ_EXT_FONT_FILE_NAME
};
//[B0A1,F7FE] 不包含D7FA-D7FE
/*按内码由小到大排列*/
static const uint16_t sansHzFont16ExtOffset[] =
{
//"、。—…‘’“”〔〕〈〉《》「」『』【】！（），－．：；？嗯"
    0xA1A2,0xA1A3,0xA1AA,0xA1AD,0xA1AE,0xA1AF,0xA1B0,0xA1B1,
    0xA1B2,0xA1B3,0xA1B4,0xA1B5,0xA1B6,0xA1B7,0xA1B8,0xA1B9,
    0xA1BA,0xA1BB,0xA1BE,0xA1BF,0xA3A1,0xA3A8,0xA3A9,0xA3AC,
    0xA3AD,0xA3AE,0xA3BA,0xA3BB,0xA3BF,0xE0C5
};

static FontInfo sansHzFont16 =
{
    HZ_FONT_WIDTH,
    HZ_FONT_HEIGHT,
    HZ_FONT_SIZE,
    0,
    0,
    sansHzFont16Data
};

typedef struct DispBitmapTag
{
    uint16_t width;
    uint16_t height;	
    uint8_t bpp;
    const uint8_t *data;
} DispBitmap;

static void getHzBitmap(DispBitmap *pBitmap, uint16_t charcode);
static void getFontBitmap(DispBitmap *pBitmap, uint16_t charcode);

static void getCharBitmap(DispBitmap *pBitmap, uint16_t charcode)
{
    if(charcode >= 0x80A0)
    {
        getHzBitmap(pBitmap, charcode);
    }
    else
    {
        getFontBitmap(pBitmap, charcode);
    }
}

static void getHzBitmap(DispBitmap *pBitmap, uint16_t charcode)
{
    const FontInfo *pInfo = &sansHzFont16;

    pBitmap->bpp = 1;
    pBitmap->width = pInfo->width;
    pBitmap->height = pInfo->height;

    if(pInfo->data)
    {
        uint8_t byte1, byte2;
        uint32_t index;

        byte1 = charcode>>8;
        byte2 = charcode&0x00ff;

        if(byte1 >= 0xB0 && byte1 <= 0xF7 &&
            byte2 >= 0xA1 && byte2 <= 0xFE)
        {
            index = (byte1 - 0xB0)*(0xFE - 0xA1 + 1) + byte2 - 0xA1;
            pBitmap->data = pInfo->data + index*pInfo->size;
        }
        else
        {
            pBitmap->data = blankChar;
            for(index = 0; index < sizeof(sansHzFont16ExtOffset)/sizeof(uint16_t); index++)
            {
                if(charcode < sansHzFont16ExtOffset[index])
                {
                    break;
                }

                if(charcode == sansHzFont16ExtOffset[index])
                {
                    pBitmap->data = sansHzFont16ExtData + index*pInfo->size;
                    break;
                }
            }
        }
    }
    else
    {
        pBitmap->data = blankChar;
    }
}

static void getFontBitmap(DispBitmap *pBitmap, uint16_t charcode)
{
    const FontInfo *pInfo = &sansFont16;

    pBitmap->bpp = 1;
    pBitmap->width = pInfo->width;
    pBitmap->height = pInfo->height;

    if(pInfo->data)
    {
        if(charcode >= pInfo->start && charcode <= pInfo->end)
        {
            uint32_t index = charcode - pInfo->start;

            pBitmap->data = pInfo->data + index*pInfo->size;            
        }
        else
        {
            pBitmap->data = blankChar;
        }
    }
    else
    {
        pBitmap->data = blankChar;
    }
}

static uint16_t get_ucs2_offset(uint16_t ucs2)
{
    uint16_t   offset, page, tmp;
    uint8_t    *mirror_ptr, ch;

    page = (ucs2>>8) - 0x4E;
    ucs2 &= 0xFF;

    tmp        = ucs2>>6; /* now 0 <= tmp < 4  */ 
    offset     = ucs2_index_table_4E00_9FFF[page][tmp];  
    mirror_ptr = (uint8_t*)&ucs2_mirror_4E00_9FFF[page][tmp<<3]; /* [0, 8, 16, 24] */ 

    tmp = ucs2&0x3F; /* mod 64 */ 

    while(tmp >= 8)
    {
        offset += number_of_bit_1[*mirror_ptr];
        mirror_ptr++;
        tmp -= 8;
    }

    ch = *mirror_ptr;
    if(ch&(0x1<<tmp))
    {   /* Ok , this ucs2 can be covert to GB2312. */ 
        while(tmp) 
        { 
            if(ch&0x1)
            offset++;
            ch>>=1;
            tmp--;
        }
        return offset;
    }

    return (uint16_t)(-1);
}

uint16_t unicode_to_gb2312(uint16_t ucs2)
{
	uint16_t gb = 0xA1A1;
	if(0x80 > ucs2)
    {
        // can be convert to ASCII char
        gb = ucs2;
    }
    else
    {
        if((0x4E00 <= ucs2) && (0xA000 > ucs2))
        {
            uint16_t offset = get_ucs2_offset(ucs2);
            if((uint16_t)(-1) != offset)
            {
                gb = ucs2_to_gb2312_table[offset];
            }
        }
        else
        {
            uint16_t uint16_tcount = sizeof(tab_UCS2_to_GBK)/4;
            for(uint16_t ui=0; ui < uint16_tcount; ui++)
            {
                if(ucs2 == tab_UCS2_to_GBK[ui][0])
                {
                    gb = tab_UCS2_to_GBK[ui][1];
                }
            }
                
        }
    }
	return gb;
}

extern lv_font_t lv_font_roboto_16;

typedef struct
{
	int32_t code;
	DispBitmap bitmap;
} platform_bitmap_cache_t;

static platform_bitmap_cache_t g_s_cache = 
{
	.code = -1,
	.bitmap = {0}
};

static uint16_t platform_unicode_to_gb2312(uint32_t unicode)
{
	uint16_t ucs2 = unicode;
	uint16_t gb = unicode_to_gb2312(ucs2);
	// osiTracePrintf(0, "%s code %x", __FUNCTION__, gb);
	return gb;
}

static bool platform_get_bitmap(const lv_font_t *font, DispBitmap *bitmap, uint16_t code)
{
	// osiTracePrintf(0, "%s code %x", __FUNCTION__, code);
	platform_bitmap_cache_t *cache = (platform_bitmap_cache_t *)font->dsc;
	if (cache->code == code)
	{
		*bitmap = cache->bitmap;
		return true;
	}
	getCharBitmap(bitmap, code);
	if (bitmap->data != blankChar)
	{
		cache->code = code;
		cache->bitmap = *bitmap;
		return true;
	}
	return false;
}

static bool platform_get_glyph_dsc(const lv_font_t *font, lv_font_glyph_dsc_t *out, uint32_t letter, uint32_t letter_next)
{
	DispBitmap bitmap;
	if (platform_get_bitmap(font, &bitmap, platform_unicode_to_gb2312(letter)))
	{
		out->bpp = bitmap.bpp;
		out->adv_w = bitmap.width;
		out->box_h = bitmap.height;
		out->box_w = bitmap.width;
		out->ofs_x = 0;
		out->ofs_y = 0;
		return true;
	}
	return lv_font_roboto_16.get_glyph_dsc(&lv_font_roboto_16, out, letter, letter_next);
}

static const uint8_t *platform_get_glyph_bitmap(const struct lv_font_t *font, uint32_t unicode_letter)
{
	DispBitmap bitmap;
	if (platform_get_bitmap(font, &bitmap, platform_unicode_to_gb2312(unicode_letter)))
	{
		return bitmap.data;
	}
	return lv_font_roboto_16.get_glyph_bitmap(&lv_font_roboto_16, unicode_letter);
}

lv_font_t lv_font_hz = {
    .dsc = &g_s_cache,
    .get_glyph_bitmap = platform_get_glyph_bitmap,
    .get_glyph_dsc = platform_get_glyph_dsc,
    .line_height = HZ_FONT_HEIGHT,
    .base_line = 0,
};