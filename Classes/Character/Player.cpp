/**
* @file Player.cpp
*/

#include "Player.h"
#include "./Item/PlayerWeapon/Weapon.h"
#include "./Scene/HelloWorldScene.h"
#include <cmath>

Player* Player::create(const std::string& filename)
{
	auto player = new(std::nothrow) Player();
	if (!player)
	{
		return nullptr;
	}
	player->bindPictureSprite(cocos2d::Sprite::create(filename));

	if (player && player->sprite_)
	{
		//设置角色初始位置
		player->setPosition(cocos2d::Vec2(50, 50));
		//标记角色
		player->setTag(ME);

		player->bindAnimate("MIKU");

		//初始化角色武器和弹药
		player->primaryWeapon_ = Weapon::create("default_weapon.png");
		player->secondaryWeapon_ = Weapon::create("default_sec_weapon.png");
		player->bulletFilename_ = "dart.png";
		player->addChild(player->primaryWeapon_);
		player->addChild(player->secondaryWeapon_);
		player->primaryWeapon_->setVisible(true);			//默认显示主武器，不显示副武器
		player->secondaryWeapon_->setVisible(false);
		player->moveSpeed_ = 400.f;
		player->health_ = 3;
		player->shield_ = 0.5f;
		//为角色设置物理躯干
		player->bindPhysicsBody();

		player->autorelease();
		return player;
	}
	return nullptr;
}

bool Player::bindPhysicsBody()
{
	auto physicsBody = cocos2d::PhysicsBody::createBox(sprite_->getContentSize(), cocos2d::PhysicsMaterial(0.0f, 0.0f, 0.0f));
	physicsBody->setDynamic(false);
	physicsBody->setContactTestBitmask(1);
	physicsBody->setCategoryBitmask(5);
	setPhysicsBody(physicsBody);

	return true;
}

void Player::listenToKeyPress(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* unusedEvent)
{
	using K = cocos2d::EventKeyboard::KeyCode;

	//WASD按下时，数组的对应元素更新为true
	if (keyCode == K::KEY_W)
	{
		keyPressed_[W] = true;
	}
	if (keyCode == K::KEY_A)
	{
		keyPressed_[A] = true;
	}
	if (keyCode == K::KEY_S)
	{
		keyPressed_[S] = true;
	}
	if (keyCode == K::KEY_D)
	{
		keyPressed_[D] = true;
	}
	if (keyCode == K::KEY_Q)
	{
		switchWeapon();
	}
	if (keyCode == K::KEY_SPACE)
	{
		dodge();
	}
		
}

void Player::listenToKeyRelease(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* unusedEvent)
{
	using K = cocos2d::EventKeyboard::KeyCode;

	//WASD按下时，数组的对应元素更新为false
	if (keyCode == K::KEY_W)
	{
		keyPressed_[W] = false;
	}
	if (keyCode == K::KEY_A)
	{
		keyPressed_[A] = false;
	}
	if (keyCode == K::KEY_S)
	{
		keyPressed_[S] = false;
	}
	if (keyCode == K::KEY_D)
	{
		keyPressed_[D] = false;
	}
}

void Player::listenToMouseEvent(cocos2d::Vec2 facingPoint, bool isPressed)
{
	facingPoint_ = facingPoint;

	if (isPressed)
	{
		//Attack
	}
}

void Player::getInjured(int damage)
{
	if (!superBody_)
	{
		int realDamage = static_cast<int>(damage * (1 - shield_));
		if (realDamage >= health_)
		{
			die();
		}
		else
		{
			health_ -= realDamage;
		}
	}	
}

bool Player::isAlive()
{
	return isAlive_;
}

void Player::dodge()
{
	auto dodgeDirection = cocos2d::Vec2::ZERO;
	if (keyPressed_[W])
	{
		dodgeDirection.y += 1;
	}
	if (keyPressed_[A])
	{
		dodgeDirection.x -= 1;
	}
	if (keyPressed_[S])
	{
		dodgeDirection.y -= 1;
	}
	if (keyPressed_[D])
	{
		dodgeDirection.x += 1;
	}
	if (dodgeDirection.x == 0 && dodgeDirection.y == 0)
	{
		auto facingDir = facingPoint_ - getPosition();
		dodgeDirection.x = -facingDir.x;
		dodgeDirection.y = -facingDir.y;
	}

	dodgeDirection.normalize();
	auto delay = cocos2d::DelayTime::create(dodgeTime_);
	auto start = cocos2d::CallFunc::create([=]() {
		DodgeAnimeStart();
		});
	auto act = cocos2d::CallFunc::create([=]() {
		DodgeAnime(dodgeDirection);
		});
	auto end = cocos2d::CallFunc::create([=]() {
		DodgeAnimeEnd();
		});
	this->runAction(cocos2d::Sequence::create(start, act, delay, end, nullptr));
}

