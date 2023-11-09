#include "Video.hpp"

SDLRenderer::SDLRenderer(int width, int height, int scalingFactor)
	: m_textureWidth(width)
	, m_textureHeight(height)
	, m_windowWidth(width* scalingFactor)
	, m_windowHeight(height* scalingFactor)
	, m_scaling(scalingFactor)
{
	SDL_CreateWindowAndRenderer(m_windowWidth, m_windowHeight, 0, &m_window, &m_renderer);
	SDL_SetWindowTitle(m_window, "GGBoy");
	m_texture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, m_textureWidth, m_textureHeight);
	m_textureTransform = { 0,0, m_windowWidth, m_windowHeight };
}

SDLRenderer::~SDLRenderer()
{
	SDL_DestroyRenderer(m_renderer);
	SDL_DestroyWindow(m_window);
	SDL_DestroyTexture(m_texture);
}

static ggb::RGBA colorCorrection(ggb::RGBA rgb)
{
	int r = rgb.r;
	int g = rgb.g;
	int b = rgb.b;

	r = 3.0 / 4 * rgb.r + 1.0 / 4 * rgb.g;
	g = 3.0 / 5 * rgb.g + 1.0 / 5 * rgb.b + 1.0 / 5 * rgb.r;
	b = 4.0 / 6 * rgb.b + 1.0 / 6 * rgb.r + 1.0 / 6 * rgb.g;

	rgb.r = std::clamp(r, 0, 255);
	rgb.g = std::clamp(g, 0, 255);
	rgb.b = std::clamp(b, 0, 255);
	return rgb;
}

void SDLRenderer::renderNewFrame(const ggb::FrameBuffer& framebuffer)
{
	startRendering();

	for (int x = 0; x < framebuffer.m_buffer.size(); x++)
	{
		for (int y = 0; y < framebuffer.m_buffer[0].size(); y++)
		{
			const auto& rgba = colorCorrection(framebuffer.m_buffer[x][y]);

			const uint32_t pixelPosition = (y * m_pitch) + x * 3;
			m_lockedPixels[pixelPosition] = rgba.r;
			m_lockedPixels[pixelPosition + 1] = rgba.g;
			m_lockedPixels[pixelPosition + 2] = rgba.b;
		}
	}
	finishRendering();
}

void SDLRenderer::startRendering()
{
	SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 0);
	SDL_RenderClear(m_renderer);
	m_pitch = 0;
	m_lockedPixels = nullptr;
	SDL_LockTexture(m_texture, nullptr, reinterpret_cast<void**>(&m_lockedPixels), &m_pitch);
}

void SDLRenderer::finishRendering()
{
	SDL_UnlockTexture(m_texture);
	SDL_RenderCopy(m_renderer, m_texture, nullptr, &m_textureTransform);
	SDL_RenderPresent(m_renderer);
}
