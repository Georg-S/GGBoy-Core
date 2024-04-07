#include "PixelProcessingUnit.hpp"

#include <iostream>

#include "Utility.hpp"
#include "Logging.hpp"
#include "Constants.hpp"


using namespace ggb;

static constexpr int VRAM_TILE_COUNT = 384;

ggb::PixelProcessingUnit::PixelProcessingUnit(BUS* bus)
{
	setBus(bus);
	m_currentRowBuffer = std::vector<RGBA>(8, { 0,0,0,0 });
	m_objColorBuffer = std::vector<uint8_t>(8, 0);
	m_currentObjectRowPixelBuffer = std::vector<ObjectPixel>(GAME_WINDOW_WIDTH, { {} });
	m_pixelBuffer = std::vector<BackgroundAndWindowPixel>(GAME_WINDOW_WIDTH, { {} });
	m_gameFrameBuffer = std::make_unique<FrameBuffer>(GAME_WINDOW_WIDTH, GAME_WINDOW_HEIGHT);
	m_tileDataFrameBuffer = std::make_unique<FrameBuffer>(TILE_DATA_WIDTH, TILE_DATA_HEIGHT);
	m_vramTiles = std::vector<Tile>(VRAM_TILE_COUNT, {});
}

void ggb::PixelProcessingUnit::reset()
{
	m_cycleCounter = 0;
	m_GBCBackgroundColorRAM.reset();
	m_GBCObjectColorRAM.reset();
}

void ggb::PixelProcessingUnit::setBus(BUS* bus)
{
	m_bus = bus;
	m_LCDControl = m_bus->getPointerIntoMemory(LCD_CONTROL_REGISTER_ADDRESS);
	m_LCDStatus = m_bus->getPointerIntoMemory(LCD_STATUS_REGISTER_ADDRESS);
	m_LCDYCoordinate = m_bus->getPointerIntoMemory(LCD_Y_COORDINATE_ADDRESS);
	m_LYCompare = m_bus->getPointerIntoMemory(LCD_Y_COMPARE_ADDRESS);
	m_backgroundPalette = m_bus->getPointerIntoMemory(BACKGROUND_PALETTE_ADDRESS);
	m_objectPalette0 = m_bus->getPointerIntoMemory(OBJECT_PALETTE_0_ADDRESS);
	m_objectPalette1 = m_bus->getPointerIntoMemory(OBJECT_PALETTE_1_ADDRESS);
	m_viewPortXPos = m_bus->getPointerIntoMemory(LCD_VIEWPORT_X_ADDRESS);
	m_viewPortYPos = m_bus->getPointerIntoMemory(LCD_VIEWPORT_Y_ADDRESS);
	m_windowXPos = m_bus->getPointerIntoMemory(LCD_WINDOW_X_ADDRESS);
	m_windowYPos = m_bus->getPointerIntoMemory(LCD_WINDOW_Y_ADDRESS);
	m_GBCBackgroundColorRAM.setBus(m_bus);
	m_GBCObjectColorRAM.setBus(m_bus);
	m_VRAMBank0Ptr = m_bus->getVRAMStartPointer(0);
	m_VRAMBank1Ptr = m_bus->getVRAMStartPointer(1);
	m_GBCObjectPriorityMode = m_bus->getPointerIntoMemory(GBC_OBJECT_PRIORITY_MODE_ADDRESS);

	static constexpr uint8_t objectSize = 4; // bytes
	m_objects.resize(OBJECT_COUNT);
	for (auto i = 0; i < OBJECT_COUNT; i++)
	{
		uint16_t objectAddress = OAM_ADDRESS + (i * objectSize);
		m_objects[i].yPosition = m_bus->getPointerIntoMemory(objectAddress);
		m_objects[i].xPosition = m_bus->getPointerIntoMemory(objectAddress + 1);
		m_objects[i].tileIndex = m_bus->getPointerIntoMemory(objectAddress + 2);
		m_objects[i].attributes = m_bus->getPointerIntoMemory(objectAddress + 3);
	}
}

