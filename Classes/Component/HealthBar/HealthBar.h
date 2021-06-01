/**
* @file HealthBar.h
*/

#ifndef __HEALTH_BAR_H__
#define __HEALTH_BAR_H__

#include "cocos2d.h"
#include "Character/Character.h"

class HealthBar : public cocos2d::Node
{
public:


	static HealthBar* create(Character* character);


	void update(float dt);


protected:

	cocos2d::Sprite* sprite_;
	
	Character* character_;

	int curHealth_;

	cocos2d::ProgressTimer* health_;
	cocos2d::Label* healthInfo_;


};

#endif // !__HEALTH_BAR_H__