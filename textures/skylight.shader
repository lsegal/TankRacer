textures/sky1
{
	skyparms textures/sky1 1024 -

	q3map_lightImage textures/sky1.jpg

	q3map_sunExt 1 1 1 140 -35 25 3 16		//adds deviance and samples
	q3map_skylight 100 3

	surfaceparm sky
	surfaceparm noimpact
	surfaceparm nolightmap
	surfaceparm nodlight

	nopicmip
	nomipmaps

	qer_editorimage textures/sky1.jpg

	{
		map textures/sky1.jpg
		tcMod scale 3 3
		//tcMod scroll 0.005 -0.0125
		rgbGen identityLighting
	}
	{
		map textures/sky2.jpg
		blendFunc GL_ONE_MINUS_SRC_ALPHA GL_SRC_ALPHA
		tcMod transform 0.25 0 0 0.25 0.1075 0.1075
		rgbGen identityLighting
	}
}