void ggb::PixelProcessingUnit::step(int elapsedCycles)
{
	const auto currentMode = getCurrentLCDMode();
	if (!isEnabled())
	{
		if (currentMode == LCDMode::HBLank)
			return;

		if (currentMode != LCDMode::HBLank)
		{
			// This is probably the "correct" way of handling the disabling of the screen
			setLCDMode(LCDMode::HBLank);
			*m_LCDYCoordinate = 0;
			m_cycleCounter = 0;
			return;
		}
		return;
	}

	const auto currentModeDuration = getModeDuration(currentMode);
	m_cycleCounter += elapsedCycles;

	if (m_cycleCounter < currentModeDuration)
		return;

	m_cycleCounter -= currentModeDuration;

	switch (currentMode)
	{
	case ggb::LCDMode::OAMBlocked:
	{
		setLCDMode(LCDMode::VRAMBlocked);
		return;
	}
	case ggb::LCDMode::VRAMBlocked:
	{
		setLCDMode(LCDMode::HBLank);
		handleModeTransitionInterrupt(LCDInterrupt::HBlank);
		writeCurrentScanLineIntoFrameBuffer();
		m_bus->handleHBlank();
		return;
	}
	case ggb::LCDMode::HBLank:
	{
		auto line = incrementScanline();
		if (line >= 144)
		{
			setLCDMode(LCDMode::VBLank);
			m_bus->requestInterrupt(INTERRUPT_VBLANK_BIT);
			handleModeTransitionInterrupt(LCDInterrupt::VBlank);
			updateAndRenderTileData();
			renderGame();
		}
		else
		{
			setLCDMode(LCDMode::OAMBlocked);
			handleModeTransitionInterrupt(LCDInterrupt::OAM);
		}
		return;
	}
	case ggb::LCDMode::VBLank:
	{
		auto line = incrementScanline();
		if (line == 0)
		{
			setLCDMode(LCDMode::OAMBlocked);
			handleModeTransitionInterrupt(LCDInterrupt::OAM);
		}
		return;
	}
	default:
		assert(!"INVALID mode");
	}
}

bool ggb::PixelProcessingUnit::isEnabled() const
{
	return isBitSet(*m_LCDControl, 7);
}

LCDMode ggb::PixelProcessingUnit::getCurrentLCDMode() const
{
	const uint8_t buf = *m_LCDStatus & 0b11;
	return static_cast<LCDMode>(buf);
}

void ggb::PixelProcessingUnit::setLCDMode(LCDMode mode)
{
	setBitToValue(*m_LCDStatus, 0, static_cast<uint8_t>(mode) & 1);
	setBitToValue(*m_LCDStatus, 1, static_cast<uint8_t>(mode) & (1 << 1));
}

void ggb::PixelProcessingUnit::setTileDataRenderer(std::unique_ptr<Renderer> renderer)
{
	m_tileDataRenderer = std::move(renderer);
}

void ggb::PixelProcessingUnit::setGameRenderer(std::unique_ptr<Renderer> renderer)
{
	m_gameRenderer = std::move(renderer);
}

Dimensions ggb::PixelProcessingUnit::getTileDataDimensions() const
{
	return Dimensions{ TILE_DATA_WIDTH, TILE_DATA_HEIGHT };
}

void ggb::PixelProcessingUnit::setDrawTileData(bool enable)
{
	m_drawTileData = enable;
}

void ggb::PixelProcessingUnit::setDrawWholeBackground(bool enable)
{
	m_drawWholeBackground = enable;
}

void ggb::PixelProcessingUnit::serialization(Serialization* serialization)
{
	// TODO emulator should probably save / serialize the last saved frame and render it (will be useful if the emulator is paused)
	serialization->read_write(m_cycleCounter);
	serialization->read_write(m_drawWholeBackground);
	serialization->read_write(m_drawTileData);
	serialization->read_write(m_GBCMode);
	serialization->read_write(m_objects);
	serialization->read_write(m_currentScanlineObjects);
	serialization->read_write(m_vramTiles);
	serialization->read_write(m_currentRowBuffer);
	serialization->read_write(m_objColorBuffer);
	serialization->read_write(m_pixelBuffer);
	serialization->read_write(m_currentObjectRowPixelBuffer);

	m_gameFrameBuffer->serialization(serialization);
	m_tileDataFrameBuffer->serialization(serialization);
	m_GBCBackgroundColorRAM.serialization(serialization);
	m_GBCObjectColorRAM.serialization(serialization);
}

