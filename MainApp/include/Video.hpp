#pragma once
#include <RenderingUtility.hpp>
#include <SDL.h>

class SDLRenderer : public ggb::Renderer
{
public:
	SDLRenderer(int width, int height, int scalingFactor = 1);
	~SDLRenderer();
	virtual void renderNewFrame(const ggb::FrameBuffer& framebuffer);

private:
	void startRendering();
	void finishRendering();
	SDL_Window* m_window = nullptr;
	SDL_Renderer* m_renderer = nullptr;
	SDL_Texture* m_texture = nullptr;
	SDL_Rect m_textureTransform = {};
	uint8_t* m_lockedPixels = nullptr;

	int m_textureWidth = 0;
	int m_textureHeight = 0;
	int m_windowWidth = 0;
	int m_windowHeight = 0;
	int m_pitch = 0;
	int m_scaling = 1;
};