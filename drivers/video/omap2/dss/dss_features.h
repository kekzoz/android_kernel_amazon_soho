/*
 * linux/drivers/video/omap2/dss/dss_features.h
 *
 * Copyright (C) 2010 Texas Instruments
 * Author: Archit Taneja <archit@ti.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __OMAP2_DSS_FEATURES_H
#define __OMAP2_DSS_FEATURES_H

#if defined(CONFIG_OMAP4_DSS_HDMI) || defined(CONFIG_OMAP5_DSS_HDMI)
#include "ti_hdmi.h"
#endif

#define MAX_DSS_MANAGERS	3
#define MAX_DSS_OVERLAYS	4
#define MAX_DSS_LCD_MANAGERS	2
#define MAX_NUM_DSI		2

/* DSS has feature id */
enum dss_feat_id {
	FEAT_LCDENABLEPOL,
	FEAT_LCDENABLESIGNAL,
	FEAT_PCKFREEENABLE,
	FEAT_FUNCGATED,
	FEAT_MGR_LCD2,
	FEAT_LINEBUFFERSPLIT,
	FEAT_ROWREPEATENABLE,
	FEAT_RESIZECONF,
	/* Independent core clk divider */
	FEAT_CORE_CLK_DIV,
	FEAT_LCD_CLK_SRC,
	/* DSI-PLL power command 0x3 is not working */
	FEAT_DSI_PLL_PWR_BUG,
	FEAT_DSI_PLL_FREQSEL,
	FEAT_DSI_DCS_CMD_CONFIG_VC,
	FEAT_DSI_VC_OCP_WIDTH,
	FEAT_DSI_REVERSE_TXCLKESC,
	FEAT_DSI_GNQ,
	FEAT_HDMI_CTS_SWMODE,
	FEAT_HDMI_AUDIO_USE_MCLK,
	FEAT_HANDLE_UV_SEPARATE,
	FEAT_ATTR2,
	FEAT_VENC_REQUIRES_TV_DAC_CLK,
	FEAT_CPR,
	FEAT_PRELOAD,
	FEAT_FIR_COEF_V,
	FEAT_ALPHA_FIXED_ZORDER,
	FEAT_ALPHA_FREE_ZORDER,
	FEAT_FIFO_MERGE,
	/* An unknown HW bug causing the normal FIFO thresholds not to work */
	FEAT_OMAP3_DSI_FIFO_BUG,
	FEAT_DSI_PLL_SELFREQDCO,
	FEAT_DSI_PLL_REFSEL,
	FEAT_WB,
	FEAT_FLIP_IMMEDIATE,
	FEAT_MFLAG,
	FEAT_3D,
};

/* DSS register field id */
enum dss_feat_reg_field {
	FEAT_REG_FIRHINC,
	FEAT_REG_FIRVINC,
	FEAT_REG_FIFOHIGHTHRESHOLD,
	FEAT_REG_FIFOLOWTHRESHOLD,
	FEAT_REG_FIFOSIZE,
	FEAT_REG_HORIZONTALACCU,
	FEAT_REG_VERTICALACCU,
	FEAT_REG_DISPC_CLK_SWITCH,
	FEAT_REG_DSIPLL_REGN,
	FEAT_REG_DSIPLL_REGM,
	FEAT_REG_DSIPLL_REGM_DISPC,
	FEAT_REG_DSIPLL_REGM_DSI,
};

enum dss_range_param {
	FEAT_PARAM_DSS_FCK,
	FEAT_PARAM_DSS_PCD,
	FEAT_PARAM_DSIPLL_REGN,
	FEAT_PARAM_DSIPLL_REGM,
	FEAT_PARAM_DSIPLL_REGM_DISPC,
	FEAT_PARAM_DSIPLL_REGM_DSI,
	FEAT_PARAM_DSIPLL_FINT,
	FEAT_PARAM_DSIPLL_LPDIV,
	FEAT_PARAM_DOWNSCALE,
	FEAT_PARAM_LINEWIDTH,
	FEAT_PARAM_MGR_WIDTH,
	FEAT_PARAM_MGR_HEIGHT,
};

/* DSS Feature Functions */
int dss_feat_get_num_mgrs(void);
int dss_feat_get_num_ovls(void);
unsigned long dss_feat_get_param_min(enum dss_range_param param);
unsigned long dss_feat_get_param_max(enum dss_range_param param);
enum omap_display_type dss_feat_get_supported_displays(enum omap_channel channel);
enum omap_color_mode dss_feat_get_supported_color_modes(enum omap_plane plane);
enum omap_overlay_caps dss_feat_get_overlay_caps(enum omap_plane plane);
bool dss_feat_color_mode_supported(enum omap_plane plane,
		enum omap_color_mode color_mode);
const char *dss_feat_get_clk_source_name(enum omap_dss_clk_source id);

u32 dss_feat_get_buffer_size_unit(void);	/* in bytes */
u32 dss_feat_get_burst_size_unit(void);		/* in bytes */

int dss_feat_get_dsi_ddr_div(void);

unsigned long dss_feat_get_hdmi_core_sys_offset(void);
unsigned long dss_feat_get_hdmi_cec_offset(void);
unsigned long dss_feat_get_hdmi_i2cm_offset(void);

bool dss_has_feature(enum dss_feat_id id);
void dss_feat_get_reg_field(enum dss_feat_reg_field id, u8 *start, u8 *end);
void dss_features_init(void);
#if defined(CONFIG_OMAP4_DSS_HDMI) || defined(CONFIG_OMAP5_DSS_HDMI)
void dss_init_hdmi_ip_ops(struct hdmi_ip_data *ip_data);
#endif
#endif