void ggb::PixelProcessingUnit::GBCWriteToColorRAM(uint16_t address, uint8_t value)
{
	if (address == GBC_BACKGROUND_PALETTE_DATA_ADDRESS)
	{
		m_GBCBackgroundColorRAM.write(value);
		return;
	}
	assert(address == GBC_OBJECT_COLOR_PALETTE_DATA_ADDRESS);
	m_GBCObjectColorRAM.write(value);
}

uint8_t ggb::PixelProcessingUnit::GBCReadColorRAM(uint16_t address) const
{
	if (address == GBC_BACKGROUND_PALETTE_DATA_ADDRESS)
		return m_GBCBackgroundColorRAM.read();

	assert(address == GBC_OBJECT_COLOR_PALETTE_DATA_ADDRESS);
	return m_GBCObjectColorRAM.read();
}

void ggb::PixelProcessingUnit::renderGame()
{
	if (!m_gameRenderer)
		return;

	m_gameRenderer->renderNewFrame(*m_gameFrameBuffer);
}

void ggb::PixelProcessingUnit::writeCurrentScanLineIntoFrameBuffer()
{
	if (m_GBCMode) 
	{
		m_GBCBackgroundColorRAM.updateColorPalettes();
		m_GBCObjectColorRAM.updateColorPalettes();
	}

	if (m_GBCMode || isBitSet(*m_LCDControl, 0)) 
	{
		// Not quite correct, instead of turning the background and window off, it should be white
		writeCurrentBackgroundLineIntoFrameBuffer();
		if (isBitSet(*m_LCDControl, 5))
			writeCurrentWindowLineIntoBuffer();
	}

	updateCurrentScanlineObjects();
	if (isBitSet(*m_LCDControl, 1))
		writeCurrentObjectLineIntoBuffer();

	for (int x = 0; x < GAME_WINDOW_WIDTH; x++)
	{
		const auto& backgroundAndWindowPixel = m_pixelBuffer[x];
		const auto& objectPixel = m_currentObjectRowPixelBuffer[x];

		m_gameFrameBuffer->setPixel(x, scanLine(), backgroundAndWindowPixel.rgb);
		if (objectPixel.pixelSet)
		{
			if (objectPixel.backgroundOverObj && (backgroundAndWindowPixel.rawColorValue != 0))
				continue;
			m_gameFrameBuffer->setPixel(x, scanLine(), objectPixel.rgb);
		}
	}
}

void ggb::PixelProcessingUnit::updateCurrentScanlineObjects()
{
	// Offset needed to convert object coordinates to screen coordinates
	static const int screenOffset = 16;

	m_currentScanlineObjects.clear();
	m_currentScanlineObjects.reserve(10);
	for (const auto& obj : m_objects)
	{
		int yObjStart = static_cast<int>(*obj.yPosition) - screenOffset;
		int yObjEnd = static_cast<int>(*obj.yPosition) + getObjectHeight() - screenOffset - 1;

		if (*m_LCDYCoordinate < yObjStart || *m_LCDYCoordinate > yObjEnd)
			continue;

		m_currentScanlineObjects.emplace_back(obj);
		if (m_currentScanlineObjects.size() >= MAX_ALLOWED_OBJS_PER_SCANLINE)
			break;
	}

	if (!m_GBCMode || isBitSet(*m_GBCObjectPriorityMode, 0)) 
	{
		// Order objects by x-coordinate
		// If obj1.x == obj2.x the obj which is first in memory should overlap the one coming after it -> therefore use stable_sort
		std::stable_sort(m_currentScanlineObjects.begin(), m_currentScanlineObjects.end(), [](const Object& lhs, const Object& rhs)
			{
				return *lhs.xPosition < *rhs.xPosition;
			});
	}
	else 
	{
		// Objects are already oredered by appearance in OAM, so nothing to do here
	}
	// Use reverse order, so that we can just iterate over the objects 
	// and paint one after another and the overlapping is handled correctly
	std::reverse(m_currentScanlineObjects.begin(), m_currentScanlineObjects.end());
}

