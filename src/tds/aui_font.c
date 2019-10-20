/**@file
 *	(c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *	All rights reserved
 *
 *	@file				aui_font.c
 *	@brief
 *
 *	@version			1.0
 *	@date				10/10/2013 03:59:52 PM
 *	@revision			none
 *
 *	@author 			Summer Xia <summer.xia@alitech.com>
 */

#include <types.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <osal/osal.h>
#include <aui/aui_font.h>

#include <freetype2/ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_BITMAP_H

#define STATUS_NOINIT 0
#define STATUS_OK	  1

static ft_library	  g_library;
static ft_face		  g_face;
static aui_font_data  g_font;
static int			  g_nb_font;
static int			  g_curfont;
static int			  g_status = STATUS_NOINIT;

/**
 *	function name:			aui_font_init
 *	@brief					true type font initialize
 *
 *	@param[in] p_font_attr	font attribute information
 *
 *	@retval 0				success
 *	@retval !0				fail
 *
 *	@author 				summer xia <summer.xia@alitech.com>
 *	@date					06/19/2013, created
 *
 *	@note
 */
AUI_RTN_CODE aui_font_init(aui_font_attr *p_font_attr)
{
	ft_error error = 0;

	g_status = STATUS_NOINIT;
	memset(&g_font, 0, sizeof(g_font));
	memcpy(&g_font, &p_font_attr->font, sizeof(g_font));

	g_nb_font = p_font_attr->font.nb_of_font;

	g_library = malloc(g_nb_font * sizeof(ft_library));
	g_face = malloc(g_nb_font * sizeof(ft_face));

	g_curfont = g_font.deffont;

	error = ft_init_free_type( &g_library );
	if (error)
	{
		aui_rtn(AUI_MODULE_FONT, FONT_ERR, NULL);
	}

	error = ft_new_memory_face( g_library,
				g_font.fonts[g_curfont].data,
				g_font.fonts[g_curfont].length,
				0,
				&g_face );
	if (error)
	{
		aui_rtn(AUI_MODULE_FONT, FONT_ERR, NULL);
	}

	error = ft_set_pixel_sizes( g_face, g_font.fonts[g_curfont].defsize, 0 );
	if (error)
	{
		aui_rtn(AUI_MODULE_FONT, FONT_ERR, NULL);
	}

	g_status = STATUS_OK;

	return AUI_RTN_SUCCESS;
}

/**
 *	Function Name:		aui_font_de_init
 *	@brief
 *
 *	@param void 		no parameter
 *
 *	@return 			AUI_RTN_CODE
 *
 *	@author 			Summer Xia <summer.xia@alitech.com>
 *	@date				10/11/2013, Created
 *
 *	@note
 */
AUI_RTN_CODE aui_font_de_init(void)
{
	g_status = STATUS_NOINIT;

	ft_done_face(g_face);
	ft_done_free_type(g_library);

	memset(&g_font, 0, sizeof(g_font));
	g_nb_font = 0;
	g_curfont = 0;
	g_library = NULL;
	g_face = NULL;

	return AUI_RTN_SUCCESS;
}

/**
 *	function name:		aui_font_draw_text
 *	@brief				draw text on osd screen
 * *  @param[in] us_unicode  input unicode
 *	@param[in] puc_bitmap_buffer The buffer to store bitmap. If it is
 *						   set to NULL, then only bitmap information
 *						   is stored in pst_bitmap.
 *	@param[out] pst_bitmap bitmap infomation
 *
 *	@return 			AUI_RTN_CODE
 *
 *	@author 			summer xia <summer.xia@alitech.com>
 *	@date				06/19/2013, created
 *
 *	@note				If the width or rows in pst_bitmap is zero,
 *						that means no bitmap is generated from the font
 *						library.
 */
