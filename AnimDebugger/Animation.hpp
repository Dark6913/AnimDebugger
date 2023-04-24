#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>

class Animation : public sf::Drawable
{
public:
	Animation();
	void loadFromFile(std::string file, uint8_t frames_cnt, uint8_t rows_cnt, float frame_duration);
	void loadFromMemory(sf::Texture& texture, uint8_t frames_cnt, uint8_t rows_cnt, float frame_duration);
	void setPosition(sf::Vector2f position);
	void setScale(sf::Vector2f scale);
	void setOrigin(sf::Vector2f offset);
	void setCurrentFrame(uint8_t frame);
	void setCurrentRow(uint8_t row);
	void playLoopInStraightOrder(float tick);
	void playLoopInReverseOrder(float tick);
	bool playInStraightOrder(float tick);
	bool playInReverseOrder(float tick);
	void reset();
	static void relese();

	sf::IntRect getFrameRect() { return m_sprite.getTextureRect(); };
	const sf::Texture* getTexture() { return m_sprite.getTexture(); };
	int8_t getCurrentFrame() { return m_cur_frame; };
	int8_t getCurrentRow() { return m_cur_row; };
	int8_t getFramesCount() { return m_frames_count; };
	int8_t getRowsCount() { return m_rows_count; };

	sf::FloatRect getGlobalBounds() { return m_sprite.getGlobalBounds(); };
	sf::Vector2f getScale() { return m_sprite.getScale(); };
	sf::Vector2f getOriginOffset() { return m_sprite.getOrigin(); };
private:
	static std::unordered_map<std::string, sf::Texture*> m_textures;
	sf::Sprite m_sprite;
	int8_t m_cur_frame;
	int8_t m_cur_row;
	int8_t m_frames_count;
	int8_t m_rows_count;
	sf::Vector2f m_origin_offset;
	float m_frame_duration;
	float m_time;

	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	void update();
};