void Player::updateFacingStatus()
{
	auto direction = facingPoint_ - getPosition();
	preFacingStatus_ = curFacingStatus_;
	
	if (direction.x > 0 && abs(direction.y) <= direction.x)
	{
		curFacingStatus_ = FacingStatus::right;
	}
	else if (direction.x < 0 && abs(direction.y) <= abs(direction.x))
	{
		curFacingStatus_ = FacingStatus::left;
	}
	else if (direction.y > 0 && abs(direction.x) <= direction.y)
	{
		curFacingStatus_ = FacingStatus::up;
	}
	else if (direction.y < 0 && abs(direction.x) <= abs(direction.y))
	{
		curFacingStatus_ = FacingStatus::down;
	}

	if (preFacingStatus_ != curFacingStatus_)
	{
		statusChanged_ = true;
	}
}

void Player::updateWalkingStatus()
{
	preWalkingStatus_ = curWalkingStatus_;
	curWalkingStatus_ = WalkingStatus::idle;
	for (auto i : keyPressed_)
	{
		if (i)
			curWalkingStatus_ = WalkingStatus::walk;
	}
	if (preWalkingStatus_ != curWalkingStatus_)
	{
		statusChanged_ = true;
	}
}

void Player::DodgeAnimeStart()
{
	canDodge_ = false;
	superBody_ = true;
	allowMove_ = false;
}

void Player::DodgeAnime(cocos2d::Vec2 dir)
{
	getPhysicsBody()->setVelocity(moveSpeed_ * dodgeSpeedBoost_ * dir);	
}

void Player::DodgeAnimeEnd()
{
	canDodge_ = true;
	superBody_ = false;
	allowMove_ = true;
}

void Player::switchWeapon()
{
	if (secondaryWeapon_ != nullptr)
	{
		primaryWeapon_->setVisible(false);
		secondaryWeapon_->setVisible(false);
		auto t = primaryWeapon_;
		primaryWeapon_ = secondaryWeapon_;
		secondaryWeapon_ = t;
	}
}

Weapon* Player::getPrimaryWeaponInstance()
{
	return primaryWeapon_;
}

Weapon* Player::getSecondaryWeaponInstance()
{
	return secondaryWeapon_;
}

const std::string Player::getBulletName() const
{
	return bulletFilename_;
}

void Player::update(float dt)
{
	auto velocity = cocos2d::Vec2::ZERO;
	if (allowMove_)
	{
		if (keyPressed_[W])
		{
			velocity.y += moveSpeed_ * speedBoostFactor_;
		}
		if (keyPressed_[A])
		{
			velocity.x -= moveSpeed_ * speedBoostFactor_;
		}
		if (keyPressed_[S])
		{
			velocity.y -= moveSpeed_ * speedBoostFactor_;
		}
		if (keyPressed_[D])
		{
			velocity.x += moveSpeed_ * speedBoostFactor_;
		}

		//先进行标准化，再乘以模长
		velocity.normalize();
		velocity *= moveSpeed_;
		getPhysicsBody()->setVelocity(velocity);

		updateFacingStatus();
		updateWalkingStatus();
		updateMoveAnimate();
		statusChanged_ = false;

		auto playerPos = this->getPosition();
		auto runningScene = cocos2d::Director::getInstance()->getRunningScene();
		auto boxOfNode = runningScene->getChildByTag(133);
		cocos2d::Vec2 boxPositionInScene = cocos2d::Vec2::ZERO;
		//如果场景已经被释放，找不到我方player位置，直接退出
		if (boxOfNode != nullptr)
		{
			//获得当前我方player位置
			auto boxOfPlayer = dynamic_cast<Sprite*>(boxOfNode);
			boxPositionInScene = boxOfPlayer->getPosition();
			//撞到了
			if (fabs(boxPositionInScene.x - playerPos.x) < 154.5 && fabs(boxPositionInScene.y - playerPos.y) < 99.4)
			{
				this->setPosition(cocos2d::Vec2(playerPos.x, 250.5));
			}
		}

	}	
}