AUI_RTN_CODE aui_font_get_bitmap(unsigned short us_unicode,
				unsigned char *puc_bitmap_buffer,
				unsigned long length,
				aui_bitmap_info *pst_bitmap)
{
	ft_glyph_slot slot;
	ft_error error = 0;
	ft_matrix matrix;
	ft_glyph glyph;
	ft_uint glyph_index;
	ft_bitmap bitmap;
	unsigned long size;
	int i;

	if (g_status == STATUS_NOINIT)
		aui_rtn(AUI_MODULE_FONT, FONT_ERR, NULL);

	slot = g_face->glyph;

	/* set up matrix */
	matrix.xx = (ft_fixed)( 0x10000L );
	matrix.xy = (ft_fixed)( 0x00000L );
	matrix.yx = (ft_fixed)( 0x00000L );
	matrix.yy = (ft_fixed)( 0x10000L );

	/* set transformation */
	ft_set_transform( g_face, &matrix, 0/*&pen*/ );

	glyph_index = ft_get_char_index( g_face, us_unicode );

	error = ft_load_glyph(
			g_face, 			  /* handle to face object */
			glyph_index,		/* glyph index			 */
			FT_LOAD_RENDER/*FT_LOAD_DEFAULT*/ );  /* load flags, see below */
	if (error)
	{
		aui_rtn(AUI_MODULE_FONT, FONT_ERR, NULL);
	}

	error = ft_get_glyph( g_face->glyph, &glyph );
	if (error)
	{
		aui_rtn(AUI_MODULE_FONT, FONT_ERR, NULL);
	}

	error = ft_glyph_to_bitmap(
			&glyph,
			ft_render_mode_normal,
			/*FT_RENDER_MODE_MONO,*/
			0,
			1 );		  /* destroy original image == true */

	bitmap = ((ft_bitmap_glyph)glyph)->bitmap;
	if (pst_bitmap != NULL)
	{
		pst_bitmap->width = bitmap.width;
		pst_bitmap->rows = bitmap.rows;
		pst_bitmap->bitmap_left = slot->bitmap_left;
		pst_bitmap->bitmap_top = slot->bitmap_top;
	}

	if(bitmap.width && bitmap.rows && puc_bitmap_buffer != NULL)
	{
		size = bitmap.width * bitmap.rows;
		if (size > length)
			size = length;
		for(i=0; i<size; i++)
			puc_bitmap_buffer[i] = bitmap.buffer[i];
	}

	ft_done_glyph(glyph);

	return AUI_RTN_SUCCESS;
}


/**
 *	function name:		aui_font_set_font
 *	@brief				select which font to use
 *
 *	@param[in] i_lang	font index
 *
 *	@return 			AUI_RTN_CODE
 *
 *	@author 			summer xia <summer.xia@alitech.com>
 *	@date				06/19/2013, created
 *
 *	@note
 */
AUI_RTN_CODE aui_font_set_font(int i_lang)
{
	ft_error error = 0;

	if (g_status == STATUS_NOINIT)
		aui_rtn(AUI_MODULE_FONT, FONT_ERR, NULL);

	if (i_lang >= g_nb_font)
		aui_rtn(AUI_MODULE_FONT, FONT_ERR, NULL);

	if (i_lang == g_curfont)
		return AUI_RTN_SUCCESS;

	g_status = STATUS_NOINIT;

	ft_done_face(g_face);

	g_curfont = i_lang;

	error = ft_new_memory_face( g_library,
				g_font.fonts[g_curfont].data,
				g_font.fonts[g_curfont].length,
				0,
				&g_face );
	if (error)
	{
		aui_rtn(AUI_MODULE_FONT, FONT_ERR, NULL);
	}

	error = ft_set_pixel_sizes( g_face, g_font.fonts[g_curfont].defsize, 0 );
	if (error)
	{
		aui_rtn(AUI_MODULE_FONT, FONT_ERR, NULL);
	}

	g_status = STATUS_OK;

	return AUI_RTN_SUCCESS;
}

/**
 *	function name:		aui_font_set_size
 *	@brief				set char size
 *
 *	@param[in] i_size	char size
 *
 *	@return 			AUI_RTN_CODE
 *
 *	@author 			summer xia <summer.xia@alitech.com>
 *	@date				06/19/2013, created
 *
 *	@note
 */
AUI_RTN_CODE aui_font_set_size(int i_size)
{
	ft_error error = 0;

	if (g_status == STATUS_NOINIT)
		aui_rtn(AUI_MODULE_FONT, FONT_ERR, NULL);

	error = ft_set_pixel_sizes( g_face, i_size, 0 );
	if (error)
	{
		aui_rtn(AUI_MODULE_FONT, FONT_ERR, NULL);
	}

	return AUI_RTN_SUCCESS;
}

/**
 *	function name:		aui_font_get_def_size
 *	@brief				get default char size
 *
 *	@param[in] i_lang	which font
 *	@param[out] pi_size the char size of the font
 *
 *	@return 			AUI_RTN_CODE
 *
 *	@author 			summer xia <summer.xia@alitech.com>
 *	@date				06/19/2013, created
 *
 *	@note
 */
AUI_RTN_CODE aui_font_get_def_size(int i_lang, int *pi_size)
{
	if (g_status == STATUS_NOINIT)
		aui_rtn(AUI_MODULE_FONT, FONT_ERR, NULL);

	if (i_lang >= g_nb_font)
		aui_rtn(AUI_MODULE_FONT, FONT_ERR, NULL);

	*pi_size = g_font.fonts[i_lang].defsize;

	return AUI_RTN_SUCCESS;
}

/**
 *	function name:		aui_font_get_current_font
 *	@brief				get current used font
 *
 *	@return 			AUI_RTN_CODE
 *
 *	@author 			summer xia <summer.xia@alitech.com>
 *	@date				06/19/2013, created
 *
 *	@note
 */
AUI_RTN_CODE aui_font_get_current_font(int *pi_lang)
{
	if (g_status == STATUS_NOINIT)
		aui_rtn(AUI_MODULE_FONT, FONT_ERR, NULL);

	*pi_lang = g_curfont;

	return AUI_RTN_SUCCESS;
}