void ggb::PixelProcessingUnit::writeCurrentBackgroundLineIntoFrameBuffer()
{
	RenderingScanlineData data = {};
	const uint16_t backgroundTileMap = isBitSet(*m_LCDControl, 3) ? 0x9C00 : 0x9800;
	data.signedAddressingMode = !isBitSet(*m_LCDControl, 4);

	// TODO is the % 256 correct?
	const auto yPosInBackground = (scanLine() + *m_viewPortYPos) % 256;
	auto lineShift = ((yPosInBackground / TILE_HEIGHT) * TILE_MAP_WIDTH);
	assert(lineShift < TILE_MAP_SIZE);

	data.tileRow = yPosInBackground % TILE_HEIGHT;
	data.tileColumn = *m_viewPortXPos % TILE_WIDTH;

	for (data.screenXPos = 0; data.screenXPos < GAME_WINDOW_WIDTH;)
	{
		auto xBuf = ((*m_viewPortXPos + data.screenXPos) / TILE_WIDTH) & 0x1F;
		auto tileMapIndex = xBuf + lineShift;
		assert(tileMapIndex < TILE_MAP_SIZE);
		data.tileIndexAddress = backgroundTileMap + tileMapIndex;

		writeTileIntoBuffer(&data);
	}
}


void ggb::PixelProcessingUnit::writeTileIntoBuffer(RenderingScanlineData* inOutData)
{
	auto& screenXPos = inOutData->screenXPos;
	auto& tileColumn = inOutData->tileColumn;
	// TODO implement priority
	const auto GBCTileData = getBackgroundTileAttributes(inOutData->tileIndexAddress);
	const auto colorPaletteIndex = GBCTileData & 0b111;
	const auto& GBCPalette = GBCGetBackgroundAndWindowColorPalette(colorPaletteIndex);
	const auto tileAddress = getTileAddress(inOutData->tileIndexAddress, inOutData->signedAddressingMode);
	uint8_t* vramBank = getVRAMBankPointer(GBCTileData);
	int tileRow = inOutData->tileRow;

	if (isBitSet(GBCTileData, 6))
		tileRow = (TILE_HEIGHT - 1) - tileRow; // Flip Y

	getTileRowData(vramBank, tileAddress, tileRow, m_objColorBuffer);
	if (isBitSet(GBCTileData, 5))
		std::reverse(m_objColorBuffer.begin(), m_objColorBuffer.end()); // Flip X

	while (inOutData->tileColumn < TILE_WIDTH && screenXPos < GAME_WINDOW_WIDTH)
	{
		if (screenXPos >= 0)
		{
			const auto colorValue = m_objColorBuffer[tileColumn];
			const auto& rgb = GBCPalette.getColor(colorValue);
			m_pixelBuffer[screenXPos] = { rgb, colorValue };
		}
		++tileColumn;
		++screenXPos;
	}
	tileColumn = 0;
}

void ggb::PixelProcessingUnit::writeCurrentWindowLineIntoBuffer()
{
	const auto convertScreenCoordinateToWindow = [&](int screenCoord) -> int
	{
		return (screenCoord + 7) - *m_windowXPos;
	};
	const auto convertWindowCoordinateToScreen = [](int windowCoord) -> int
	{
		return windowCoord - 7;
	};

	if (scanLine() < *m_windowYPos)
		return;

	RenderingScanlineData data = {};
	// TODO Refactor it into the same method as the background?
	const uint16_t windowTileMap = isBitSet(*m_LCDControl, 6) ? 0x9C00 : 0x9800;
	data.signedAddressingMode = !isBitSet(*m_LCDControl, 4);

	const auto yPos = scanLine() - *m_windowYPos;
	const auto yTileOffset = (yPos / TILE_HEIGHT) * TILE_MAP_WIDTH;
	assert(yTileOffset < TILE_MAP_SIZE);
	const auto screenX = convertWindowCoordinateToScreen(*m_windowXPos);
	data.tileRow = yPos % TILE_HEIGHT;
	data.tileColumn = 0; // Window row rendering always starts at the beginning of a tile

	for (data.screenXPos = screenX; data.screenXPos < GAME_WINDOW_WIDTH;)
	{
		auto tileIndexAddress = (convertScreenCoordinateToWindow(data.screenXPos) / TILE_WIDTH) + yTileOffset;
		data.tileIndexAddress = windowTileMap + tileIndexAddress;
		writeTileIntoBuffer(&data);
	}
}

