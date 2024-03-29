/*
 * ASoC Driver for I-Sabre K2M
 *
 * Author: Satoru Kawase
 *      Copyright 2017 Audiophonics
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <sound/core.h>
#include <sound/soc.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>

#include "i-sabre-codec.h"


static int snd_tb_i_sabre_k2m_init(struct snd_soc_pcm_runtime *rtd)
{
	struct snd_soc_codec *codec = rtd->codec;
	unsigned int value;

	/* Device ID */
	value = snd_soc_read(codec, ISABRECODEC_REG_01);
	dev_info(codec->dev, "Audiophonics Device ID : %02X\n", value);

	/* API revision */
	value = snd_soc_read(codec, ISABRECODEC_REG_02);
	dev_info(codec->dev, "Audiophonics API revision : %02X\n", value);

	return 0;
}

static int snd_tb_i_sabre_k2m_hw_params(
	struct snd_pcm_substream *substream, struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd     = substream->private_data;
	struct snd_soc_dai         *cpu_dai = rtd->cpu_dai;
	unsigned int mclk;
	unsigned int mclk_fs = 512;
	int ret = 0;

	mclk = params_rate(params) * mclk_fs;

	ret = snd_soc_dai_set_sysclk(cpu_dai, 0, mclk, 0);
	if (ret && (ret != -ENOTSUPP)) {
		return ret;
	}

	return 0;
}

/* machine stream operations */
static struct snd_soc_ops snd_tb_i_sabre_k2m_ops = {
	.hw_params = snd_tb_i_sabre_k2m_hw_params,
};


static struct snd_soc_dai_link snd_tb_i_sabre_k2m_dai[] = {
	{
		.name           = "I-Sabre K2M",
		.stream_name    = "I-Sabre K2M DAC",
		.cpu_dai_name   = "rockchip-i2s",
		.codec_dai_name = "i-sabre-codec-dai",
		.platform_name  = "rockchip-i2s",
		.codec_name     = "i-sabre-codec-i2c.1-0048",
		.dai_fmt        = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF
						| SND_SOC_DAIFMT_CBS_CFS,
		.init           = snd_tb_i_sabre_k2m_init,
		.ops            = &snd_tb_i_sabre_k2m_ops,
	}
};

/* audio machine driver */
static struct snd_soc_card snd_tb_i_sabre_k2m = {
	.name      = "I-Sabre K2M DAC",
	.owner     = THIS_MODULE,
	.dai_link  = snd_tb_i_sabre_k2m_dai,
	.num_links = ARRAY_SIZE(snd_tb_i_sabre_k2m_dai)
};


static int snd_tb_i_sabre_k2m_probe(struct platform_device *pdev)
{
	int ret = 0;

	snd_tb_i_sabre_k2m.dev = &pdev->dev;
	if (pdev->dev.of_node) {
		struct device_node *i2s_node;
		struct snd_soc_dai_link *dai;

		dai = &snd_tb_i_sabre_k2m_dai[0];
		i2s_node = of_parse_phandle(pdev->dev.of_node,
							"i2s-controller", 0);
		if (i2s_node) {
			dai->cpu_dai_name     = NULL;
			dai->cpu_of_node      = i2s_node;
			dai->platform_name    = NULL;
			dai->platform_of_node = i2s_node;
		} else {
			dev_err(&pdev->dev,
			    "Property 'i2s-controller' missing or invalid\n");
			return (-EINVAL);
		}

		dai->name        = "I-Sabre K2M";
		dai->stream_name = "I-Sabre K2M DAC";
		dai->dai_fmt     = SND_SOC_DAIFMT_I2S
					| SND_SOC_DAIFMT_NB_NF
					| SND_SOC_DAIFMT_CBS_CFS;
	}

	/* Wait for registering codec driver */
	mdelay(50);

	ret = snd_soc_register_card(&snd_tb_i_sabre_k2m);
	if (ret) {
		dev_err(&pdev->dev,
			"snd_soc_register_card() failed: %d\n", ret);
	}

	return ret;
}

static int snd_tb_i_sabre_k2m_remove(struct platform_device *pdev)
{
	return snd_soc_unregister_card(&snd_tb_i_sabre_k2m);
}

static const struct of_device_id snd_tb_i_sabre_k2m_of_match[] = {
	{ .compatible = "audiophonics,i-sabre-k2m", },
	{}
};
MODULE_DEVICE_TABLE(of, snd_tb_i_sabre_k2m_of_match);

static struct platform_driver snd_tb_i_sabre_k2m_driver = {
	.driver = {
		.name           = "snd-tb-i-sabre-k2m",
		.owner          = THIS_MODULE,
		.of_match_table = snd_tb_i_sabre_k2m_of_match,
	},
	.probe  = snd_tb_i_sabre_k2m_probe,
	.remove = snd_tb_i_sabre_k2m_remove,
};
module_platform_driver(snd_tb_i_sabre_k2m_driver);

MODULE_DESCRIPTION("ASoC Driver for I-Sabre K2M");
MODULE_AUTHOR("Audiophonics <http://www.audiophonics.fr>");
MODULE_LICENSE("GPL");
