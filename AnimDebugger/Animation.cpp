#include "Animation.hpp"

std::unordered_map<std::string, sf::Texture*> Animation::m_textures;

Animation::Animation()
{
	m_cur_frame = 0;
	m_frames_count = 0;
	m_rows_count = 0;
	m_frame_duration = 0;
	m_time = 0;
	m_cur_row = 0;
	m_origin_offset = {0, 0};
}

void Animation::loadFromFile(std::string file, uint8_t frames_cnt, uint8_t rows_cnt, float frame_duration)
{
	sf::Texture* texture;
	try
	{
		texture = m_textures.at(file);
	}
	catch (std::out_of_range& e)
	{
		texture = new sf::Texture();
		texture->loadFromFile(file);
		m_textures.insert(std::pair<std::string, sf::Texture*>(file, texture));
	}

	m_sprite.setTexture(*texture);
	m_frames_count = frames_cnt;
	m_rows_count = rows_cnt;
	m_frame_duration = frame_duration;
	m_time = frame_duration;
	this->setOrigin(m_origin_offset);
	update();
}

void Animation::loadFromMemory(sf::Texture& texture, uint8_t frames_cnt, uint8_t rows_cnt, float frame_duration)
{
	m_sprite.setTexture(texture);
	m_frames_count = frames_cnt;
	m_rows_count = rows_cnt;
	m_frame_duration = frame_duration;
	m_time = frame_duration;
	this->setOrigin(m_origin_offset);
	update();
}

void Animation::setPosition(sf::Vector2f position)
{
	m_sprite.setPosition(position);
}

void Animation::setScale(sf::Vector2f scale)
{
	m_sprite.setScale(scale);
}

void Animation::setOrigin(sf::Vector2f offset)
{
	m_origin_offset = offset;
	if (m_frames_count != 0 && m_rows_count != 0)
		m_sprite.setOrigin(
			m_sprite.getTexture()->getSize().x / m_frames_count * offset.x,
			m_sprite.getTexture()->getSize().y / m_rows_count * offset.y
		);
}

void Animation::setCurrentFrame(uint8_t frame)
{
	if (m_frames_count != 0) m_cur_frame = frame % m_frames_count;
	else m_cur_frame = 0;
	update();
}

void Animation::setCurrentRow(uint8_t row)
{
	if (m_rows_count != 0) m_cur_row = row % m_rows_count;
	else m_cur_row = 0;
	update();
}

void Animation::playLoopInStraightOrder(float tick)
{
	m_time -= tick;
	if (m_time <= 0)
	{
		m_time = m_frame_duration;
		m_cur_frame++;
	}
	if (m_cur_frame >= m_frames_count) m_cur_frame = 0;
	update();
}

void Animation::playLoopInReverseOrder(float tick)
{
	m_time -= tick;
	if (m_time <= 0)
	{
		m_time = m_frame_duration;
		m_cur_frame--;
	}
	if (m_cur_frame < 0) m_cur_frame = m_frames_count - 1;
	update();
}

bool Animation::playInStraightOrder(float tick)
{
	if (m_cur_frame < m_frames_count - 1)
	{
		m_time -= tick;
		if (m_time <= 0)
		{
			m_time = m_frame_duration;
			m_cur_frame++;
		}
		update();
		return false;
	}
	else 
	{
		update();
		return true;
	}
}

bool Animation::playInReverseOrder(float tick)
{
	if (m_cur_frame > 0)
	{
		m_time -= tick;
		if (m_time <= 0)
		{
			m_time = m_frame_duration;
			m_cur_frame--;
		}
		update();
		return false;
	}
	else
	{
		update();
		return true;
	}
}

void Animation::update()
{
	sf::Vector2u frame_size = { m_sprite.getTexture()->getSize().x / m_frames_count, m_sprite.getTexture()->getSize().y / m_rows_count};
	m_sprite.setTextureRect(
		sf::IntRect(
			m_cur_frame * frame_size.x,
			m_cur_row * frame_size.y,
			frame_size.x,
			frame_size.y
		)
	);
}

void Animation::reset()
{
	m_cur_frame = 0;
}

void Animation::relese()
{
	for (auto it = m_textures.begin(); it != m_textures.end(); it++)
		delete it->second;
}

void Animation::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(m_sprite, states);
}