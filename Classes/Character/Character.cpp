/**
* @file Character.cpp
*/

#include "cocos2d.h"
#include "Character.h"

void Character::bindPictureSprite(cocos2d::Sprite* sprite)
{
	sprite_ = sprite;
	addChild(sprite_);
}

void Character::bindPictureMonster(cocos2d::Sprite* sprite)
{
	monster_ = sprite;
	addChild(monster_);
}