void ggb::PixelProcessingUnit::writeCurrentObjectLineIntoBuffer()
{
	constexpr int TRANSPARENT_PIXEL_VALUE = 0;
	constexpr int SCREEN_Y_OFFSET = 16;
	constexpr int SCREEN_X_OFFSET = 8;

	for (auto& obj : m_currentObjectRowPixelBuffer)
		obj.pixelSet = false;

	for (const auto& obj : m_currentScanlineObjects)
	{
		uint16_t tileAddress = TILE_MAP_1_ADDRESS + (*obj.tileIndex * TILE_MEMORY_SIZE);
		const auto objScreenYPos = *obj.yPosition - SCREEN_Y_OFFSET;
		const auto& colorPalette = GBCGetObjectColorPalette(obj);
		const bool backgroundOverObj = obj.drawBackgroundOverObject();
		auto objTileLine = scanLine() - objScreenYPos;
		uint8_t* vramBank = getVRAMBankPointer(*obj.attributes);

		if (obj.isFlipYSet())
			objTileLine = (getObjectHeight() - 1) - objTileLine;

		// Kind of hacky for 8x16 tiles e.g. to just read the 14th tile row (even though tiles only have 8 rows)
		// however since 8x16 objects have their tiles continuous in memory, this seems to be the cleanest way to handle them
		getTileRowData(vramBank, tileAddress, objTileLine, m_objColorBuffer);
		if (obj.isFlipXSet())
			std::reverse(m_objColorBuffer.begin(), m_objColorBuffer.end());

		for (int x = *obj.xPosition - SCREEN_X_OFFSET, objX = 0; objX < TILE_WIDTH && x < GAME_WINDOW_WIDTH; ++x, ++objX)
		{
			if (x < 0)
				continue;

			const auto currentColorValue = m_objColorBuffer[objX];
			if (currentColorValue == TRANSPARENT_PIXEL_VALUE)
				continue;

			m_currentObjectRowPixelBuffer[x] = { colorPalette.getColor(currentColorValue), backgroundOverObj, true };
		}
	}

	for (int x = 0; x < GAME_WINDOW_WIDTH; x++)
	{
		if (m_currentObjectRowPixelBuffer[x].pixelSet)
			m_gameFrameBuffer->setPixel(x, scanLine(), m_currentObjectRowPixelBuffer[x].rgb);
	}
}

uint8_t* ggb::PixelProcessingUnit::getVRAMBankPointer(uint8_t attributes)
{
	if (m_GBCMode && isBitSet(attributes, 3))
		return m_VRAMBank1Ptr;
	return m_VRAMBank0Ptr;
}

uint16_t ggb::PixelProcessingUnit::getTileAddress(uint16_t tileIndexAddress, bool useSignedAddressing)
{
	auto index = getVRAMIndexFromAddress(tileIndexAddress);

	if (useSignedAddressing)
	{
		// Currently this cast is implementation defined behavior, with C++20 this can be easily made well defined
		int8_t tileIndex = static_cast<int8_t>(m_VRAMBank0Ptr[index]); // The tile maps are only in the VRAM Bank 0
		return TILE_MAP_2_ADDRESS + (tileIndex * TILE_MEMORY_SIZE);
	}

	auto tileIndex = m_VRAMBank0Ptr[index]; // The tile maps are only in the VRAM Bank 0
	return TILE_MAP_1_ADDRESS + (tileIndex * TILE_MEMORY_SIZE);
}

void ggb::PixelProcessingUnit::handleModeTransitionInterrupt(LCDInterrupt type)
{
	if (isBitSet(*m_LCDStatus, static_cast<int>(type)))
		m_bus->requestInterrupt(INTERRUPT_LCD_STAT_BIT);
}

constexpr int ggb::PixelProcessingUnit::getModeDuration(LCDMode mode) const
{
	// In a real gameboy the length of "VRAMBlocked" and "HBLank" vary,
	// however to simplify it we assume the mode duration is always the same
	switch (mode)
	{
	case ggb::LCDMode::HBLank:		return 204;
	case ggb::LCDMode::VBLank:		return 456;
	case ggb::LCDMode::OAMBlocked:	return 80;
	case ggb::LCDMode::VRAMBlocked:	return 172;
	default:
		assert(!"FAILURE");
		return 0;
	}
}

uint8_t ggb::PixelProcessingUnit::scanLine() const
{
	return *m_LCDYCoordinate;
}

uint8_t ggb::PixelProcessingUnit::incrementScanline()
{
	*m_LCDYCoordinate = (*m_LCDYCoordinate + 1) % 154;

	if (*m_LYCompare == *m_LCDYCoordinate)
	{
		setBit(*m_LCDStatus, 2);

		if (isBitSet(*m_LCDStatus, 6))
			m_bus->requestInterrupt(INTERRUPT_LCD_STAT_BIT);
	}
	else
	{
		clearBit(*m_LCDStatus, 2);
	}

	return *m_LCDYCoordinate;
}

static ColorPalette getPalette(uint8_t value)
{
	ColorPalette result = {};
	result.m_color[0] = convertGBColorToRGB(static_cast<GBColor>(value & 0b11));
	result.m_color[1] = convertGBColorToRGB(static_cast<GBColor>((value >> 2) & 0b11));
	result.m_color[2] = convertGBColorToRGB(static_cast<GBColor>((value >> 4) & 0b11));
	result.m_color[3] = convertGBColorToRGB(static_cast<GBColor>((value >> 6) & 0b11));
	return result;
}

ColorPalette ggb::PixelProcessingUnit::getBackgroundAndWindowColorPalette() const
{
	return getPalette(*m_backgroundPalette);
}

ColorPalette ggb::PixelProcessingUnit::GBCGetBackgroundAndWindowColorPalette(size_t index) const
{
	if (m_GBCMode)
		return m_GBCBackgroundColorRAM.getColorPalette(index);
	return getPalette(*m_backgroundPalette);
}

ColorPalette ggb::PixelProcessingUnit::GBCGetObjectColorPalette(const Object& obj) const
{
	if (m_GBCMode)
		return m_GBCObjectColorRAM.getColorPalette(obj.getGBCPaletteIndex());

	if (obj.usePalette1())
		return getPalette(*m_objectPalette1);
	return getPalette(*m_objectPalette0);
}

static void renderTileData(const std::vector<Tile>& tiles, FrameBuffer* frameBuffer, Renderer* renderer)
{
	int currX = 0;
	int currY = 0;
	constexpr int margin = 10;

	for (const auto& tile : tiles)
	{
		for (int x = 0; x < TILE_WIDTH; x++)
		{
			for (int y = 0; y < TILE_HEIGHT; y++)
			{
				const auto& color = tile.m_data[x][y];
				frameBuffer->m_buffer[currX * margin + x][currY * margin + y] = color;
			}
		}
		currX += 1;

		if (currX == 20)
		{
			currX = 0;
			currY += 1;
		}
	}
	renderer->renderNewFrame(*frameBuffer);
}

void ggb::PixelProcessingUnit::updateAndRenderTileData()
{
	if (!m_drawTileData || !m_tileDataRenderer)
		return; // We don't want to render the tile data -> therefore we don't update the data as well

	const auto colorPalette = getBackgroundAndWindowColorPalette();
	for (uint16_t i = 0; i < VRAM_TILE_COUNT; i++)
		overWriteTileData(m_bus, i, colorPalette, &m_vramTiles[i], m_objColorBuffer);

	renderTileData(m_vramTiles, m_tileDataFrameBuffer.get(), m_tileDataRenderer.get());
}

int ggb::PixelProcessingUnit::getObjectHeight() const
{
	if (isBitSet(*m_LCDControl, 2))
		return TILE_HEIGHT * 2;
	return TILE_HEIGHT;
}

uint8_t ggb::PixelProcessingUnit::getBackgroundTileAttributes(uint16_t address) const
{
	auto index = getVRAMIndexFromAddress(address);
	return m_VRAMBank1Ptr[index];
}
